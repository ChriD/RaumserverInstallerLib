
#include "sshClient/scpActions.h"


namespace RaumserverInstaller
{
    namespace SSHClient
    {

        SCPActions::SCPActions() : RaumserverInstallerBase()
        {  
            stopThreads = false;
        }


        SCPActions::~SCPActions()
        {
            //cancelActions();
        }


        void SCPActions::setError(const std::string &_error, const std::int16_t &_errorCode)
        {
            logError(_error + std::to_string(_errorCode), CURRENT_POSITION);
            error = _error;
            errorCode = _errorCode;
        }


        void SCPActions::cancelActions()
        {
            stopThreads = true;
            if (copyDirThreadObject.joinable())
            {                
                copyDirThreadObject.join();
            }
            stopThreads = false;
        }


        bool SCPActions::makeDir(std::string _dir)
        {      
            std::int16_t returnCode;            
      
            if (!sessionsExists())
                return false;     

            if (_dir == ".." || _dir == ".")
                return true;

            //localSession = ssh_scp_new(sshSession, SSH_SCP_WRITE | SSH_SCP_RECURSIVE, "..");
            //if (localSession == nullptr)
            //{
            //   setError("SCP Session can not be instanciated! Error: " + std::string(ssh_get_error(sshSession)));            
            //    return false;
            //}
            
            returnCode = ssh_scp_push_directory(scpSession, _dir.c_str(), S_IRWXU);
            if (returnCode != SSH_OK)
            {
                setError("Can't create Directory: " + std::string(ssh_get_error(sshSession)));
                //ssh_scp_close(scpSession);
                //ssh_scp_free(scpSession);
                return false;            
            }

            return true;
        }


        bool SCPActions::removeDir(std::string _dir)
        {
            std::int32_t returnCode = 0;
            std::string result;

            if (!sessionsExists())
                return false;               

            executeCommand("rm - rf " + _dir, result);

            if (!result.empty())
                return false;
            
            return true;
        }


        void SCPActions::copyDir(std::string _clientDir, std::string _remoteDir, bool _recursive, bool _sync)
        {
            if (!sessionsExists())
                return;

            // start a thread which will handle the copying of the directory to the remote computer
            // there are signals for file copying when buffer copy is done aso...
            copyDirThreadObject = std::thread(&SCPActions::copyDirThread, this, _clientDir, _remoteDir, _recursive);
            if (_sync)
                copyDirThreadObject.join();            
        }


        void SCPActions::copyDirThread(std::string _clientDir, std::string _remoteDir, bool _recursive)
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

        bool SCPActions::removeFile(std::string _file)
        {
            std::int32_t returnCode = 0;
            std::string result;

            if (!sessionsExists())
                return false;

            executeCommand("rm - r " + _file, result);

            if (!result.empty())
                return false;

            return true;
        }


        bool SCPActions::copyFile(std::string _clientFile, std::string _remoteFile)
        {
            std::int32_t returnCode, childCount = 0;
            bool ret = true;

            if (!sessionsExists())
                return false;

            // create folders if not exists from '_remoteFile'
            auto parts = Tools::StringUtil::explodeString(_remoteFile, "/");
            std::string folders = "";
            for (std::uint32_t i = 0; i < parts.size() - 1; i++)
            {
                folders += folders.empty() ? "" : "/";
                folders += parts[i];
                if (!makeDir(folders))                                 
                    return false;

                childCount++;
                
                // set the acces to the folder (chmod). For now we will set full access.
                if (!setChmod("/" + folders, S_IRWXU | S_IRWXG | S_IRWXO))                                    
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

                returnCode = ssh_scp_push_file(scpSession, _remoteFile.c_str(), fileSize, /*S_IRUSR | S_IWUSR*/ 0777);
                if (returnCode != SSH_OK)
                {
                    setError("Can't open file '" + _remoteFile + "' for writing: " + std::string(ssh_get_error(sshSession)));
                    ret = false;
                }

                if (ret)
                {

                    std::int64_t fileSizeLong = fileSize;
                    std::string clientFile = _clientFile;

                    sigStartFileCopying.fire(clientFile, fileSizeLong);

                    // write to remote file with buffering
                    while (clientFileStream)
                    {
                        bufferCount += bufferSize;
                        if (bufferCount > fileSize)
                            bufferSize = (fileSize - (bufferCount - bufferSize));
                        clientFileStream.read(buffer, bufferSize);

                        returnCode = ssh_scp_write(scpSession, buffer, (size_t)bufferSize);
                        if (returnCode != SSH_OK)
                        {
                            // TODO: Hmm... why error when working?!
                            //setError("Can't write to file '" + _remoteFile + "': " + std::string(ssh_get_error(sshSession)));
                            //ret = false;
                            //break;
                        }

                        sigFileCopying.fire(clientFile, bufferCount, fileSizeLong);
                    }

                    // INFO: creating the files with the bitIdentifiers for 777 for access doesnt work proper.
                    // I dont't know why so we have to call an 'extra' setChmod for the created files
                    if (!setChmod("/" + _remoteFile, S_IRWXU | S_IRWXG | S_IRWXO))
                        ret = false;

                    // signal file copied
                    sigEndFileCopying.fire(clientFile, fileSizeLong);
                }

            }

            // jump back to root
            for (std::int16_t i = 0; i < childCount; i++)
                previousDir();
           
            return ret;
        }


