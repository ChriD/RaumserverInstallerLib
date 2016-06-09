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
        httpClient.abortAllRequests();
        deviceInstaller.abortInstall();
    }


    void RaumserverInstaller::init()
    {  
        connections.connect(deviceDiscoveryUPNP.sigDeviceFound, this, &RaumserverInstaller::onDeviceFound);
        connections.connect(deviceDiscoveryUPNP.sigDeviceRemoved, this, &RaumserverInstaller::onDeviceRemoved);

        connections.connect(deviceInstaller.sigInstallProgress, this, &RaumserverInstaller::onInstallProgress);
        connections.connect(deviceInstaller.sigInstallDone, this, &RaumserverInstaller::onInstallDone);
        
        httpClient.init();        
    }


    VersionInfo::VersionInfo RaumserverInstaller::getVersionInfo()
    {
        return versionInfo;
    }


    void RaumserverInstaller::initLogObject(Log::LogType _defaultLogLevel, const std::string &_logFilePath, const std::vector<std::shared_ptr<Log::LogAdapter>> &_adapterList)
    {
        // create the log object (if not already provided) which will be used throughout the whole kernel and his modules                
        logObject = std::shared_ptr<Log::Log>(new Log::Log());

        // we do register some log adapters for the log object with standard values and set to log only ERRORS and CRITICALERRORS
        // the log level itself will be set after reading the settings file but we want to have logging while reading settings file
        // so we create the logObject with some standard values if they are not given
        if (_adapterList.empty())
        {
            auto logAdapterConsole = std::shared_ptr<Log::LogAdapter_Console>(new Log::LogAdapter_Console());
            logObject->registerAdapter(logAdapterConsole);

            auto logAdapterFile = std::shared_ptr<Log::LogAdapter_File>(new Log::LogAdapter_File());
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
        
        httpClient.setLogObject(logObject);
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


    DeviceInformation RaumserverInstaller::getDeviceInformation(std::string _ip)
    {
        DeviceInformation deviceInfo;
        std::unique_lock<std::mutex> lock(mutexDeviceInformationMap);
        auto it = deviceInformationMap.find(_ip);
        if (it != deviceInformationMap.end())
            deviceInfo = it->second;
        return deviceInfo;
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

        // we do use the standard port of the raumserver (8080)
        httpClient.request("http://" + _deviceInformation.ip + ":8080/raumserver/data/getVersion", nullptr, nullptr, this, std::bind(&RaumserverInstaller::onRequestResult, this, std::placeholders::_1));
    }


    void RaumserverInstaller::onDeviceRemoved(DeviceInformation _deviceInformation)
    {
        try
        {
            // lock map when inserting a value 
            mutexDeviceInformationMap.lock();
            try
            {
                auto it = deviceInformationMap.find(_deviceInformation.ip);
                if (it != deviceInformationMap.end())
                {
                    deviceInformationMap.erase(_deviceInformation.ip);
                }
            }
            catch (...)
            {
                logError("Exception ocurred while deleting device to map!", CURRENT_POSITION);
            }
            mutexDeviceInformationMap.unlock();
        }
        // be sure we will find recursive locks (deadlocks) created by the application!
        catch (...)
        {
            logError("Exception ocurred while locking device map!", CURRENT_POSITION);
        }

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
        auto networkAdapterList = deviceDiscoveryUPNP.getNetworkAdaptersInformation();
        return networkAdapterList;
    }


    NetworkAdaperInformation RaumserverInstaller::getNetworkAdapterInformation(std::uint16_t _id)
    {
        NetworkAdaperInformation empty;

        auto adapterInfoList = deviceDiscoveryUPNP.getNetworkAdaptersInformation();
        for (auto adpterInfo : adapterInfoList)
        {
            if (adpterInfo.id == _id)
                return adpterInfo;
        }
        return empty;
    }


    void RaumserverInstaller::setNetworkAdapter(NetworkAdaperInformation _networkAdapterInformation)
    {
        deviceDiscoveryUPNP.setNetworkAdapter(_networkAdapterInformation);
    }
   

    void RaumserverInstaller::startInstallToDevice(DeviceInformation _deviceInformation)
    {    
        try
        {
            deviceInstaller.setLogObject(getLogObject());
            deviceInstaller.setDevice(_deviceInformation);
            deviceInstaller.startInstall();
        }
        catch ( ... )
        {
            logError("Error while installing!", CURRENT_POSITION);            
        }
    }

    void RaumserverInstaller::startRemoveFromDevice(DeviceInformation _deviceInformation)
    {
        try
        {
            deviceInstaller.setLogObject(getLogObject());
            deviceInstaller.setDevice(_deviceInformation);
            deviceInstaller.startRemove();
        }
        catch (...)
        {
            logError("Error while removing!", CURRENT_POSITION);
        }
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
        SSHClient::SSHClient sshClient;

        if (stopSSHAccessCheckThreads)
            return;

        sshClient.setLogObject(getLogObject());

        try
        {            
            sshClient.setOption(ssh_options_e::SSH_OPTIONS_HOST, _ip);
            sshClient.setAuth("root", "");
            if (sshClient.connectSSH())
            {
                if (sshClient.connectSFTP())
                {
                    hasSFTPAccess = true;
                }
            }
        }
        catch (std::exception &e)
        {
            logError("Exception when checking SSHAccess for: " + _ip, CURRENT_POSITION);
            logError(e.what(), CURRENT_POSITION);            
        }
        catch (std::string &e)
        {
            logError("Exception when checking SSHAccess for: " + _ip, CURRENT_POSITION);
            logError(e, CURRENT_POSITION);            
        }
        catch (...)
        {
            logError("Unknown exception when checking SSHAccess for: " + _ip, CURRENT_POSITION);            
        }

        mutexDeviceInformationMap.lock();

        try
        {

            auto it = deviceInformationMap.find(_ip);
            if (it != deviceInformationMap.end())
            {
                auto access = hasSFTPAccess ? "yes" : "no";
                logInfo("SSH Access for " + it->second.name + " (" + it->second.ip + "): " + access, CURRENT_POSITION);               

                it->second.sshAccess = hasSFTPAccess ? UnknownYesNo::YES : UnknownYesNo::NO;

                // check if the raumserver is installed by checking if the folder is there
                if (hasSFTPAccess)
                {
                    if (sshClient.sftp.existsFile(remoteInstallationPath.substr(0, remoteInstallationPath.length() - 1)) && 
                        sshClient.sftp.existsFile(remoteInstallationPathInitScript + "S99raumserver"))
                        it->second.raumserverInstalled = UnknownYesNo::YES;
                    else
                        it->second.raumserverInstalled = UnknownYesNo::NO;
                }

                // get a copy of the info struct
                deviceInfo = it->second;
               
            }
        }
        catch (std::exception &e)
        {
            logError("Exception when updateing raumserverInstalled info for: " + _ip, CURRENT_POSITION);
            logError(e.what(), CURRENT_POSITION);
        }
        catch (std::string &e)
        {
            logError("Exception when updateing raumserverInstalled info for: " + _ip, CURRENT_POSITION);
            logError(e, CURRENT_POSITION);
        }
        catch (...)
        {
            logError("Unknown exception when when updateing raumserverInstalled info for: " + _ip, CURRENT_POSITION);
        }

        mutexDeviceInformationMap.unlock();
        
        sigDeviceInformationChanged.fire(deviceInfo);
    }   


    void RaumserverInstaller::onRequestResult(HttpClient::HttpRequest *_request)
    {
        DeviceInformation deviceInfo;
        std::string ip;
        
        mutexDeviceInformationMap.lock();

        try
        {
            
            LUrlParser::clParseURL url = LUrlParser::clParseURL::ParseURL(_request->getRequestUrl());
            ip = url.m_Host;

            auto it = deviceInformationMap.find(ip);
            if (it != deviceInformationMap.end())
            {                
                auto access = !_request->getResponse()->getErrorCode() ? "yes" : "no";
                logInfo("Is Raumserver running for " + it->second.name + " (" + it->second.ip + "): " + access, CURRENT_POSITION);

                it->second.raumserverRuns = !_request->getResponse()->getErrorCode() ? UnknownYesNo::YES : UnknownYesNo::NO;
               
                if (it->second.raumserverRuns == UnknownYesNo::YES && _request->getResponse()->getData().size())
                {              
                    Json::Value rootValue;
                    Json::Reader jsonReader;

                    //auto jsonString = std::string(_request->getResponse()->getData().data());
                    auto responseData = _request->getResponse()->getData();
                    auto jsonString = std::string(responseData.begin(), responseData.end());

                    if (jsonReader.parse(jsonString, rootValue, false))
                    {      
                        auto versionInfoNode = rootValue["versionInfo"];
                        if (!versionInfoNode.isNull())
                        {
                            auto raumserverLibNode = versionInfoNode["raumserverLib"];
                            if (!raumserverLibNode.isNull())
                                it->second.raumserverVersion = raumserverLibNode.asString();
                        }                    
                    }                   
                }

                // get a copy of the info struct
                deviceInfo = it->second;
            }
            
        }
        catch (std::exception &e)
        {
            logError("Exception when updateing IsRunning info for: " + ip, CURRENT_POSITION);
            logError(e.what(), CURRENT_POSITION);
        }
        catch (std::string &e)
        {
            logError("Exception when updateing IsRunning info for: " + ip, CURRENT_POSITION);
            logError(e, CURRENT_POSITION);
        }
        catch (...)
        {
            logError("Unknown Exception when updateing IsRunning info for: " + ip, CURRENT_POSITION);
        }

        mutexDeviceInformationMap.unlock();

        sigDeviceInformationChanged.fire(deviceInfo);


    }


}

