
#include "deviceInstaller/deviceInstaller.h"


namespace RaumserverInstaller
{
    namespace DeviceInstaller
    {

        DeviceInstaller::DeviceInstaller() : RaumserverInstallerBase()
        {
            progressPercentage = 0;
            progressType = DeviceInstallerProgressType::DIPT_INSTALL;
        }


        DeviceInstaller::~DeviceInstaller()
        {
        }


        void DeviceInstaller::startInstall()
        {
            progressType = DeviceInstallerProgressType::DIPT_INSTALL;
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