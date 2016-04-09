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

// libssh http://stackoverflow.com/questions/8204259/how-to-use-libssh-in-vs10

// http://stackoverflow.com/questions/13691520/how-to-copy-a-file-in-c-c-with-libssh-and-sftp


#pragma once
#ifndef RAUMSERVERINSTALLER_H
#define RAUMSERVERINSTALLER_H

#include "raumserverInstallerBase.h"
#include "deviceDiscovery/deviceDiscovery_UPNP.h"

#include <libssh/libssh.h> 
#include <libssh/sftp.h>

#include "sshClient/sshClient.h"


namespace RaumserverInstaller
{
    class RaumserverInstaller : public RaumserverInstallerBase
    {
        public:
            EXPORT RaumserverInstaller();
            EXPORT  ~RaumserverInstaller();

            /**
            * Does the Initialisation of the RaumserverInstallerLib
            */
            EXPORT void init(); 
            /**
            * initializes the log object
            * has to be called before init!
            */
            EXPORT virtual void initLogObject(Raumkernel::Log::LogType _defaultLogLevel = Raumkernel::Log::LogType::LOGTYPE_ERROR, const std::string &_logFilePath = "logs/", const std::vector<std::shared_ptr<Raumkernel::Log::LogAdapter>> &_adapterList = std::vector<std::shared_ptr<Raumkernel::Log::LogAdapter>>());
            /**
            * Starts the discovering of devices where we can install the component on
            * this method is async
            */
            EXPORT void startDiscoverDevicesForInstall();
            /**
            * Starts installing the Raumserver to the given device
            * This method will do an async install. The current status and progress can be consumed by attaching to the installProgress signal
            */
            EXPORT void startInstallToDevice(DeviceInformation _deviceInformation);
            /**
            * Returns a vector of NetworkAdapters structures
            * init has to be run before this will return any stuff
            */
            EXPORT std::vector<NetworkAdaperInformation> getNetworkAdapterList();
            /**
            * sets the network adapter we are searching for devices  to install the component
            */
            EXPORT void setNetworkAdapter(const NetworkAdaperInformation &_networkAdapterInformation);
            /**
            * returns a copy of the device map
            */
            EXPORT std::map<std::string, DeviceInformation> getDeviceMap();
            /**
            * returns a copy of the device map
            */
            EXPORT void initDiscover();

            /**
            * only for testp
            */
            EXPORT void test();

            /**
            * will be fired if a device was found where we can install the component on
            */
            sigs::signal<void(DeviceInformation)> sigDeviceFoundForInstall;
            /**
            * will be fired if a device was removed where we can install the component on
            */
            sigs::signal<void(DeviceInformation)> sigDeviceRemovedForInstall;
            /**
            * this signal will be fired once in a while when installing the component
            */
            sigs::signal<void(/*InstallProgressInformation*/)> sigInstallProgressInformation;
            /**
            * this signal will be fired when the installation is complete or there was an error
            */
            sigs::signal<void(/*InstallProgressInformation, successful*/)> sigInstallCompleted;

        protected:       

            DeviceDiscovery::DeviceDiscovery_UPNP   deviceDiscoveryUPNP;                    

            void onDeviceFound(DeviceInformation _deviceInformation);
            void onDeviceRemoved(DeviceInformation _deviceInformation);

            // a mutex that will secure our device list 
            std::mutex mutexDeviceInformationMap;
            std::map<std::string, DeviceInformation> deviceInformationMap;

            sigs::connections connections;
    };

}

#endif