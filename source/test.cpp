
#include <raumserverInstaller.h>
#include <rlutil/rlutil.h>


int main()
{
    RaumserverInstaller::RaumserverInstaller    *raumServerInstaller;

    raumServerInstaller = new RaumserverInstaller::RaumserverInstaller();

    raumServerInstaller->init();
    raumServerInstaller->initLogObject(Log::LogType::LOGTYPE_INFO);
    raumServerInstaller->initDiscover();

       
    auto adapterList = raumServerInstaller->getNetworkAdapterList();
    for (auto item : adapterList)
    {
        std::cout << item.name + " - " + item.fullName;
        std::cout << std::endl;
    }    

    std::int16_t    idx;
    std::cout << "Adapter IDX: ";
    std::cin >> idx;

    raumServerInstaller->setNetworkAdapter(adapterList[idx]);
    raumServerInstaller->startDiscoverDevicesForInstall();    

    rlutil::getkey();

    RaumserverInstaller::DeviceInformation deviceInfo;

    deviceInfo.name = "Dummmy";
    deviceInfo.ip = "10.0.0.4";
    deviceInfo.type = RaumserverInstaller::DeviceType::DT_UPNPDEVICE_RAUMFELD;

    //raumServerInstaller->startInstallToDevice(deviceInfo);

    //rlutil::getkey();

    deviceInfo.ip = "10.0.0.8";
    deviceInfo.hardwareType = "3";
    raumServerInstaller->startInstallToDevice(deviceInfo);

    rlutil::getkey();

    delete raumServerInstaller;
    raumServerInstaller = new RaumserverInstaller::RaumserverInstaller();

    raumServerInstaller->startRemoveFromDevice(deviceInfo);

    rlutil::getkey();

    delete raumServerInstaller;

    return 0;
}





