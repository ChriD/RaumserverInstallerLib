
#include "sshClient/sshClient.h"


namespace RaumserverInstaller
{
    namespace SSHClient
    {

        SSHClient::SSHClient() : RaumserverInstallerBase()
        {
            sshSession = nullptr;
            sftpSession = nullptr;
        }


        SSHClient::~SSHClient()
        {
            closeSFTP();
            closeSSH();
        }


        bool SSHClient::failed(std::string _error, std::string _libError, std::int32_t _errorCode)
        {
            logError(_error + " / " + _libError, CURRENT_POSITION);
            return false;
        }


        void SSHClient::setOption(const ssh_options_e &_option, const std::string &_value)
        {
            if (sshOptions.find(_option) != sshOptions.end())
                sshOptions.erase(_option);
            sshOptions.insert(std::make_pair(_option, _value));
        }


        void SSHClient::setAuth(const std::string &_user, const std::string &_password)
        {
            user = _user;
            password = _password;
        }


        bool SSHClient::connectSSH()
        {
            std::int32_t returnCode;

            // there is already a session! we have to close the current one
            if (sshSession != nullptr)
            {                
                if (!closeSSH())
                    return failed("There is aleady an open SSH Session. Trying to close this session failed!");
            }
            
            sshSession = ssh_new();
            if (sshSession == nullptr)
                return failed("SSH Session can not be instanciated!");

            for (auto it : sshOptions)
            {                
                ssh_options_set(sshSession, it.first, it.second.c_str());
            }

            // Connect to server
            returnCode = ssh_connect(sshSession);
            if (returnCode != SSH_OK)
            {
                std::string sshError = ssh_get_error(sshSession);
                ssh_free(sshSession);
                sshSession = nullptr;
                return failed("Error connecting to host!", sshError, returnCode);
            }

            // Authenticate ourselves
            returnCode = ssh_userauth_password(sshSession, user.c_str(), password.c_str());
            if (returnCode != SSH_AUTH_SUCCESS)
            {
                std::string sshError = ssh_get_error(sshSession);
                ssh_disconnect(sshSession);
                ssh_free(sshSession);
                sshSession = nullptr;
                return failed("Error authenticating with user '" + user + "' and password: '" + password +  "' !", sshError, returnCode);               
            }

            sftp.setSessions(sshSession, nullptr);

            return true;
        }


        bool SSHClient::connectSFTP()
        {
            std::int32_t returnCode;

            // there is already a session! we have to cloe the current one
            if (sftpSession != nullptr)                
                closeSFTP();
            
            // if there is no ssh session we have to open one
            if (sshSession == nullptr)
            {
                if (!connectSSH())
                    return false;
            }
            
            sftpSession = sftp_new(sshSession);
            if (sftpSession == nullptr)                           
                return failed("Error allocating SFTP session!", ssh_get_error(sshSession));
         
            returnCode = sftp_init(sftpSession);
            if (returnCode != SSH_OK)
            {
                std::int32_t sftpErrorCode = sftp_get_error(sftpSession);
                sftp_free(sftpSession);
                sftpSession = nullptr;
                return failed("Error initializing SFTP session!", "", sftpErrorCode);
            }

            sftp.setSessions(sshSession, sftpSession);

            return true;
        }


        bool SSHClient::closeSSH()
        {
            if (sshSession == nullptr)
                return false;// failed("No SSH session to close!");

            // if there is a sftp session too we have to close it before we close the ssh session
            if (sftpSession != nullptr)                            
                closeSFTP();            

            ssh_disconnect(sshSession);
            ssh_free(sshSession);
            sshSession = nullptr;

            sftp.setSessions(nullptr, nullptr);

            return true;
        }


        bool SSHClient::closeSFTP()
        {
            if (sftpSession == nullptr)
                return false; //failed("No SFTP session to close!");
            
            sftp.cancelActions();
            sftp_free(sftpSession);
            sftpSession = nullptr;

            sftp.setSessions(sshSession, nullptr);

            return true;
        }


        void SSHClient::setLogObject(std::shared_ptr<Log::Log> _logger)
        {
            RaumserverInstaller::RaumserverInstallerBase::setLogObject(_logger);
            sftp.setLogObject(_logger);
        }


        bool SSHClient::executeCommand(const std::string &_command, std::string &_result)
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