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
#ifndef RAUMSERVERINSTALLER_H
#define RAUMSERVERINSTALLER_H

#include <atomic>

#include "raumserverInstallerBase.h"
#include "deviceDiscovery/deviceDiscovery_UPNP.h"
#include "deviceInstaller/deviceInstaller_RF.h"
#include "httpclient/httpClient.h"
#include "versionInfo.h"

namespace RaumserverInstaller
{
    class RaumserverInstaller : public RaumserverInstallerBase
    {
        public:
            EXPORT RaumserverInstaller();
            EXPORT  ~RaumserverInstaller();

            /**
            * Returns the version information
            */
            EXPORT VersionInfo::VersionInfo getVersionInfo();
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
            * Starts removing the Raumserver from the given device
            * This method will do an async remove. The current status and progress can be consumed by attaching to the removeProgress signal
            */
            EXPORT void startRemoveFromDevice(DeviceInformation _deviceInformation);
            /**
            * Returns a vector of NetworkAdapters structures
            * init has to be run before this will return any stuff
            */
            EXPORT std::vector<NetworkAdaperInformation> getNetworkAdapterList();
            /**
            * Returns the Network Adapter Information by Id
            */
            EXPORT NetworkAdaperInformation getNetworkAdapterInformation(std::uint16_t _id);
            /**
            * sets the network adapter we are searching for devices  to install the component
            */
            EXPORT void setNetworkAdapter(NetworkAdaperInformation _networkAdapterInformation);
            /**
            * returns a copy of the device map
            */
            EXPORT std::map<std::string, DeviceInformation> getDeviceMap();
            /**
            * Returns the Device Information by ip
            */
            EXPORT DeviceInformation getDeviceInformation(std::string _ip);
            /**
            * returns a copy of the device map
            */
            EXPORT void initDiscover();;

            /**
            * will be fired if a device was found where we can install the component on
            */
            sigs::signal<void(DeviceInformation)> sigDeviceFoundForInstall;
            /**
            * will be fired if a device was removed where we can install the component on
            */
            sigs::signal<void(DeviceInformation)> sigDeviceRemovedForInstall;
            /**
            * will be fired if a device information was changed
            */
            sigs::signal<void(DeviceInformation)> sigDeviceInformationChanged;
            /**
            * this signal will be fired once in a while when installing the component
            */
            sigs::signal<void(DeviceInstaller::DeviceInstallerProgressInfo)> sigInstallProgressInformation;
            /**
            * this signal will be fired when the installation is complete or there was an error
            */
            sigs::signal<void(DeviceInstaller::DeviceInstallerProgressInfo)> sigInstallCompleted;

        protected:       

            DeviceDiscovery::DeviceDiscovery_UPNP   deviceDiscoveryUPNP;       
            DeviceInstaller::DeviceInstaller_RaumfeldDevice deviceInstaller;

            void onDeviceFound(DeviceInformation _deviceInformation);
            void onDeviceRemoved(DeviceInformation _deviceInformation);

            void onInstallProgress(DeviceInstaller::DeviceInstallerProgressInfo _progressInfo);
            void onInstallDone(DeviceInstaller::DeviceInstallerProgressInfo _progressInfo);

            void onRequestResult(HttpClient::HttpRequest *_request);

            void startSSHAccessCheckerThread(const std::string &_ip);
            void stopSSHAccessCheckerThreads();
            void sshAccessCheckThread(std::string _ip);

            // a mutex that will secure our device list 
            std::mutex mutexDeviceInformationMap;
            std::map<std::string, DeviceInformation> deviceInformationMap;

            // this ones are for checking the ssh access 
            std::vector<std::thread> sshAccessCheckThreads;
            std::atomic_bool stopSSHAccessCheckThreads;

            // client for cheking if server is running
            HttpClient::HttpClient httpClient;    

            // version info
            VersionInfo::VersionInfo versionInfo;

            sigs::connections connections;
    };

}

#endif