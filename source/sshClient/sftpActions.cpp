
#include "sshClient/sftpActions.h"


namespace RaumserverInstaller
{
    namespace SSHClient
    {

        SFTPActions::SFTPActions() : RaumserverInstallerBase()
        {  
            stopThreads = false;
        }


        SFTPActions::~SFTPActions()
        {
            cancelActions();
        }


        void SFTPActions::cancelActions()
        {
            stopThreads = true;
            if (copyDirThreadObject.joinable())
            {                
                copyDirThreadObject.join();
            }
            stopThreads = false;
        }


        bool SFTPActions::makeDir(std::string _dir)
        {      
            if (!sessionsExists())
                return false;

            std::int32_t returnCode;
            returnCode = sftp_mkdir(sftpSession, _dir.c_str(), /*0000777*/ S_IRWXU | S_IRWXG | S_IRWXO); /// (777 for all)
            if (returnCode != SSH_OK)
            {
                if (sftp_get_error(sftpSession) != SSH_FX_FILE_ALREADY_EXISTS)
                {
                    // TODO: @@@
                    //fprintf(stderr, "Can't create directory: %s\n", ssh_get_error(my_ssh_session));
                    return false;
                }
            }
            return true;
        }


        void SFTPActions::copyDir(std::string _clientDir, std::string _remoteDir, bool _recursive, bool _sync)
        {
            if (!sessionsExists())
                return;

            // start a thread which will handle the copying of the directory to the remote computer
            // there are signals for file copying
            copyDirThreadObject = std::thread(&SFTPActions::copyDirThread, this, _clientDir, _remoteDir, _recursive);
            if (_sync)
                copyDirThreadObject.join();            
        }


        void SFTPActions::copyDirThread(std::string _clientDir, std::string _remoteDir, bool _recursive)
        {                          
            TinyDirCpp::TinyDirCpp  tinyDirCpp;

            // Get all files we have to copy 
            auto filesToCopy = tinyDirCpp.getFiles(_clientDir, "", 0, _recursive);

            // now do copy of all files in a straight forward way
            for (auto file : filesToCopy)
            {                
                copyFile(_clientDir + file, _remoteDir + file);
            }            
        }


        bool SFTPActions::copyFile(std::string _clientFile, std::string _remoteFile)
        {
            if (!sessionsExists())
                return false;

            // create folders if not exists from '_remoteFile'
            auto parts = Raumkernel::Tools::StringUtil::explodeString(_remoteFile, "/");
            std::string folders = "";
            for (std::uint32_t i = 0; i < parts.size() - 1; i++)
            {
                folders += folders.empty() ? "" : "/";
                folders += parts[i];
                if (!makeDir(folders))
                {
                    // TODO: error
                    return false;
                }
                // set the acces to the folder (chmod). For now we will set full access.
                if (!setChmod(folders, S_IRWXU | S_IRWXG | S_IRWXO))
                {
                    // TODO: error
                    return false;
                }
            }
  

            // create file in trunc mode
            // TODO: qhy the hell doesnt this wok with the bit identifiers?
            auto sftpRemoteFile = sftp_open(sftpSession, _remoteFile.c_str(), O_RDWR | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);
            //auto sftpRemoteFile = sftp_open(sftpSession, _remoteFile.c_str(), 0777, O_TRUNC);
            if (sftpRemoteFile == NULL)
            {
                fprintf(stderr, "Can't open file for writing: %s\n", ssh_get_error(sshSession)); // TODO: @@@
                return false;
            }


            std::int32_t nBytes;
            char buffer[MAX_XFER_BUF_SIZE];            
            
            std::ifstream clientFileStream(_clientFile, std::ios::binary);

            if (clientFileStream)
            {
                clientFileStream.seekg(0, std::ios::end);
                std::ios::pos_type fileSize = clientFileStream.tellg(); 
                std::ios::pos_type bufferCount = 0;
                std::ios::pos_type bufferSize = sizeof(buffer);
                clientFileStream.seekg(0); 

                sigStartFileCopying.fire(_clientFile, fileSize);

                while (clientFileStream)
                {
                    bufferCount += bufferSize;
                    if (bufferCount > fileSize)
                        bufferSize = (fileSize - (bufferCount - bufferSize));
                    clientFileStream.read(buffer, bufferSize);
                    sftp_write(sftpRemoteFile, buffer, bufferSize); // write to remote file

                    sigFileCopying.fire(_clientFile, bufferCount, fileSize);
                }

                sftp_close(sftpRemoteFile);

                // TODO: why this si need,? it cant be done ehrn creating file correctly?!
                setChmod(_remoteFile, S_IRWXU | S_IRWXG | S_IRWXO); //TODO: @@@
               
                // signal file copied
                sigEndFileCopying.fire(_clientFile, fileSize);

            }
               

            return true;
        }


        bool SFTPActions::setChmod(std::string _fileOrDir, std::uint16_t _chmod)
        {
            std::int32_t returnCode;

            if (!sessionsExists())
                return false;

            returnCode = sftp_chmod(sftpSession, _fileOrDir.c_str(), _chmod);
            if (returnCode != SSH_OK)
            {                // TODO: @@@
                std::string errorS = ssh_get_error(sshSession);
                std::int32_t error = sftp_get_error(sftpSession);
            }
            return true;
        }


        bool SFTPActions::existsFile(std::string _file)
        {
            // create file in trunc mode
            auto sftpRemoteFile = sftp_open(sftpSession, _file.c_str(), O_RDONLY, S_IRWXU | S_IRWXG | S_IRWXO); // TODO: @@@
            if (sftpRemoteFile == NULL)                            
                return false;            
            sftp_close(sftpRemoteFile);
            return true;
        }


        bool SFTPActions::sessionsExists()
        {
            if (sshSession == nullptr || sftpSession == nullptr)
                return false;
            return true;
        }


        void SFTPActions::setSessions(ssh_session _sshSession, sftp_session _sftpSession)
        {
            sshSession = _sshSession;
            sftpSession = _sftpSession;
        }


    }
}