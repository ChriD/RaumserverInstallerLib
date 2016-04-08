
#include <raumserverInstaller.h>
#include <rlutil/rlutil.h>


int main()
{
    RaumserverInstaller::RaumserverInstaller    raumServerInstaller;

    raumServerInstaller.init();
    raumServerInstaller.startDiscoverDevicesForInstall();

    rlutil::getkey();

    return 0;
}





