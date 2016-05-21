
#include "deviceInstaller/deviceInstaller.h"


namespace RaumserverInstaller
{
    namespace DeviceInstaller
    {

        DeviceInstaller::DeviceInstaller() : RaumserverInstallerBase()
        {
            progressPercentage = 0;
            deviceInstallerFilePath = "installBuild.xml";
            progressType = DeviceInstallerProgressType::DIPT_INSTALL;   
            raumserverDaemonUpdater.init();
        }


        DeviceInstaller::~DeviceInstaller()
        {
        }


        void DeviceInstaller::startInstall()
        {
            progressType = DeviceInstallerProgressType::DIPT_INSTALL;
            loadDeviceInstallerInfoFile();
        }


        void DeviceInstaller::abortInstall()
        {
        }


        void DeviceInstaller::startRemove()
        {
            progressType = DeviceInstallerProgressType::DIPT_REMOVE;
        }


        void DeviceInstaller::abortRemove()
        {
        }


        void DeviceInstaller::setDevice(const DeviceInformation &_deviceInfo)
        {
            deviceInformation = _deviceInfo;
        }


        void DeviceInstaller::progressDebug(const std::string &_progressInfo, const std::string &_location)
        {
            logDebug(_progressInfo, _location);                     
            sigInstallProgress.fire(DeviceInstallerProgressInfo(progressType, _progressInfo, (std::uint8_t)progressPercentage, false));
        }


        void DeviceInstaller::progressWarning(const std::string &_progressInfo, const std::string &_location)
        {
            logWarning(_progressInfo, _location);
            sigInstallProgress.fire(DeviceInstallerProgressInfo(progressType, _progressInfo, (std::uint8_t)progressPercentage, false));
        }


        void DeviceInstaller::progressInfo(const std::string &_progressInfo, const std::string &_location)
        {
            logInfo(_progressInfo, _location);
            sigInstallProgress.fire(DeviceInstallerProgressInfo(progressType, _progressInfo, (std::uint8_t)progressPercentage, false));
        }


        void DeviceInstaller::progressError(const std::string &_progressInfo, const std::string &_location)
        {
            logError(_progressInfo, _location);
            sigInstallProgress.fire(DeviceInstallerProgressInfo(progressType, _progressInfo, (std::uint8_t)progressPercentage, true));
        }


        std::string DeviceInstaller::getDeviceBinaryPath()
        {
            auto it = hardwareTypeBinaries.find(std::stoi(deviceInformation.hardwareType));
            if (it == hardwareTypeBinaries.end())
            {
                logWarning("No valid binary found for device " + deviceInformation.friendlyName + "! Using standard ARMv7 binary!", CURRENT_POSITION);
                it = hardwareTypeBinaries.find(0);
            }
            if (it == hardwareTypeBinaries.end())
            {
                logError("No binary found for device " + deviceInformation.friendlyName, CURRENT_POSITION);
                return "";
            }
            
            return it->second;
        }

        
        void DeviceInstaller::loadDeviceInstallerInfoFile()
        {           
            pugi::xml_document doc;
            pugi::xml_node rootNode, installSourceNode;            

            pugi::xml_parse_result result = doc.load_file(deviceInstallerFilePath.c_str());

            hardwareTypeBinaries.clear();

            rootNode = doc.child("build");
            if (!rootNode)
            {
                logError("Wrong formated file '" + deviceInstallerFilePath + "'! Missing 'build' node!", CURRENT_POSITION);
                throw std::runtime_error("Unrecoverable Error! Please check Log file!");
            }

            installSourceNode = rootNode.child("installSource");
            if (!installSourceNode)
            {
                logError("Wrong formated file '" + deviceInstallerFilePath + "'! Missing 'installSource' node!", CURRENT_POSITION);
                throw std::runtime_error("Unrecoverable Error! Please check Log file!");
            }

            // loop through devices and store results in map
            for (auto it : installSourceNode.children())
            {
                try
                {                    
                    auto attribute = it.attribute("hardwareType");
                    if (attribute)
                    {
                        if (attribute.value() && it.value())
                            hardwareTypeBinaries.insert(std::make_pair(std::stoi(attribute.value()), std::string(it.child_value())));
                    }
                    
                }
                catch ( ... )
                {
                    logError("Wrong formated devive node in '" + deviceInstallerFilePath + "'!", CURRENT_POSITION);
                }
            }            
        }


        bool DeviceInstaller::getActualBinaries()
        {         
            raumserverDaemonUpdater.setLogObject(getLogObject());
            raumserverDaemonUpdater.run(true, false);

            // TODO: @@@
            return true;
        }
 



        DeviceInstallerProgressInfo::DeviceInstallerProgressInfo(DeviceInstallerProgressType _actionType,  const std::string &_info, const std::uint8_t &_completionPercentage, const bool &_error)
        {
            actionType = _actionType;
            info = _info;
            completionPercentage = _completionPercentage;
            error = _error;
        }

        EXPORT Json::Value DeviceInstallerProgressInfo::getJsonValue()
        {
            Json::Value progressInfo;
            progressInfo["progressInfo"]["info"] = info;
            progressInfo["progressInfo"]["actionType"] = actionType;
            progressInfo["progressInfo"]["percentage"] = completionPercentage;
            progressInfo["progressInfo"]["error"] = error;
            return progressInfo;
        }
      

    }
}