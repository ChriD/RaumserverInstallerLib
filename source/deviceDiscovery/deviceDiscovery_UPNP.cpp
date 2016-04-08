
#include "deviceDiscovery/deviceDiscovery_UPNP.h"


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
            //OpenHome::Net::UpnpLibrary::Close(); // will fail!
        }


        void DeviceDiscovery_UPNP::init()
        {
            DeviceDiscovery::init();
            
            OpenHome::Net::InitialisationParams*		initParams;
            std::vector<OpenHome::NetworkAdapter*>*		networkAdapterList;
            std::uint8_t                                networkAdapterIdx = -1;
            TIpAddress									networkAdapterAddress;

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


        void DeviceDiscovery_UPNP::setNetworkAdapter(const NetworkAdaperInformation &_networkAdapter)
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
            std::string deviceFriendlyName, location;
            _device.GetAttribute("Upnp.FriendlyName", deviceFriendlyName);
            _device.GetAttribute("Upnp.Location", location);

            logDebug("UPNP Device lost: " + deviceFriendlyName + "(" + _device.Udn() + ")", CURRENT_POSITION);
            // TODO: @@@
        }


        void DeviceDiscovery_UPNP::addInstallableDevice(const std::string &_location, const std::string &_deviceXML)
        {
            pugi::xml_document doc;
            pugi::xml_node deviceNode, rootNode, valueNode;
            std::string deviceType, modelName;

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
                logError("Device XML from device does not contain model information!", CURRENT_POSITION);
                return;
            }
            modelName = valueNode.child_value();

            // TODO: @@@

            //sigDeviceFound.fire()

            /*
            --> find this ones and aybe fill name by other map (UDN: name)

            <deviceType>urn:schemas-raumfeld-com:device:RaumfeldDevice:1</deviceType> 
          <friendlyName>Raumfeld Device</friendlyName> 

         <UDN>uuid:6a1e8f58-beca-4837-b8f0-630941227e34</UDN> 

        <modelName>Raumfeld One S</modelName> 
          <modelNumber>1</modelNumber> 
          <serialNumber>6c:ec:eb:f1:84:b7</serialNumber> 
          <raumfeld:protocolVersion>223</raumfeld:protocolVersion> 
          <raumfeld:hardwareType>13</raumfeld:hardwareType> 
            
            */

        }


        void DeviceDiscovery_UPNP::loadNetworkAdaptersInformation()
        {
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
                    adapterInfo.name = (*networkAdapterList)[i]->Name();
                    adapterInfo.fullName = (*networkAdapterList)[i]->FullName();
                    adapterInfo.address = (*networkAdapterList)[i]->Subnet();
                    networkAdapterInfomationList.push_back(adapterInfo);
                }

                // clean up the subnet list, we do not need it anymore...
                OpenHome::Net::UpnpLibrary::DestroySubnetList(networkAdapterList);

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
            return networkAdapterInfomationList;
        }


    }
}
