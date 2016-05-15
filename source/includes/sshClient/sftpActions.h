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


/*
#define O_RDONLY	0x0000
#define O_WRONLY	0x0001
#define O_RDWR		0x0002
#define O_ACCMODE	0x0003

#define O_BINARY	0x0004	
#define O_TEXT		0x0008	
#define O_NOINHERIT	0x0080	

#define O_CREAT		0x0100	
#define O_EXCL		0x0200
#define O_NOCTTY	0x0400
#define O_TRUNC		0x0800
#define O_APPEND	0x1000
#define O_NONBLOCK	0x2000
*/

#define	S_IRWXU	0000700			
#define	S_IRUSR	0000400			
#define	S_IWUSR	0000200			
#define	S_IXUSR	0000100			

#define	S_IRWXG	0000070			
#define	S_IRGRP	0000040			
#define	S_IWGRP	0000020			
#define	S_IXGRP	0000010			

#define	S_IRWXO	0000007			
#define	S_IROTH	0000004			
#define	S_IWOTH	0000002			
#define	S_IXOTH	0000001			


#define MAX_XFER_BUF_SIZE 16384


#include <map>

#include "raumserverInstallerBase.h"
#include "tools/tinydircpp.h"

#include <libssh/libssh.h> 
#include <libssh/sftp.h>

#include <fcntl.h> 
#include <stdio.h> 
#include <sys/stat.h> 
#include <sys/types.h> 

namespace RaumserverInstaller
{
    namespace SSHClient
    {

        class SFTPActions : public RaumserverInstallerBase
        {
            public:
                SFTPActions();
                ~SFTPActions();

                EXPORT void setSessions(ssh_session _sshSession, sftp_session _sftpSession);
                EXPORT void cancelActions();
                
                EXPORT bool makeDir(std::string _remoteDir);
                EXPORT bool removeDir(std::string _remoteDir);
                EXPORT void copyDir(std::string _clientDir, std::string _remoteDir, bool _recursive = true, bool _sync = false);
                EXPORT bool copyFile(std::string _clientFile, std::string _remoteFile);
                EXPORT bool removeFile(std::string _remoteFile);
                EXPORT bool setChmod(std::string _fileOrDir, std::uint16_t _chmod);
                EXPORT bool existsFile(std::string _file);
                
                sigs::signal<void(std::string _filename, std::uint64_t _copiedSize, std::uint64_t _size)> sigFileCopying;
                sigs::signal<void(std::string _filename, std::uint64_t _size)> sigEndFileCopying;
                sigs::signal<void(std::string _filename, std::uint64_t _size)> sigStartFileCopying;

            protected:
                ssh_session sshSession;
                sftp_session sftpSession;

                std::string error;
                std::int16_t errorCode;
     
                std::thread copyDirThreadObject;

                std::atomic_bool stopThreads;

                void copyDirThread(std::string _clientDir, std::string _remoteDir, bool _recursive = true);
                
                void setError(const std::string &_error = "", const std::int16_t &_errorCode = 999);
                bool sessionsExists();

        };
       
    }
}


#endif

