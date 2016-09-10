
#include "deviceInstaller/deviceInstaller_RF.h"


namespace RaumserverInstaller
{
    namespace DeviceInstaller
    {

        DeviceInstaller_RaumfeldDevice::DeviceInstaller_RaumfeldDevice() : DeviceInstaller()
        {
            abortInstallThread = false;
            fileCopyPercentage = 0;

            binaryDir = "";            
            installDir = remoteInstallationPath;
            installDirStartScript = remoteInstallationPathInitScript;

            connections.connect(sshClient.scp.sigStartFileCopying, this, &DeviceInstaller_RaumfeldDevice::onStartFileCopying);
            connections.connect(sshClient.scp.sigFileCopying, this, &DeviceInstaller_RaumfeldDevice::onFileCopying);
            connections.connect(sshClient.scp.sigEndFileCopying, this, &DeviceInstaller_RaumfeldDevice::onEndFileCopying);
        }


        DeviceInstaller_RaumfeldDevice::~DeviceInstaller_RaumfeldDevice()
        {
            abortRemove();
            abortInstall();
            connections.disconnect_all();
        }


        void DeviceInstaller_RaumfeldDevice::startInstall()
        {
            DeviceInstaller::startInstall();

            sshClient.setLogObject(getLogObject());
            sshClient.scp.setLogObject(getLogObject());

            abortRemove();
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


        void DeviceInstaller_RaumfeldDevice::startRemove()
        {
            DeviceInstaller::startRemove();

            sshClient.setLogObject(getLogObject());
            sshClient.scp.setLogObject(getLogObject());

            abortRemove();
            abortInstall();

            removeThreadObject = std::thread(&DeviceInstaller_RaumfeldDevice::removeThread, this);
        }


        void DeviceInstaller_RaumfeldDevice::abortRemove()
        {
            DeviceInstaller::abortRemove();

            abortRemoveThread = true;
            if (removeThreadObject.joinable())
            {
                logDebug("Waiting for uninstall thread to abort!", CURRENT_POSITION);
                removeThreadObject.join();
            }
            abortRemoveThread = false;
        }


        void DeviceInstaller_RaumfeldDevice::onFileCopying(std::string _filename, std::uint64_t _copiedSize, std::uint64_t _size)
        {
        }


        void DeviceInstaller_RaumfeldDevice::onStartFileCopying(std::string _filename, std::uint64_t _size)
        {            
            progressInfo("Copying file: " + _filename, CURRENT_POSITION);
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
                sigInstallDone.fire(DeviceInstallerProgressInfo(progressType, "Device '" + deviceInformation.name + "' not compatible with installer!", (std::uint8_t)progressPercentage, true));
                return;
            }

            if (deviceInformation.ip.empty())
            {
                progressError("Device '" + deviceInformation.name + "' has no IP!", CURRENT_POSITION);
                sigInstallDone.fire(DeviceInstallerProgressInfo(progressType, "Device '" + deviceInformation.name + "' has no IP!", (std::uint8_t)progressPercentage, true));
                return;
            }                     

            // Download new version if present!           
            if (!getActualBinaries())
            {
                progressError("Retrieving new binaries for RaumserverDaemon failed!", CURRENT_POSITION);
                return;
            }

            // Get correct binary dir (Raumfeld devices have different architectures)           
            binaryDir = getDeviceBinaryPath();
            if (binaryDir.empty())
            {
                progressError("No valid binary found for device " + deviceInformation.name + " (" + deviceInformation.ip + ")", CURRENT_POSITION);
                return;
            }            

            if (abortInstallThread)
                return;

            progressInfo("Try to establish SSH and SCP connection with device " + deviceInformation.name + " (" + deviceInformation.ip + ")", CURRENT_POSITION);

            sshClient.setOption(ssh_options_e::SSH_OPTIONS_HOST, deviceInformation.ip);

            // The authentication (user and password) is the same on every device
            sshClient.setAuth(sshUser, sshPassword);
           
            if (!sshClient.connectSSH())
            {
                progressError("Could not connect to Device! (SSH)", CURRENT_POSITION);
                sigInstallDone.fire(DeviceInstallerProgressInfo(progressType, "Could not connect to Device! (SSH)", (std::uint8_t)progressPercentage, true));
                return;
            }
                         
            if (!sshClient.connectSCP("..", false))
            {
                progressError("Could not connect to Device! (SCP)", CURRENT_POSITION);
                sigInstallDone.fire(DeviceInstallerProgressInfo(progressType, "Could not connect to Device! (SSH)", (std::uint8_t)progressPercentage, true));
                return;
            }      

            if (abortInstallThread)
                return;

            std::vector<std::string> remoteConsoleLines;

            progressPercentage = 10;
            progressInfo("Connected to device (SSH/SCP)", CURRENT_POSITION);

            // stop raumserver deamon if is running   
            progressInfo("Stopping Raumserver on device! Please wait...", CURRENT_POSITION);
            std::string returnDataStopDaemon;
            sshClient.executeCommand("/bin/sh /etc/init.d/S99raumserver stop", returnDataStopDaemon);
            remoteConsoleLines = Tools::StringUtil::explodeString(returnDataStopDaemon, "\n");
            for (auto it : remoteConsoleLines)
            {
                if (!it.empty())
                    progressInfo("REMOTE: " + it, CURRENT_POSITION);
            }                      

            progressInfo("Copying files to remote device...", CURRENT_POSITION);

            // for a simple progress we do count the number of files and do divide them by the percentage value whcih is left (70 for now)
            TinyDirCpp::TinyDirCpp  tinyDirCpp;    
            auto filesToCopy = tinyDirCpp.getFiles(binaryDir);
            if (filesToCopy.size() == 0)
            {
                progressError("Could not find Raumserver binaries for installing!", CURRENT_POSITION);
                sigInstallDone.fire(DeviceInstallerProgressInfo(progressType, "Could not find Raumserver binaries for installing!", (std::uint8_t)progressPercentage, true));
                return;
            }

            fileCopyPercentage = (70 / filesToCopy.size());

            // copy the files. Due the installation is in a thread we can do a 'sync' copy 
            // aborting may not be possible now because of 'sync' call but we may accept this for now
            // (so 'abortInstallThread' has no funkcion right now)
            sshClient.scp.copyDir(binaryDir, installDir, true, true);
                   
            progressPercentage = 80;

            // copy init script
            sshClient.scp.copyFile(binaryDir + "S99raumserver", installDirStartScript + "S99raumserver");
            sshClient.scp.setChmod(installDirStartScript + "S99raumserver", S_IRWXU | S_IRWXG | S_IRWXO);

            progressPercentage = 85;

            // start raumserver    
            /*
            progressInfo("Starting Raumserver on device! Please wait...", CURRENT_POSITION);
            std::string returnDataStartDaemon;            
            sshClient.executeCommand("/bin/sh /etc/init.d/S99raumserver start", returnDataStartDaemon);             
            remoteConsoleLines = Tools::StringUtil::explodeString(returnDataStartDaemon, "\n");
            for (auto it : remoteConsoleLines)
            {
                if (!it.empty())
                    progressInfo("REMOTE: " + it, CURRENT_POSITION);
            }
            */

            progressInfo("Closing SCP connection", CURRENT_POSITION);
            sshClient.closeSCP();

            // reboot raumserver                
            progressInfo("Trying to reboot device! Please wait...", CURRENT_POSITION);
            std::string returnDataStartDaemon;

            sshClient.executeCommand("reboot", returnDataStartDaemon);
//            progressInfo("Closing SSH connection", CURRENT_POSITION);
            sshClient.closeSSH();

            // wait 4,5 seconds and then try if device is reachable. if not reboot was sucsess, otherwise give info to user that a manuel restart has to be done!
            std::this_thread::sleep_for(std::chrono::milliseconds(4500));
            if(sshClient.connectSSH())
                progressError("Reboot Failed! Please restart device manually!", CURRENT_POSITION);
            else
                progressInfo("Device is now rebooting!", CURRENT_POSITION);   
            sshClient.closeSSH();

            progressPercentage = 100;
                  

            progressInfo("Installation done!", CURRENT_POSITION);
            sigInstallDone.fire(DeviceInstallerProgressInfo(progressType, "Installation done!", (std::uint8_t)progressPercentage, false));
        }


