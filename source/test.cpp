
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

    raumServerInstaller.test();

    rlutil::getkey();

    return 0;
}





