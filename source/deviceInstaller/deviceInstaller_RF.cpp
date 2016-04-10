
#include "deviceInstaller/deviceInstaller_RF.h"


namespace RaumserverInstaller
{
    namespace DeviceInstaller
    {

        DeviceInstaller_RaumfeldDevice::DeviceInstaller_RaumfeldDevice() : DeviceInstaller()
        {
            abortInstallThread = false;
        }


        DeviceInstaller_RaumfeldDevice::~DeviceInstaller_RaumfeldDevice()
        {
            abortInstall();
        }


        void DeviceInstaller_RaumfeldDevice::startInstall()
        {
            DeviceInstaller::startInstall();
            installThreadObject = std::thread(&DeviceInstaller_RaumfeldDevice::installThread, this);
        }


        void DeviceInstaller_RaumfeldDevice::abortInstall()
        {
            DeviceInstaller::abortInstall();

            abortInstallThread = true;
            if (installThreadObject.joinable())
            {
                progressInfo("Waiting for install thread to abort!", CURRENT_POSITION);
                installThreadObject.join();
            }
            abortInstallThread = false;
        }


        void DeviceInstaller_RaumfeldDevice::installThread()
        {
            // the device we do install the raumserver on with this installer object has to be a raumfeld device!
            if (deviceInformation.type != DeviceType::DT_UPNPDEVICE_RAUMFELD)
            {
                progressError("Device '" + deviceInformation.name + "' not compatible with installer!", CURRENT_POSITION);
                return;
            }

            /*
            if (deviceInformation.sshAccess != SSHAccess::SSH_NO)
            {
                progressError("Device '" + deviceInformation.name + "' has no SSH/SFTP-Access!", CURRENT_POSITION);
                return;
            }
            */

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
                sigInstallDone.fire(DeviceInstallerProgressInfo("Could not connect to Device! (SSH)", progressPercentage, true));
                return;
            }
                         
            if (!sshClient.connectSFTP())
            {
                progressError("Could not connect to Device! (SFTP)", CURRENT_POSITION);
                sigInstallDone.fire(DeviceInstallerProgressInfo("Could not connect to Device! (SSH)", progressPercentage, true));
                return;
            }

            progressPercentage = 10;
            progressInfo("Connected to device (SSH/SFTP)", CURRENT_POSITION);

            // TODO: @@@

            progressInfo("Copying files to remote device...", CURRENT_POSITION);

            // copy the files. Due the installation is in a thread we can do a 'sync' copy
            sshClient.sftp.copyDir("binaries/raumserverDaemon/", "raumserverDaemon/", true);

            // Wait till copy is finished in a loop se we may abort?!?!
            //abortInstallThread

            // TODO: the sshclient does have signals we have to connect for proress info

            // TODO: copy init script

            // TODO: maybe reboot and wait for connection again

            // TODO: start raumserver (if no reboot)

            // TODO: Then check if port of Raumserver is open

            progressPercentage = 100;
            progressInfo("Closing SSH/SFTP connection", CURRENT_POSITION);
            sshClient.closeSFTP();
            sshClient.closeSSH();

            progressInfo("Installation done!", CURRENT_POSITION);
            sigInstallDone.fire(DeviceInstallerProgressInfo("Installation done!", progressPercentage, false));
        }


    }
}