        bool SCPActions::previousDir()
        {
            std::int16_t returnCode = ssh_scp_leave_directory(scpSession);
            if (returnCode != SSH_OK)
            {
                setError("Can't leave directory: " + std::string(ssh_get_error(sshSession)));
                //return false;
            }
            return true;
        }


        bool SCPActions::setChmod(std::string _fileOrDir, std::uint16_t _chmod)
        {
            std::int32_t returnCode = 0;
            std::string result;

            if (!sessionsExists())
                return false;

            // TODO: @@@ use given chmod
            executeCommand("chmod 777 " + _fileOrDir, result);
    
            return true;
        }


        bool SCPActions::existsFile(std::string _file)
        {
            std::string result;
            /*std::int32_t returnCode = 0, size = 0;

            if (!connect(_file, true))            
                return false;            

            returnCode = ssh_scp_pull_request(scpSession);
            if (returnCode != SSH_SCP_REQUEST_NEWFILE)
            {
                setError("Error receiving information: " + std::string(ssh_get_error(sshSession)));
                return false;
            }

            size = ssh_scp_request_get_size(scpSession);
            if (size == 0)
                return false;
                */
            executeCommand("wc -c " + _file, result);
            if (result.empty())
                return false;
            
            return true;
        }


        bool SCPActions::sessionsExists()
        {
            if (sshSession == nullptr || scpSession == nullptr)
                return false;
            return true;
        }


        void SCPActions::setSessions(ssh_session _sshSession, ssh_scp _scpSession)
        {
            sshSession = _sshSession;
            scpSession = _scpSession;
        }

        bool SCPActions::connect(std::string _directory, bool _readMode, bool _leaveDir)
        {
            if (scpSession != nullptr)
            {
                if (!close())
                {
                    setError("There is aleady an open SCP Session. Trying to close this session failed!");
                    return false;
                }
            }

            std::int16_t returnCode;

            if (_readMode)
                scpSession = ssh_scp_new(sshSession, SSH_SCP_READ | SSH_SCP_RECURSIVE, _directory.c_str());
            else
                scpSession = ssh_scp_new(sshSession, SSH_SCP_WRITE | SSH_SCP_RECURSIVE, _directory.c_str());

            if (scpSession == nullptr)
            {
                setError("SCP Session can not be instanciated! Error: " + std::string(ssh_get_error(sshSession)));
                return false;
            }

            returnCode = ssh_scp_init(scpSession);
            if (returnCode != SSH_OK)
            {
                std::string error = ssh_get_error(sshSession);                
                ssh_scp_free(scpSession);
                setError("Error initializing SCP session: " + error);
                return false;
            }

            /*if (_leaveDir && !_readMode)
            {
                returnCode = ssh_scp_leave_directory(scpSession);
                if (returnCode != SSH_OK)
                {                    
                    std::string error = ssh_get_error(sshSession);
                    ssh_scp_free(scpSession);
                    setError("Error leaving dirextory: " + error);
                }
            }
            */

            return true;
        }


        bool SCPActions::close()
        {
            if (scpSession == nullptr)
                return false;

            cancelActions();

            ssh_scp_close(scpSession);
            ssh_scp_free(scpSession);

            scpSession = nullptr;
            setSessions(sshSession, nullptr);

            return true;
        }


        bool SCPActions::executeCommand(const std::string &_command, std::string &_result)
        {
            ssh_channel channel;
            int rc;

            channel = ssh_channel_new(sshSession);
            if (channel == NULL)
            {
                logError("Can't create channel: " + std::string(ssh_get_error(sshSession)), CURRENT_POSITION);
                return false;
            }

            rc = ssh_channel_open_session(channel);
            if (rc != SSH_OK)
            {
                logError("Can't create session: " + std::string(ssh_get_error(sshSession)), CURRENT_POSITION);
                ssh_channel_free(channel);
                return false;
            }

            logDebug("Execute SSH command: " + _command, CURRENT_POSITION);

            rc = ssh_channel_request_exec(channel, _command.c_str());
            if (rc != SSH_OK)
            {
                logError("Can't execute command: " + _command + " : " + std::string(ssh_get_error(sshSession)), CURRENT_POSITION);
                ssh_channel_close(channel);
                ssh_channel_free(channel);
                return false;
            }

            logDebug("Reading command return data", CURRENT_POSITION);

            // TODO: better result buffer...

            std::string result = "";
            char buffer[256];
            std::int32_t nbytes, sumBytes = 0;
            nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);
            while (nbytes > 0)
            {
                std::string partResult = buffer;
                partResult.resize(nbytes);
                result += partResult;
                sumBytes += nbytes;
                nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);
            }
            if (nbytes < 0)
            {
                // Well, no return is ok...
            }

            ssh_channel_send_eof(channel);
            ssh_channel_close(channel);
            ssh_channel_free(channel);

            _result = result;

            return true;
        }


    }
}