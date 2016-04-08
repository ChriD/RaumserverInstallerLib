
#include <raumserverInstaller.h>
#include <rlutil/rlutil.h>


int main()
{
    RaumserverInstaller::RaumserverInstaller    raumServerInstaller;

    raumServerInstaller.init();

    auto adapterList = raumServerInstaller.getNetworkAdapterList();
    for (auto item : adapterList)
    {
        std::cout << item.name + " - " + item.fullName;
        std::cout << std::endl;
    }

    rlutil::getkey();

    raumServerInstaller.startDiscoverDevicesForInstall();

    rlutil::getkey();

    return 0;
}





