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
#ifndef RAUMKSERVERINSTALLER_DEVICEINSTALLER_H
#define RAUMKSERVERINSTALLER_DEVICEINSTALLER_H

#include "raumserverInstallerBase.h"

namespace RaumserverInstaller
{
    namespace DeviceInstaller
    {

        struct DeviceInstallerProgressInfo
        {
            DeviceInstallerProgressInfo::DeviceInstallerProgressInfo(const std::string &_info, const std::uint8_t &_completionPercentage, const bool &_error);
            EXPORT Json::Value getJsonValue();

            std::string info = "";
            std::uint8_t completionPercentage = 0;
            bool error = false;
        };


        class DeviceInstaller : public RaumserverInstallerBase
        {
            public:
                DeviceInstaller();
                ~DeviceInstaller();    

                EXPORT virtual void startInstall();
                EXPORT virtual void abortInstall();
                EXPORT virtual void setDevice(const DeviceInformation &_deviceInfo);

                /**
                * 
                */
                sigs::signal<void(DeviceInstallerProgressInfo)> sigInstallProgress;
                /**
                * 
                */
                sigs::signal<void(DeviceInstallerProgressInfo)> sigInstallDone;

            protected:
                DeviceInformation deviceInformation;
                std::double_t progressPercentage;

                EXPORT virtual void progressDebug(const std::string &_progressInfo, const std::string &_location);
                EXPORT virtual void progressWarning(const std::string &_progressInfo, const std::string &_location);
                EXPORT virtual void progressInfo(const std::string &_progressInfo, const std::string &_location);
                EXPORT virtual void progressError(const std::string &_progressInfo, const std::string &_location);

        };
    }
}


#endif