        void DeviceInstaller_RaumfeldDevice::removeThread()
        {
            bool hasError = false;

            // the device we do install the raumserver on with this installer object has to be a raumfeld device!
            if (deviceInformation.type != DeviceType::DT_UPNPDEVICE_RAUMFELD)
            {
                progressError("Device '" + deviceInformation.name + "' not compatible with installer!", CURRENT_POSITION);
                sigInstallDone.fire(DeviceInstallerProgressInfo(progressType, "Device '" + deviceInformation.name + "' not compatible with installer!", (std::uint8_t)progressPercentage, true));
                return;
            }

            if (deviceInformation.ip.empty())
            {
                progressError("Device '" + deviceInformation.name + "' has no IP!", CURRENT_POSITION);
                sigInstallDone.fire(DeviceInstallerProgressInfo(progressType, "Device '" + deviceInformation.name + "' has no IP!", (std::uint8_t)progressPercentage, true));
                return;
            }

            progressInfo("Try to establish SSH and SCP connection with device " + deviceInformation.name + " (" + deviceInformation.ip + ")", CURRENT_POSITION);

            sshClient.setOption(ssh_options_e::SSH_OPTIONS_HOST, deviceInformation.ip);

            // The authentication (user and password) is the same on every device
            sshClient.setAuth(sshUser, sshPassword);

            if (!sshClient.connectSSH())
            {
                progressError("Could not connect to Device! (SSH)", CURRENT_POSITION);
                sigInstallDone.fire(DeviceInstallerProgressInfo(progressType, "Could not connect to Device! (SSH)", (std::uint8_t)progressPercentage, true));
                return;
            }

            if (!sshClient.connectSCP("..", false))
            {
                progressError("Could not connect to Device! (SCP)", CURRENT_POSITION);
                sigInstallDone.fire(DeviceInstallerProgressInfo(progressType, "Could not connect to Device! (SSH)", (std::uint8_t)progressPercentage, true));
                return;
            }

            progressPercentage = 10;
            progressInfo("Connected to device (SSH/SCP)", CURRENT_POSITION);
            // stop raumserver deamon if is running   
            progressInfo("Stopping Raumserver on device! Please wait...", CURRENT_POSITION);
            std::string returnDataStopDaemon;
            sshClient.executeCommand("/bin/sh /etc/init.d/S99raumserver stop", returnDataStopDaemon);
            auto remoteConsoleLines = Tools::StringUtil::explodeString(returnDataStopDaemon, "\n");
            for (auto it : remoteConsoleLines)
            {
                if (!it.empty())
                    progressInfo("REMOTE: " + it, CURRENT_POSITION);
            }

            progressInfo("Removing files from remote device...", CURRENT_POSITION);

            // delete raumserver directory
            progressPercentage = 80;            
            if (!sshClient.scp.removeDir("/" + installDir))
            {
                progressError("Can't delete raumserver install folder!", CURRENT_POSITION);
                hasError = true;
            }
            else
                progressInfo("Raumserver install folder deleted!", CURRENT_POSITION);

            // delete init script            
            if (!sshClient.scp.removeFile("/" + installDirStartScript + "S99raumserver"))
            {
                progressError("Can't delete raumserver start script!", CURRENT_POSITION);
                hasError = true;
            }
            else
                progressInfo("Raumserver start script deleted!", CURRENT_POSITION);
           
            progressPercentage = 100;
            progressInfo("Closing SSH/SCP connection", CURRENT_POSITION);
            sshClient.closeSCP();
            sshClient.closeSSH();

            if (hasError)
            {
                progressError("Uninstall has errors!", CURRENT_POSITION);
                sigInstallDone.fire(DeviceInstallerProgressInfo(progressType, "Uninstall has errors!", (std::uint8_t)progressPercentage, hasError));
            }
            else
            {
                progressInfo("Uninstall done!", CURRENT_POSITION);
                sigInstallDone.fire(DeviceInstallerProgressInfo(progressType, "Uninstall done!", (std::uint8_t)progressPercentage, hasError));
            }
        }


    }
}