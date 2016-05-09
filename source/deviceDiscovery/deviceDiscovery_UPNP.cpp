
#include "deviceDiscovery/deviceDiscovery_UPNP.h"
#include "tools/urlParser.h"


namespace RaumserverInstaller
{
    namespace DeviceDiscovery
    {

        DeviceDiscovery_UPNP::DeviceDiscovery_UPNP() : DeviceDiscovery()
        {        
            upnpStarted = false;
            upupDeviceListAll = nullptr;
        }


        DeviceDiscovery_UPNP::~DeviceDiscovery_UPNP()
        {
            logDebug("Closing OpenHome UPNP Control Stack", CURRENT_POSITION);  
            delete upupDeviceListAll;
            delete initParams;
            //OpenHome::Net::UpnpLibrary::Close(); // TODO: will fail! No idea why. Have to investigate
        }


        void DeviceDiscovery_UPNP::init()
        {
            DeviceDiscovery::init();
            
            try
            {                          
                logDebug("Init OpenHome UPNP Control Stack", CURRENT_POSITION);

                initParams = OpenHome::Net::InitialisationParams::Create();
                OpenHome::Net::UpnpLibrary::Initialise(initParams);    

                // after initialisation get the available network adapters
                loadNetworkAdaptersInformation();
            }
            catch (Raumkernel::Exception::RaumkernelException &e)
            {
                if (e.type() == Raumkernel::Exception::ExceptionType::EXCEPTIONTYPE_APPCRASH)
                    throw e;
            }
            catch (std::exception &e)
            {
                throw e;
            }
            catch (std::string &e)
            {
                throw e;
            }
            catch (OpenHome::Exception &e)
            {
                throw e;
            }
            catch (...)
            {
                throw std::runtime_error("Unknown exception! [DeviceFinder_Raumfeld::init]");
            }
        }


        void DeviceDiscovery_UPNP::setNetworkAdapter(NetworkAdaperInformation _networkAdapter)
        {
            selectedNetworkAdapter = _networkAdapter;
        }


        void DeviceDiscovery_UPNP::startDiscover()
        {
            initAdapter();
            discover();
        }


        void DeviceDiscovery_UPNP::initAdapter()
        {
            if (!selectedNetworkAdapter.id)
                return;

            logInfo("Starting OpenHome UPNP Control Stack with network adapter: " + selectedNetworkAdapter.name, CURRENT_POSITION);
            OpenHome::Net::UpnpLibrary::StartCp(selectedNetworkAdapter.address);            
            
            upnpStarted = true;

        }


        void DeviceDiscovery_UPNP::discover()
        {
            if (!upnpStarted)
                return;

            if (upupDeviceListAll != nullptr)
                delete upupDeviceListAll;

            OpenHome::Net::FunctorCpDeviceCpp functorDeviceFound = OpenHome::Net::MakeFunctorCpDeviceCpp(*this, &DeviceDiscovery_UPNP::onDeviceFound);
            OpenHome::Net::FunctorCpDeviceCpp functorDeviceLost = OpenHome::Net::MakeFunctorCpDeviceCpp(*this, &DeviceDiscovery_UPNP::onDeviceLost);
            upupDeviceListAll = new OpenHome::Net::CpDeviceListCppUpnpAll(functorDeviceFound, functorDeviceLost);
        }

        
        void DeviceDiscovery_UPNP::onDeviceFound(OpenHome::Net::CpDeviceCpp& _device)
        {
            std::string deviceFriendlyName, location, deviceXml;

            _device.GetAttribute("Upnp.FriendlyName", deviceFriendlyName);
            _device.GetAttribute("Upnp.Location", location);
            _device.GetAttribute("Upnp.DeviceXml", deviceXml);

            logDebug("UPNP Device found: " + deviceFriendlyName + "(" + _device.Udn() + ")", CURRENT_POSITION);
                   
            addInstallableDevice(location, deviceXml);
        }

        
        void DeviceDiscovery_UPNP::onDeviceLost(OpenHome::Net::CpDeviceCpp& _device)
        {
            std::string deviceFriendlyName, location, deviceXml;
            
            _device.GetAttribute("Upnp.FriendlyName", deviceFriendlyName);
            _device.GetAttribute("Upnp.Location", location);
            _device.GetAttribute("Upnp.DeviceXml", deviceXml);

            logDebug("UPNP Device lost: " + deviceFriendlyName + "(" + _device.Udn() + ")", CURRENT_POSITION);

            removeInstallableDevice(location, deviceXml);            
        }


        void DeviceDiscovery_UPNP::removeInstallableDevice(const std::string &_location, const std::string &_deviceXML)
        {
            addRemoveInstallableDevice(_location, _deviceXML, false);
        }


        void DeviceDiscovery_UPNP::addInstallableDevice(const std::string &_location, const std::string &_deviceXML)
        {
            addRemoveInstallableDevice(_location, _deviceXML, true);
        }


