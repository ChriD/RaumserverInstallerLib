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
#include "xml/pugixml/pugixml.hpp"
#include "updater/updater_RaumserverDaemon.h"

namespace RaumserverInstaller
{
    namespace DeviceInstaller
    {
        enum DeviceInstallerProgressType { DIPT_INSTALL = 0, DIPT_UPDATE = 1, DIPT_REMOVE = 2 };

        struct DeviceInstallerProgressInfo
        {
            DeviceInstallerProgressInfo(DeviceInstallerProgressType _actionType, const std::string &_info, const std::uint8_t &_completionPercentage, const bool &_error, const std::string &_id = "");
            EXPORT Json::Value getJsonValue();

            std::string info = "";
            std::string id = "";
            std::uint8_t completionPercentage = 0;
            bool error = false;
            DeviceInstallerProgressType actionType = DeviceInstallerProgressType::DIPT_INSTALL;
        };


        class DeviceInstaller : public RaumserverInstallerBase
        {
            public:
                DeviceInstaller();
                ~DeviceInstaller();    

                EXPORT virtual void startInstall();
                EXPORT virtual void abortInstall();
                EXPORT virtual void startRemove();
                EXPORT virtual void abortRemove();
                EXPORT virtual bool getActualBinaries();
                EXPORT virtual void setDevice(const DeviceInformation &_deviceInfo);          
                /*
                * sets ssh credentials
                */
                EXPORT void setSSHAuth(const std::string &_user, const std::string &_pass);

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
                DeviceInstallerProgressType progressType;

                std::string sshUser;
                std::string sshPassword;

                std::string binariesSourceWebUrl;
                std::string currentVersionInfoWebUrl;
                std::string deviceInstallerFilePath;
                std::map<std::uint16_t, std::string> hardwareTypeBinaries;

                Updater::Updater_RaumserverDaemon raumserverDaemonUpdater;

                sigs::connections connections;

                void onUpdaterUpdateProgress(Updater::ProgressInfo _progressInfo);
                void onUpdaterBinaryReady();
                void onUpdaterBinaryUpdated();

                EXPORT virtual void loadDeviceInstallerInfoFile();
                EXPORT virtual std::string getDeviceBinaryPath();

                EXPORT virtual void progressDebug(const std::string &_progressInfo, const std::string &_location, const std::string &_id = "");
                EXPORT virtual void progressWarning(const std::string &_progressInfo, const std::string &_location, const std::string &_id = "");
                EXPORT virtual void progressInfo(const std::string &_progressInfo, const std::string &_location, const std::string &_id = "");
                EXPORT virtual void progressError(const std::string &_progressInfo, const std::string &_location, const std::string &_id = "");

        };
    }
}


#endif

