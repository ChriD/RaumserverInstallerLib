//
// The MIT License (MIT)
//
// Copyright (c) 2016 by ChriD
//
// Permission is hereby granted, free of charge,  to any person obtaining a copy of
// this software and  associated documentation  files  (the "Software"), to deal in
// the  Software  without  restriction,  including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
// the Software,  and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:
//
// The above copyright notice and this  permission notice  shall be included in all
// copies or substantial portions of the Software.
//
// THE  SOFTWARE  IS  PROVIDED  "AS IS",  WITHOUT  WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE  AND NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE  LIABLE FOR ANY CLAIM,  DAMAGES OR OTHER LIABILITY, WHETHER
// IN  AN  ACTION  OF  CONTRACT,  TORT  OR  OTHERWISE,  ARISING  FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//


#pragma once
#ifndef RAUMKSERVERINSTALLER_SSHCLIENT_LIBSSH2_H
#define RAUMKSERVERINSTALLER_SSHCLIENT_LIBSSH2_H


#include <map>

#include "raumserverInstallerBase.h"
#include "sshClient/sftpActions.h"
#include "sshClient/scpActions_libssh2.h"
#include "sshClient/sshClient.h"

#include <libssh/libssh.h> 
#include <libssh/sftp.h>


namespace RaumserverInstaller
{
    namespace SSHClient
    {
       
        class SSHClient_Libssh2 : public SSHClient
        {
            public:
                SSHClient_Libssh2();
                ~SSHClient_Libssh2();

                EXPORT bool connectSSH() override;
                EXPORT bool connectSFTP() override;
                EXPORT bool connectSCP(std::string _directory = ".", bool _readMode = true) override;
                EXPORT bool closeSSH() override;
                EXPORT bool closeSFTP() override;
                EXPORT bool closeSCP() override;

                EXPORT void setOption(const ssh_options_e &_option, const std::string &_value) override;
                EXPORT void setAuth(const std::string &_user, const std::string &_password) override;
                
                EXPORT void setLogObject(std::shared_ptr<Log::Log> _logger) override;

                EXPORT bool executeCommand(const std::string &_command, std::string &_result) override;

                SFTPActions sftp;
                SCPActions scp;

            protected:
                std::string user;
                std::string password;

                ssh_session sshSession;
                sftp_session sftpSession; 
                ssh_scp scpSession;

                std::map<ssh_options_e, std::string> sshOptions;

                bool failed(std::string _error, std::string _libError = "", std::int32_t _errorCode = 0);

        };
    }
}


#endif

