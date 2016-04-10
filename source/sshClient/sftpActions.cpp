
#include "sshClient/sftpActions.h"


namespace RaumserverInstaller
{
    namespace SSHClient
    {

        SFTPActions::SFTPActions()
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


        void SFTPActions::copyDir(std::string _clientDir, std::string _remoteDir, bool _sync)
        {
            if (!sessionsExists())
                return;

            // start a thread which will handle the copying of the directory to the remote computer
            // there are signals for file copying
            copyDirThreadObject = std::thread(&SFTPActions::copyDirThread, this, _clientDir, _remoteDir);
            if (_sync)
                copyDirThreadObject.join();            
        }


        void SFTPActions::copyDirThread(std::string _clientDir, std::string _remoteDir)
        {
            bool allFilesProcessed = false;
            
            // get all files we have to copy 
            TinyDirCpp::TinyDirCpp  tinyDirCpp;

            // create remote directory
            auto filesToCopy = tinyDirCpp.getFiles(_clientDir);

            for (auto file : filesToCopy)
            {                
                copyFile(_clientDir + file, _remoteDir + file);
            }            
        }


        bool SFTPActions::copyFile(std::string _clientFile, std::string _remoteFile)
        {
            if (!sessionsExists())
                return false;

            // create folders if not exists
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
                if (!setChmod(folders, S_IRWXU | S_IRWXG | S_IRWXO))
                {
                    // TODO: error
                    return false;
                }
            }
  

            // create file in trunc mode
            auto sftpRemoteFile = sftp_open(sftpSession, _remoteFile.c_str(), /*O_RDWR*/ 0777, O_TRUNC);
            if (sftpRemoteFile == NULL)
            {
                fprintf(stderr, "Can't open file for writing: %s\n", ssh_get_error(sshSession));
                return SSH_ERROR;
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

                while (clientFileStream)
                {
                    bufferCount += bufferSize;
                    if (bufferCount > fileSize)
                        bufferSize = (fileSize - (bufferCount - bufferSize));
                    clientFileStream.read(buffer, bufferSize);
                    sftp_write(sftpRemoteFile, buffer, bufferSize); // write to remote file
                }

                sftp_close(sftpRemoteFile);

                setChmod(_remoteFile, S_IRWXU | S_IRWXG | S_IRWXO);
               
            }
               

            return false;
        }


        bool SFTPActions::setChmod(std::string _fileOrDir, std::uint16_t _chmod)
        {
            std::int32_t returnCode;

            if (!sessionsExists())
                return false;

            returnCode = sftp_chmod(sftpSession, _fileOrDir.c_str(), _chmod);
            if (returnCode != SSH_OK)
            {
                // TODO: @@@
                std::string errorS = ssh_get_error(sshSession);
                std::int32_t error = sftp_get_error(sftpSession);
            }

            /*

            sftp_attributes attributes;

            // TODO: @@@

            auto sftpRemoteFile = sftp_open(sftpSession, _fileOrDir.c_str(), 0777, O_RDONLY); // TODO: @@@

            // TODO: @@@@ chekc remote file

            attributes = sftp_fstat(sftpRemoteFile);
            attributes->permissions = _chmod;

            returnCode = sftp_setstat(sftpSession, _fileOrDir.c_str(), attributes);
            if (returnCode != SSH_OK)
            {
                // TODO: @@@
                std::string errorS = ssh_get_error(sshSession);
                std::int32_t error = sftp_get_error(sftpSession);
            }

            */
            // TODO: set chmod!
            // http://api.libssh.org/master/libssh_tutor_sftp.html

            /*



            int sftp_setstat(sftp_session  	sftp,
            const char *  	file,
            sftp_attributes  	attr
            )
            */

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