
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


        void SFTPActions::setError(const std::string &_error ,const std::int16_t &_errorCode)
        {
            logError(_error + std::to_string(_errorCode), CURRENT_POSITION);
            error = _error;
            errorCode = _errorCode;
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
            returnCode = sftp_mkdir(sftpSession, _dir.c_str(), S_IRWXU | S_IRWXG | S_IRWXO); /// (777 for all)
            if (returnCode != SSH_OK)
            {
                if (sftp_get_error(sftpSession) != SSH_FX_FILE_ALREADY_EXISTS)
                {
                    setError("Can't create Directory: " + std::string(ssh_get_error(sshSession)));                   
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
            // there are signals for file copying when buffer copy is done aso...
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
                    return false;
                
                // set the acces to the folder (chmod). For now we will set full access.
                if (!setChmod(folders, S_IRWXU | S_IRWXG | S_IRWXO))                                    
                    return false;                
            }

            // Create file in trunc mode
            // INFO: Bit identifiers for access (chmod) doesn't work proper. Maybe i am doeing it wrong but i've created a workaround
            // by setting the chmod value later in the methos
            auto sftpRemoteFile = sftp_open(sftpSession, _remoteFile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, /*S_IRWXU | S_IRWXG | S_IRWXO*/ 1);
            if (sftpRemoteFile == NULL)
            {
                setError("Can't open file '" + _remoteFile  + "' for writing: " + std::string(ssh_get_error(sshSession)), sftp_get_error(sftpSession));
                return false;
            }
            
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

                // write to remote file with buffering
                while (clientFileStream)
                {
                    bufferCount += bufferSize;
                    if (bufferCount > fileSize)
                        bufferSize = (fileSize - (bufferCount - bufferSize));
                    clientFileStream.read(buffer, bufferSize);
                    sftp_write(sftpRemoteFile, buffer, (size_t)bufferSize); 

                    sigFileCopying.fire(_clientFile, bufferCount, fileSize);
                }

                sftp_close(sftpRemoteFile);
                
                // INFO: creating the files with the bitIdentifiers for 777 for access doesnt work proper.
                // I dont't know why so we have to call an 'extra' setChmod for the created files
                if (!setChmod(_remoteFile, S_IRWXU | S_IRWXG | S_IRWXO))
                    return false;
               
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
            {                
                setError("Error chmod file " + _fileOrDir  + " : " + std::string(ssh_get_error(sshSession)), sftp_get_error(sftpSession));
                return false;
            }
            return true;
        }


        bool SFTPActions::existsFile(std::string _file)
        {
            // create file in trunc mode
            auto sftpRemoteFile = sftp_open(sftpSession, _file.c_str(), O_RDONLY, S_IRWXU | S_IRWXG | S_IRWXO);
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