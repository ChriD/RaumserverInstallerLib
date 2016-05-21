#include <updater/updater_RaumserverDaemon.h>

namespace Updater
{

    Updater_RaumserverDaemon::Updater_RaumserverDaemon() : Updater()
    {
        // TODO: get from installBuild.xml
        source = "http://bassmaniacs.com/data/binaries.zip";
        destination = "binaries/";
        downloadDestination = "binaries/binaries.zip";
    }


    Updater_RaumserverDaemon::~Updater_RaumserverDaemon()
    {
    }


    bool Updater_RaumserverDaemon::doGetBinary()
    {                       
        //return doGetBinary_HTTP();     
        return true;
    }


    bool Updater_RaumserverDaemon::doUpdate()
    {        
        // extract the downloaded ZIP file into the binaries folder
        // then we do not have to do anything here because the installer will update the binaries on the related device
        if (extractFile(downloadDestination, destination))
        {
            progressInfo("RaumserverDaemon binaries been updated sucessfully!", CURRENT_POSITION);
            return true;
        }

        progressError("Error while updating RaumserverDaemon binaries!", CURRENT_POSITION);
        return false;
    }

}