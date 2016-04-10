#include "raumserverInstaller.h"

namespace RaumserverInstaller
{
    
    RaumserverInstaller::RaumserverInstaller()
    {
        stopSSHAccessCheckThreads = false;
    }


    RaumserverInstaller::~RaumserverInstaller()
    {
        stopSSHAccessCheckerThreads();
    }


    void RaumserverInstaller::init()
    {  
        connections.connect(deviceDiscoveryUPNP.sigDeviceFound, this, &RaumserverInstaller::onDeviceFound);
        connections.connect(deviceDiscoveryUPNP.sigDeviceRemoved, this, &RaumserverInstaller::onDeviceRemoved);

        connections.connect(deviceInstaller.sigInstallProgress, this, &RaumserverInstaller::onInstallProgress);
        connections.connect(deviceInstaller.sigInstallDone, this, &RaumserverInstaller::onInstallDone);
    }

    void RaumserverInstaller::initLogObject(Raumkernel::Log::LogType _defaultLogLevel, const std::string &_logFilePath, const std::vector<std::shared_ptr<Raumkernel::Log::LogAdapter>> &_adapterList)
    {
        // create the log object (if not already provided) which will be used throughout the whole kernel and his modules                
        logObject = std::shared_ptr<Raumkernel::Log::Log>(new Raumkernel::Log::Log());

        // we do register some log adapters for the log object with standard values and set to log only ERRORS and CRITICALERRORS
        // the log level itself will be set after reading the settings file but we want to have logging while reading settings file
        // so we create the logObject with some standard values if they are not given
        if (_adapterList.empty())
        {
            auto logAdapterConsole = std::shared_ptr<Raumkernel::Log::LogAdapter_Console>(new Raumkernel::Log::LogAdapter_Console());
            logObject->registerAdapter(logAdapterConsole);

            auto logAdapterFile = std::shared_ptr<Raumkernel::Log::LogAdapter_File>(new Raumkernel::Log::LogAdapter_File());
            if (!_logFilePath.empty())
                logAdapterFile->setLogFilePath(_logFilePath);
            logObject->registerAdapter(logAdapterFile);
        }
        else
        {
            for (auto i : _adapterList)
            {
                logObject->registerAdapter(i);
            }
        }

        logObject->setLogLevel(_defaultLogLevel);
    }


    void RaumserverInstaller::initDiscover()
    {
        deviceDiscoveryUPNP.setLogObject(getLogObject());
        // init the UPNP device discovery (which will init our UPNP stack too!)
        deviceDiscoveryUPNP.init();
    }


    void RaumserverInstaller::startDiscoverDevicesForInstall()
    {
        // start the dsicovering for UPNP Devices where we can install the component on               
        deviceDiscoveryUPNP.startDiscover();         
    }


    std::map<std::string, DeviceInformation> RaumserverInstaller::getDeviceMap()
    {
        // lock while copying the map
        std::unique_lock<std::mutex> lock(mutexDeviceInformationMap);
        return deviceInformationMap;
    }


    void RaumserverInstaller::onDeviceFound(DeviceInformation _deviceInformation)
    { 
        try
        {
            // lock map when inserting a value 
            mutexDeviceInformationMap.lock();
            try
            {
                deviceInformationMap.insert(std::make_pair(_deviceInformation.ip, _deviceInformation));
            }
            catch (...)
            {
                logError("Exception ocurred while adding device to map!", CURRENT_POSITION);
            }
            mutexDeviceInformationMap.unlock();
        }
        // be sure we will find recursive locks (deadlocks) created by the application!
        catch (...)
        {
            logError("Exception ocurred while locking device map!", CURRENT_POSITION);
        }        
    
        sigDeviceFoundForInstall.fire(_deviceInformation);   

        startSSHAccessCheckerThread(_deviceInformation.ip);
    }


    void RaumserverInstaller::onDeviceRemoved(DeviceInformation _deviceInformation)
    {
        // TODO: @@@
        sigDeviceRemovedForInstall.fire(_deviceInformation);
    }


    void RaumserverInstaller::onInstallProgress(DeviceInstaller::DeviceInstallerProgressInfo _progressInfo)
    {
        sigInstallProgressInformation.fire(_progressInfo);       
    }


    void RaumserverInstaller::onInstallDone(DeviceInstaller::DeviceInstallerProgressInfo _progressInfo)
    {
        sigInstallCompleted.fire(_progressInfo);        
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
        // commented because it makes some problems i have to investigate further
        //stopSSHAccessCheckerThreads();
        
        deviceInstaller.setLogObject(getLogObject());
        deviceInstaller.setDevice(_deviceInformation);
        deviceInstaller.startInstall();

    }


    void RaumserverInstaller::startSSHAccessCheckerThread(const std::string &_ip)
    {              
        sshAccessCheckThreads.push_back(std::move(std::thread(&RaumserverInstaller::sshAccessCheckThread, this, _ip)));
    }

    
    void RaumserverInstaller::stopSSHAccessCheckerThreads()
    {
        stopSSHAccessCheckThreads = true;

        for (auto &thread : sshAccessCheckThreads)
        {
            if (thread.joinable())
                thread.join();
        }

        stopSSHAccessCheckThreads = false;
    }


    void RaumserverInstaller::sshAccessCheckThread(std::string _ip)
    {
        bool hasSFTPAccess = false;
        DeviceInformation deviceInfo;

        if (stopSSHAccessCheckThreads)
            return;

        SSHClient::SSHClient sshClient;
        sshClient.setOption(ssh_options_e::SSH_OPTIONS_HOST, _ip);
        sshClient.setAuth("root", "");
        if (sshClient.connectSSH())
        {
            if (sshClient.connectSFTP())
            {
                hasSFTPAccess = true;
            }
        }        

        mutexDeviceInformationMap.lock();

        try
        {

            auto it = deviceInformationMap.find(_ip);
            if (it != deviceInformationMap.end())
            {
                auto access = hasSFTPAccess ? "yes" : "no";
                logInfo("SSH Access for " + it->second.name + " (" + it->second.ip + "): " + access, CURRENT_POSITION);               
                it->second.sshAccess = hasSFTPAccess ? SSHAccess::SSH_YES : SSHAccess::SSH_NO;

                // get a copy of the info struct
                deviceInfo = it->second;
               
            }
        }
        catch (...)
        {
            // TODO: @@@
        }

        mutexDeviceInformationMap.unlock();
        
        sigDeviceInformationChanged.fire(deviceInfo);

    }


    void RaumserverInstaller::test()
    {
        

        // TODO: connect to signal
        DeviceInformation      devInfo;

        devInfo.name = "Tetsdevice";
        devInfo.ip = "10.0.0.1";
        devInfo.sshAccess = SSHAccess::SSH_YES;
        devInfo.type = DeviceType::DT_UPNPDEVICE_RAUMFELD;

        startInstallToDevice(devInfo);
     
              
    }

}

