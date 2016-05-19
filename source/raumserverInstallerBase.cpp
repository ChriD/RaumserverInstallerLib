
#include "raumserverInstallerBase.h"


namespace RaumserverInstaller
{

    RaumserverInstallerBase::RaumserverInstallerBase() : Log::LogBase()
    {
    }


    RaumserverInstallerBase::~RaumserverInstallerBase()
    {
    }



    Json::Value DeviceInformation::getJsonValue()
    {
        Json::Value deviceInfo;
        deviceInfo["deviceInfo"]["ip"] = ip;
        deviceInfo["deviceInfo"]["name"] = name;
        deviceInfo["deviceInfo"]["friendlyName"] = friendlyName;
        deviceInfo["deviceInfo"]["udn"] = UDN;
        deviceInfo["deviceInfo"]["sshAccess"] = sshAccess;
        deviceInfo["deviceInfo"]["raumserverInstalled"] = raumserverInstalled;
        deviceInfo["deviceInfo"]["raumserverRuns"] = raumserverRuns;
        deviceInfo["deviceInfo"]["raumserverVersion"] = raumserverVersion;
        deviceInfo["deviceInfo"]["hardwareType"] = hardwareType;
        deviceInfo["deviceInfo"]["modelNumber"] = modelNumber;
        deviceInfo["deviceInfo"]["type"] = type;
        return deviceInfo;
    }

}

