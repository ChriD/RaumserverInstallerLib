
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
        }


        bool SSHClient::failed(std::string _error, std::string _libError, std::int32_t _errorCode)
        {
            return false;
        }


        void SSHClient::setOption(const ssh_options_e &_option, const std::string &_value)
        {
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
                return failed("No SSH session to close!");

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
                return failed("No SFTP session to close!");
            
            sftp.cancelActions();
            sftp_free(sftpSession);
            sftpSession = nullptr;

            sftp.setSessions(sshSession, nullptr);

            return true;
        }

    }
}