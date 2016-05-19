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
#ifndef RAUMKSERVERINSTALLER_SSHCLIENT_H
#define RAUMKSERVERINSTALLER_SSHCLIENT_H


#include <map>

#include "raumserverInstallerBase.h"
#include "sshClient/sftpActions.h"

#include <libssh/libssh.h> 
#include <libssh/sftp.h>


namespace RaumserverInstaller
{
    namespace SSHClient
    {
       
        class SSHClient : public RaumserverInstallerBase
        {
            public:
                SSHClient();
                ~SSHClient();

                EXPORT bool connectSSH();
                EXPORT bool connectSFTP();
                EXPORT bool closeSSH();
                EXPORT bool closeSFTP();

                EXPORT void setOption(const ssh_options_e &_option, const std::string &_value);
                EXPORT void setAuth(const std::string &_user, const std::string &_password);
                
                EXPORT void setLogObject(std::shared_ptr<Log::Log> _logger) override;

                SFTPActions sftp;

            protected:
                std::string user;
                std::string password;

                ssh_session sshSession;
                sftp_session sftpSession;

                std::map<ssh_options_e, std::string> sshOptions;

                bool failed(std::string _error, std::string _libError = "", std::int32_t _errorCode = 0);

        };
    }
}


#endif

