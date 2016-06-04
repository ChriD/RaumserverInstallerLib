
#include <settings/settings.h>

namespace Settings
{
  
    SettingsManager::SettingsManager() : Log::LogBase()
    {
        settingsFileName = "settings.xml";
    }


    SettingsManager::~SettingsManager()
    {
        logDebug("Destroying Settings-Manager", CURRENT_POSITION);
    }


    void SettingsManager::setFileName(const std::string &_settingsFileName)
    {
        settingsFileName = _settingsFileName;
    }


    void SettingsManager::initSettings()
    {
        logDebug("Init settings file", CURRENT_POSITION);
        initSettingsFile(settingsFileName);        
    }


    std::string SettingsManager::getValue(const std::string &_xPath, const std::string &_defaultValue)
    {
        // lock the map while we do read some settings
        std::lock_guard<std::mutex> lock(mutexSettingsAccess);        

        auto settingsXPathNode = applicationNode.select_single_node(_xPath.c_str());
        if (settingsXPathNode)
            return  settingsXPathNode.node().child_value();
        return _defaultValue;
    }

    void SettingsManager::initSettingsFile(const std::string &_fileName)
    {
        logDebug("Loading settings from file '" + _fileName + "'", CURRENT_POSITION);

        // lock the map while we do update it
        std::lock_guard<std::mutex> lock(mutexSettingsAccess);

        try
        {            
            // we do have to have a settings file. if no file name is given or we are not able to open the file we have to crash the kernel!
            if (_fileName.empty())
            {
                logCritical("Not settings file specified! System will not be able to start!", CURRENT_POSITION);                    
                throw std::runtime_error("Unrecoverable error! ABORT!");
            }

            std::ifstream settingsFileStream(_fileName.c_str());
            if (settingsFileStream.fail())
            {
                logCritical("Can not open settings file '" + _fileName + "'! System will not be able to start!", CURRENT_POSITION);
                throw std::runtime_error("Unrecoverable error! ABORT!");
            }

            pugi::xml_parse_result result = doc.load_file(_fileName.c_str());

            // find the root node which has to be the 'Application' node	
            applicationNode = doc.child("Application");
            if (!applicationNode)
            {
                logCritical("'Application' node not found in settings file! System will not be able to start!", CURRENT_POSITION);
                throw std::runtime_error("Unrecoverable error! ABORT!");
            }

            logDebug("Settings file initialized...", CURRENT_POSITION);          
        }
        catch (std::exception &e)
        {
            throw e;
        }
        catch (std::string &e)
        {
            throw e;
        }
        catch (...)
        {
            throw std::runtime_error("Unknown exception! [SettingsManager::initSettingsFile]");
        }

    }

}