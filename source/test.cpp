
#include <raumserverInstaller.h>
#include <rlutil/rlutil.h>


int main()
{
    RaumserverInstaller::RaumserverInstaller    raumServerInstaller;

    raumServerInstaller.init();
    raumServerInstaller.initLogObject(Raumkernel::Log::LogType::LOGTYPE_INFO);
    raumServerInstaller.initDiscover();

    
    auto adapterList = raumServerInstaller.getNetworkAdapterList();
    /*
    for (auto item : adapterList)
    {
        std::cout << item.name + " - " + item.fullName;
        std::cout << std::endl;
    } 
    */

    raumServerInstaller.setNetworkAdapter(adapterList[3]);
    raumServerInstaller.startDiscoverDevicesForInstall();    

    rlutil::getkey();

    RaumserverInstaller::DeviceInformation deviceInfo;

    deviceInfo.name = "Dummmy";
    deviceInfo.ip = "10.0.0.1";
    deviceInfo.type = RaumserverInstaller::DeviceType::DT_UPNPDEVICE_RAUMFELD;

    raumServerInstaller.startInstallToDevice(deviceInfo);

    rlutil::getkey();

    raumServerInstaller.startInstallToDevice(deviceInfo);

    rlutil::getkey();

    return 0;
}





