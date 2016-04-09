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
        // the object will create a device information where we do subscribe on add
        connections.connect(deviceDiscoveryUPNP.sigDeviceFound, this, &RaumserverInstaller::onDeviceFound);
        connections.connect(deviceDiscoveryUPNP.sigDeviceRemoved, this, &RaumserverInstaller::onDeviceRemoved);
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

        // TODO: Start for check ssh access!
    
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


    void RaumserverInstaller::test()
    {
        // http://api.libssh.org/master/libssh_tutor_guided_tour.html
        ssh_session my_ssh_session;
        int rc;
        char *password;
        // Open session and set options
        my_ssh_session = ssh_new();
        if (my_ssh_session == NULL)
            exit(-1);
        ssh_options_set(my_ssh_session, SSH_OPTIONS_HOST, "10.0.0.1");
        // Connect to server
        rc = ssh_connect(my_ssh_session);
        if (rc != SSH_OK)
        {
            fprintf(stderr, "Error connecting to localhost: %s\n",
                ssh_get_error(my_ssh_session));
            ssh_free(my_ssh_session);
            exit(-1);
        }
        // Verify the server's identity
        // For the source code of verify_knowhost(), check previous example
        /*
        if (verify_knownhost(my_ssh_session) < 0)
        {
            ssh_disconnect(my_ssh_session);
            ssh_free(my_ssh_session);
            exit(-1);
        }
        */

        // Authenticate ourselves
        password = "";
        rc = ssh_userauth_password(my_ssh_session, "root", password);
        if (rc != SSH_AUTH_SUCCESS)
        {
            logError(ssh_get_error(my_ssh_session), CURRENT_POSITION);
            fprintf(stderr, "Error authenticating with password: %s\n",
                ssh_get_error(my_ssh_session));
            ssh_disconnect(my_ssh_session);
            ssh_free(my_ssh_session);
            exit(-1);
        }

        // TODO: @@@
    
        ssh_disconnect(my_ssh_session);
        ssh_free(my_ssh_session);
    }

}

