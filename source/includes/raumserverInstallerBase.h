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
#ifndef RAUMSERVERINSTALLER_BASE_H
#define RAUMSERVERINSTALLER_BASE_H

#include <signals/signals.hpp>
#include <logger/logger.h>
#include <json/json.h>

namespace RaumserverInstaller
{

    const std::string remoteInstallationPath = "../etc/raumfeld/raumserverDaemon/";
    const std::string remoteInstallationPathInitScript = "../etc/init.d/";

    enum DeviceType { DT_UNKNOWN = 0 , DT_UPNPDEVICE_RAUMFELD = 1};
    enum UnknownYesNo  { UNKNOWN = -1, YES = 1, NO = 0 };    
  
    /**
    * Network adapter information
    */
    struct NetworkAdaperInformation
    {
        std::string name = "";
        std::string fullName = "";
        std::uint32_t address = 0;
        std::uint16_t id = 0;

        /* for gcc?
        NetworkAdaperInformation()
        {            
            name = "";
            fullName = "";
            address = 0;
            id = 0;
        }
        */
    };


    /**
    * Device information struct where the raumserver can be installed on
    */
    struct DeviceInformation
    {

        EXPORT Json::Value getJsonValue();      

        std::string name = "";
        std::string friendlyName = "";
        std::string ip = "";
        std::string UDN = "";
        DeviceType type = DeviceType::DT_UNKNOWN;
        UnknownYesNo sshAccess = UnknownYesNo::UNKNOWN;
        UnknownYesNo raumserverRuns = UnknownYesNo::UNKNOWN;
        UnknownYesNo raumserverInstalled = UnknownYesNo::UNKNOWN;
        std::string raumserverVersion = "";
    };



    class RaumserverInstallerBase
    {
        public:
            RaumserverInstallerBase();
            ~RaumserverInstallerBase();

            EXPORT virtual void setLogObject(std::shared_ptr<Raumkernel::Log::Log> _log);
            EXPORT std::shared_ptr<Raumkernel::Log::Log> getLogObject();

        protected:
            std::shared_ptr<Raumkernel::Log::Log> logObject;

            EXPORT virtual void logDebug(const std::string &_log, const std::string &_location);
            EXPORT virtual void logWarning(const std::string &_log, const std::string &_location);
            EXPORT virtual void logInfo(const std::string &_log, const std::string &_location);
            EXPORT virtual void logError(const std::string &_log, const std::string &_location);
            EXPORT virtual void logCritical(const std::string &_log, const std::string &_location);
    };

}

#endif