        void DeviceDiscovery_UPNP::addRemoveInstallableDevice(const std::string &_location, const std::string &_deviceXML, bool _add)
        {
            pugi::xml_document doc;
            pugi::xml_node deviceNode, rootNode, valueNode;
            std::string deviceType, modelName, friendlyName, udn;

            pugi::xml_parse_result result = doc.load_string(_deviceXML.c_str());

            // find the root node which has to be the 'device' node	
            rootNode = doc.child("root");
            if (!rootNode)
            {
                logError("Device XML from device does not contain root block!", CURRENT_POSITION);
                return;
            }

            // find the root node which has to be the 'device' node	
            deviceNode = rootNode.child("device");
            if (!deviceNode)
            {
                logError("Device XML from device does not contain device information!", CURRENT_POSITION);
                return;
            }            

            valueNode = deviceNode.child("deviceType");
            if (!valueNode)
            {
                logError("Device XML from device does not contain type information!", CURRENT_POSITION);
                return;
            }
            deviceType = valueNode.child_value();

            valueNode = deviceNode.child("modelName");
            if (!valueNode)
            {
                logError("Device XML from device does not contain modelName information!", CURRENT_POSITION);
                return;
            }
            modelName = valueNode.child_value();


            valueNode = deviceNode.child("friendlyName");
            if (!valueNode)
            {
                logError("Device XML from device does not contain friendlyName information!", CURRENT_POSITION);
                return;
            }
            friendlyName = valueNode.child_value();


            valueNode = deviceNode.child("UDN");
            if (!valueNode)
            {
                logError("Device XML from device does not contain friendlyName information!", CURRENT_POSITION);
                return;
            }
            udn = valueNode.child_value();


            if (deviceType      == "urn:schemas-raumfeld-com:device:RaumfeldDevice:1" &&
                friendlyName    == "Raumfeld Device")
            {
                // parse the location uri to get the ip
                LUrlParser::clParseURL url = LUrlParser::clParseURL::ParseURL(_location);

                // device may be suitable, so we create a device information and add it to the internal map (ip | info)
                DeviceInformation   deviceInformation;                
                deviceInformation.ip = url.m_Host;
                deviceInformation.name = modelName;               
                deviceInformation.UDN = udn;
                deviceInformation.type = DeviceType::DT_UPNPDEVICE_RAUMFELD;                           
                
                if (_add)
                {
                    logInfo("Device (" + deviceInformation.ip + ") '" + deviceInformation.name + "' found for installation!", CURRENT_POSITION);
                    sigDeviceFound.fire(deviceInformation);
                }
                else
                {
                    logInfo("Device (" + deviceInformation.ip + ") '" + deviceInformation.name + "' removed for installation!", CURRENT_POSITION);
                    sigDeviceRemoved.fire(deviceInformation);
                }

            }
            else
            {
                logDebug("Device '" + friendlyName + "' not suitable for installation!", CURRENT_POSITION);
                return;
            } 
        }


        void DeviceDiscovery_UPNP::loadNetworkAdaptersInformation()
        {
            std::unique_lock<std::mutex> lock(adapterListMutex);

            std::vector<OpenHome::NetworkAdapter*>		*networkAdapterList;                   

            try
            {

                networkAdapterInfomationList.clear();

                logDebug("Retrieve network adapter list", CURRENT_POSITION);
                networkAdapterList = OpenHome::Net::UpnpLibrary::CreateSubnetList();

                if ((*networkAdapterList).size() == 0)
                {
                    logCritical("No network adapter available!", CURRENT_POSITION);
                    throw Raumkernel::Exception::RaumkernelException(Raumkernel::Exception::ExceptionType::EXCEPTIONTYPE_APPCRASH, CURRENT_POSITION, "Unrecoverable error! ABORT!", 200);
                }

                // iterate through the list and try to find the adapter with the given name
                for (std::uint8_t i = 0; i < (*networkAdapterList).size(); i++)
                {
                    std::string adapterName = (*networkAdapterList)[i]->Name();
                    logDebug("Adapter " + std::to_string(i) + ": " + adapterName, CURRENT_POSITION);

                    NetworkAdaperInformation adapterInfo;
                    adapterInfo.id = i+1;
                    adapterInfo.name = std::string((*networkAdapterList)[i]->Name());
                    adapterInfo.fullName = std::string((*networkAdapterList)[i]->FullName());
                    adapterInfo.address = (*networkAdapterList)[i]->Subnet();
                    networkAdapterInfomationList.push_back(adapterInfo);

                    // we have to delete the char* from the OhNetstack after we have copied it to the std::string
                    // look at the description of 'FullName'
                    //delete (*networkAdapterList)[i]->FullName();
                }

                // clean up the subnet list, we do not need it anymore...
                //OpenHome::Net::UpnpLibrary::DestroySubnetList(networkAdapterList);

            }
            catch (Raumkernel::Exception::RaumkernelException &e)
            {
                if (e.type() == Raumkernel::Exception::ExceptionType::EXCEPTIONTYPE_APPCRASH)
                    throw e;
            }
            catch (std::exception &e)
            {
                throw e;
            }
            catch (std::string &e)
            {
                throw e;
            }
            catch (OpenHome::Exception &e)
            {
                throw e;
            }
            catch (...)
            {
                throw std::runtime_error("Unknown exception! [DeviceFinder_Raumfeld::getNetworkAdaptersInformation]");
            }            
        }


        std::vector<NetworkAdaperInformation> DeviceDiscovery_UPNP::getNetworkAdaptersInformation()
        {
            std::unique_lock<std::mutex> lock(adapterListMutex);            
            return networkAdapterInfomationList;
        }


    }
}
