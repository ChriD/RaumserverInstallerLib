
#include "deviceInstaller/deviceInstaller_RF.h"


namespace RaumserverInstaller
{
    namespace DeviceInstaller
    {

        DeviceInstaller_RaumfeldDevice::DeviceInstaller_RaumfeldDevice() : DeviceInstaller()
        {
            abortInstallThread = false;
            fileCopyPercentage = 0;

            binaryDir = "binaries/raumserverDaemon/";
            installDir = "raumserverDaemon/";

            connections.connect(sshClient.sftp.sigEndFileCopying, this, &DeviceInstaller_RaumfeldDevice::onStartFileCopying);
            connections.connect(sshClient.sftp.sigFileCopying, this, &DeviceInstaller_RaumfeldDevice::onFileCopying);
            connections.connect(sshClient.sftp.sigEndFileCopying, this, &DeviceInstaller_RaumfeldDevice::onEndFileCopying);
        }


        DeviceInstaller_RaumfeldDevice::~DeviceInstaller_RaumfeldDevice()
        {
            abortInstall();
        }


        void DeviceInstaller_RaumfeldDevice::startInstall()
        {
            DeviceInstaller::startInstall();

            sshClient.setLogObject(getLogObject());
            sshClient.sftp.setLogObject(getLogObject());           

            abortInstall();

            installThreadObject = std::thread(&DeviceInstaller_RaumfeldDevice::installThread, this);
        }


        void DeviceInstaller_RaumfeldDevice::abortInstall()
        {
            DeviceInstaller::abortInstall();            

            abortInstallThread = true;
            if (installThreadObject.joinable())
            {
                logDebug("Waiting for install thread to abort!", CURRENT_POSITION);
                installThreadObject.join();
            }
            abortInstallThread = false;
        }


        void DeviceInstaller_RaumfeldDevice::onFileCopying(std::string _filename, std::uint64_t _copiedSize, std::uint64_t _size)
        {
        }


        void DeviceInstaller_RaumfeldDevice::onStartFileCopying(std::string _filename, std::uint64_t _size)
        {            
            progressInfo("Copying File " + _filename, CURRENT_POSITION);
            progressPercentage += fileCopyPercentage;
        }


        void DeviceInstaller_RaumfeldDevice::onEndFileCopying(std::string _filename, std::uint64_t _size)
        {           
        }


        void DeviceInstaller_RaumfeldDevice::installThread()
        {
            // the device we do install the raumserver on with this installer object has to be a raumfeld device!
            if (deviceInformation.type != DeviceType::DT_UPNPDEVICE_RAUMFELD)
            {
                progressError("Device '" + deviceInformation.name + "' not compatible with installer!", CURRENT_POSITION);
                return;
            }

            if (deviceInformation.ip.empty())
            {
                progressError("Device '" + deviceInformation.name + "' has no IP!", CURRENT_POSITION);
                return;
            }

            progressInfo("Try to establish SSH and SFTP connection with device " + deviceInformation.name + " (" + deviceInformation.ip + ")", CURRENT_POSITION);

            sshClient.setOption(ssh_options_e::SSH_OPTIONS_HOST, deviceInformation.ip);

            // The authentication (user and password) is the same on every device
            sshClient.setAuth("root", "");
           
            if (!sshClient.connectSSH())
            {
                progressError("Could not connect to Device! (SSH)", CURRENT_POSITION);
                sigInstallDone.fire(DeviceInstallerProgressInfo("Could not connect to Device! (SSH)", (std::uint8_t)progressPercentage, true));
                return;
            }
                         
            if (!sshClient.connectSFTP())
            {
                progressError("Could not connect to Device! (SFTP)", CURRENT_POSITION);
                sigInstallDone.fire(DeviceInstallerProgressInfo("Could not connect to Device! (SSH)", (std::uint8_t)progressPercentage, true));
                return;
            }

            // TODO: stop raumserver deamon if is running

            progressPercentage = 10;
            progressInfo("Connected to device (SSH/SFTP)", CURRENT_POSITION);
            progressInfo("Copying files to remote device...", CURRENT_POSITION);

            // for a simple progress we do count the number of files and do divide them by the percentage value whcih is left (70 for now)
            TinyDirCpp::TinyDirCpp  tinyDirCpp;    
            auto filesToCopy = tinyDirCpp.getFiles(binaryDir);
            if (filesToCopy.size() == 0)
            {
                progressError("Could not find Raumserver binaries for installing!", CURRENT_POSITION);
                sigInstallDone.fire(DeviceInstallerProgressInfo("Could not find Raumserver binaries for installing!", (std::uint8_t)progressPercentage, true));
                return;
            }

            fileCopyPercentage = (70 / filesToCopy.size());

            // copy the files. Due the installation is in a thread we can do a 'sync' copy 
            // abporting may not be possible now because of 'sync' call but we may accept this for now
            // (so 'abortInstallThread' has no funkcion right now)
            sshClient.sftp.copyDir(binaryDir, installDir, true, true);
       
            

            progressPercentage = 80;

            // TODO: copy init script

            progressPercentage = 85;

            // TODO: start raumserver (if no reboot)

            // TODO: Then check if port of Raumserver is open
            // while loop always adding one percentage???

            progressPercentage = 100;
            progressInfo("Closing SSH/SFTP connection", CURRENT_POSITION);
            sshClient.closeSFTP();
            sshClient.closeSSH();

            progressInfo("Installation done!", CURRENT_POSITION);
            sigInstallDone.fire(DeviceInstallerProgressInfo("Installation done!", (std::uint8_t)progressPercentage, false));
        }


    }
}