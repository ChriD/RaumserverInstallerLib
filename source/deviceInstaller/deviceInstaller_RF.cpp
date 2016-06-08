
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

            connections.connect(sshClient.sftp.sigStartFileCopying, this, &DeviceInstaller_RaumfeldDevice::onStartFileCopying);
            connections.connect(sshClient.sftp.sigFileCopying, this, &DeviceInstaller_RaumfeldDevice::onFileCopying);
            connections.connect(sshClient.sftp.sigEndFileCopying, this, &DeviceInstaller_RaumfeldDevice::onEndFileCopying);
        }


        DeviceInstaller_RaumfeldDevice::~DeviceInstaller_RaumfeldDevice()
        {
            abortRemove();
            abortInstall();
        }


        void DeviceInstaller_RaumfeldDevice::startInstall()
        {
            DeviceInstaller::startInstall();

            sshClient.setLogObject(getLogObject());
            sshClient.sftp.setLogObject(getLogObject());           

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
            sshClient.sftp.setLogObject(getLogObject());

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

            progressInfo("Try to establish SSH and SFTP connection with device " + deviceInformation.name + " (" + deviceInformation.ip + ")", CURRENT_POSITION);

            sshClient.setOption(ssh_options_e::SSH_OPTIONS_HOST, deviceInformation.ip);

            // The authentication (user and password) is the same on every device
            sshClient.setAuth("root", "");
           
            if (!sshClient.connectSSH())
            {
                progressError("Could not connect to Device! (SSH)", CURRENT_POSITION);
                sigInstallDone.fire(DeviceInstallerProgressInfo(progressType, "Could not connect to Device! (SSH)", (std::uint8_t)progressPercentage, true));
                return;
            }
                         
            if (!sshClient.connectSFTP())
            {
                progressError("Could not connect to Device! (SFTP)", CURRENT_POSITION);
                sigInstallDone.fire(DeviceInstallerProgressInfo(progressType, "Could not connect to Device! (SSH)", (std::uint8_t)progressPercentage, true));
                return;
            }      

            if (abortInstallThread)
                return;

            progressPercentage = 10;
            progressInfo("Connected to device (SSH/SFTP)", CURRENT_POSITION);

            // stop raumserver deamon if is running   
            progressInfo("Stopping Raumserver on device! Please wait...", CURRENT_POSITION);
            std::string returnDataStopDaemon;
            sshClient.executeCommand("/bin/sh /etc/init.d/S99raumserver stop", returnDataStopDaemon);
            progressInfo("REMOTE: " + returnDataStopDaemon, CURRENT_POSITION);

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
            sshClient.sftp.copyDir(binaryDir, installDir, true, true);
                   
            progressPercentage = 80;

            // copy init script
            sshClient.sftp.copyFile(binaryDir + "S99raumserver", installDirStartScript + "S99raumserver");
            sshClient.sftp.setChmod(installDirStartScript + "S99raumserver", S_IRWXU | S_IRWXG | S_IRWXO);

            progressPercentage = 85;

            // start raumserver         
            progressInfo("Starting Raumserver on device! Please wait...", CURRENT_POSITION);
            std::string returnDataStartDaemon;            
            sshClient.executeCommand("/bin/sh /etc/init.d/S99raumserver start", returnDataStartDaemon);   
            progressInfo("REMOTE: " + returnDataStartDaemon, CURRENT_POSITION);

            // TODO: Then check if Raumserver is running (use standard port)
            // while loop always adding one percentage???
            //httpClient.request("http://" + _deviceInformation.ip + ":8080/raumserver/data/getVersion", nullptr, nullptr, this, std::bind(&RaumserverInstaller::onRequestResult, this, std::placeholders::_1));
            // request.wait();

            progressPercentage = 100;
            progressInfo("Closing SSH/SFTP connection", CURRENT_POSITION);
            sshClient.closeSFTP();
            sshClient.closeSSH();            

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

            progressInfo("Try to establish SSH and SFTP connection with device " + deviceInformation.name + " (" + deviceInformation.ip + ")", CURRENT_POSITION);

            sshClient.setOption(ssh_options_e::SSH_OPTIONS_HOST, deviceInformation.ip);

            // The authentication (user and password) is the same on every device
            sshClient.setAuth("root", "");

            if (!sshClient.connectSSH())
            {
                progressError("Could not connect to Device! (SSH)", CURRENT_POSITION);
                sigInstallDone.fire(DeviceInstallerProgressInfo(progressType, "Could not connect to Device! (SSH)", (std::uint8_t)progressPercentage, true));
                return;
            }

            if (!sshClient.connectSFTP())
            {
                progressError("Could not connect to Device! (SFTP)", CURRENT_POSITION);
                sigInstallDone.fire(DeviceInstallerProgressInfo(progressType, "Could not connect to Device! (SSH)", (std::uint8_t)progressPercentage, true));
                return;
            }

            progressPercentage = 10;
            progressInfo("Connected to device (SSH/SFTP)", CURRENT_POSITION);

            // TODO: stop raumserver deamon if is running

            progressInfo("Removing files from remote device...", CURRENT_POSITION);

            // delete raumserver directory
            progressPercentage = 80;            
            if (!sshClient.sftp.removeDir(installDir))
            {
                progressError("Can't delete raumserver install folder!", CURRENT_POSITION);
                hasError = true;
            }
            else
                progressInfo("Raumserver install folder deleted!", CURRENT_POSITION);

            // delete init script            
            if (!sshClient.sftp.removeFile(installDirStartScript + "S99raumserver"))
            {
                progressError("Can't delete raumserver start script!", CURRENT_POSITION);
                hasError = true;
            }
            else
                progressInfo("Raumserver start script deleted!", CURRENT_POSITION);
           
            progressPercentage = 100;
            progressInfo("Closing SSH/SFTP connection", CURRENT_POSITION);
            sshClient.closeSFTP();
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