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
#ifndef RAUMKSERVERINSTALLER_SFTPACTIONS_H
#define RAUMKSERVERINSTALLER_SFTPACTIONS_H


#include <map>

#include "raumserverInstallerBase.h"

#include <libssh/libssh.h> 
#include <libssh/sftp.h>


namespace RaumserverInstaller
{
    namespace SSHClient
    {

        class SFTPActions
        {
            public:
                SFTPActions();
                ~SFTPActions();

                EXPORT void setSessions(ssh_session _sshSession, sftp_session _sftpSession);
                EXPORT void cancelActions();

                //EXPORT bool createDir(std::string _dir)
                //EXPORT bool moveToDir(std::string _dir)

                // will be sync or async (signal fileCopyStatus(FileCopyStatus _)
                //EXPORT bool copyDir(std::string _clientDir, std::string _remoteDir, bool _sync = false) --> starts thread
                //EXPORT bool copyFile(std::string _clientFile, std::string _remoteFile, bool _sync = false) --> starts thread

            protected:
                ssh_session sshSession;
                sftp_session sftpSession;

                bool sessionsExists();
        };
       
    }
}


#endif

