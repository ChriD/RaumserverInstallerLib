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
#ifndef RAUMKSERVERINSTALLER_DEVICEINSTALLER_RF_H
#define RAUMKSERVERINSTALLER_DEVICEINSTALLER_RF_H

#include "deviceInstaller/deviceInstaller.h"
#include "sshClient/sshClient.h";

namespace RaumserverInstaller
{
    namespace DeviceInstaller
    {

        class DeviceInstaller_RaumfeldDevice : public DeviceInstaller
        {
            public:
                DeviceInstaller_RaumfeldDevice();
                ~DeviceInstaller_RaumfeldDevice();

                EXPORT virtual void startInstall() override;
                EXPORT virtual void abortInstall() override;

            protected:
                SSHClient::SSHClient sshClient;

                std::string binaryDir;                
                std::string installDir;

                std::atomic_bool abortInstallThread;
                std::thread installThreadObject;  

                std::double_t fileCopyPercentage;

                void installThread();

                void onStartFileCopying(std::string _filename, std::uint64_t _size);
                void onFileCopying(std::string _filename, std::uint64_t _copiedSize, std::uint64_t _size);
                void onEndFileCopying(std::string _filename, std::uint64_t _size);
             
                signals::connections connections;

        };
    }
}


#endif

