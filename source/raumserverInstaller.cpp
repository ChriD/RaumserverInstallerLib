#include "raumserverInstaller.h"

namespace RaumserverInstaller
{
    
    RaumserverInstaller::RaumserverInstaller()
    {
    }


    RaumserverInstaller::~RaumserverInstaller()
    {
    }


    void RaumserverInstaller::init()
    {   
        initDiscover();
    }


    void RaumserverInstaller::initDiscover()
    {
        // init the UPNP device discovery (which will init our UPNP stack too!)
        deviceDiscoveryUPNP.init();
    }


    void RaumserverInstaller::startDiscoverDevicesForInstall()
    {
        // start the dsicovering for UPNP Devices where we can install the component on
        // the object will create a device information where we do subscribe on add
        connections.connect(deviceDiscoveryUPNP.sigDeviceFound, this, &RaumserverInstaller::onDeviceFound);
        connections.connect(deviceDiscoveryUPNP.sigDeviceRemoved, this, &RaumserverInstaller::onDeviceRemoved);
        deviceDiscoveryUPNP.startDiscover();        
    }


    void RaumserverInstaller::onDeviceFound(DeviceInformation _deviceInformation)
    {
        // TODO: @@@
        sigDeviceFoundForInstall.fire(_deviceInformation);
    }


    void RaumserverInstaller::onDeviceRemoved(DeviceInformation _deviceInformation)
    {
        // TODO: @@@
        sigDeviceRemovedForInstall.fire(_deviceInformation);
    }


    std::vector<NetworkAdaperInformation> RaumserverInstaller::getNetworkAdapterList()
    {
        // use the list directly from the UPNP stack who is so gentle to give us this informations
        return deviceDiscoveryUPNP.getNetworkAdaptersInformation();
    }


    void RaumserverInstaller::setNetworkAdapter(const NetworkAdaperInformation &_networkAdapterInformation)
    {
        deviceDiscoveryUPNP.setNetworkAdapter(_networkAdapterInformation);
    }
   

    void RaumserverInstaller::startInstallToDevice(DeviceInformation _deviceInformation)
    {

    }

}

