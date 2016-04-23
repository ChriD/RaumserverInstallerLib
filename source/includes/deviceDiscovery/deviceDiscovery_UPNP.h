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
#ifndef RAUMKSERVERINSTALLER_DEVICEDISCOVERY_UPNP_H
#define RAUMKSERVERINSTALLER_DEVICEDISCOVERY_UPNP_H

#include <list>

#include "deviceDiscovery/deviceDiscovery.h"
#include "xml/pugixml/pugixml.hpp"

#include <OpenHome/Net/Cpp/OhNet.h>
#include <OpenHome/Net/Cpp/CpDevice.h>
#include <OpenHome/Net/Cpp/CpDeviceDv.h>
#include <OpenHome/Net/Cpp/CpDeviceUpnp.h>


namespace RaumserverInstaller
{
    namespace DeviceDiscovery
    {
   
        class DeviceDiscovery_UPNP : public DeviceDiscovery
        {
            public:
                DeviceDiscovery_UPNP();
                ~DeviceDiscovery_UPNP();

                virtual void init() override;
                virtual void startDiscover();
                virtual void setNetworkAdapter(NetworkAdaperInformation _networkAdapter);
                virtual void loadNetworkAdaptersInformation();
               
                virtual std::vector<NetworkAdaperInformation> getNetworkAdaptersInformation();                

            protected:
                OpenHome::Net::CpDeviceListCppUpnpAll* upupDeviceListAll;

                std::vector<NetworkAdaperInformation> networkAdapterInfomationList;
                NetworkAdaperInformation selectedNetworkAdapter;
                bool upnpStarted;

                void initAdapter();
                void discover();
                void addRemoveInstallableDevice(const std::string &_location, const std::string &_deviceXML, bool _add = true);
                void addInstallableDevice(const std::string &_location, const std::string &_deviceXML);
                void removeInstallableDevice(const std::string &_location, const std::string &_deviceXML);

                void onDeviceFound(OpenHome::Net::CpDeviceCpp& _device);
                void onDeviceLost(OpenHome::Net::CpDeviceCpp& _device);

                OpenHome::Net::InitialisationParams* initParams;
               
        };

    }
}

#endif