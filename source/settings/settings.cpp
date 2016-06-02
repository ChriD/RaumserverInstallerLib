
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


    void SettingsManager::loadSettings()
    {
        logDebug("Load settings", CURRENT_POSITION);
        loadSettingsFromFile(settingsFileName);        
    }


    std::string SettingsManager::getValue(const std::string &_settingsPath, const std::string &_defaultValue, const std::map<std::string, std::string> &_attributeFilter, const std::uint16_t &_index)
    {
        // lock the map while we do read some settings
        std::lock_guard<std::mutex> lock(mutexSettingsMapAccess);


        // TODO: @@@ attribute filter

        if (settingsMap.find(_settingsPath) != settingsMap.end())
            return settingsMap.at(_settingsPath);
        return _defaultValue;
    }


    void SettingsManager::walkNode(pugi::xml_node _node, const std::string &_path, const int &_indent)
    {
        const auto ind = std::string(_indent * 4, ' ');
        std::string path = "";

        // we do not want to have the "Application" node be stored in the path!
        if (_indent >= 1)
            path = _path;

        for (auto childNode : _node.children())
        {
            if (childNode.value() && !std::string(childNode.value()).empty())
            {
                logDebug("Found setting: " + path + " = " + std::string(childNode.value()), CURRENT_POSITION);
                settingsMap.insert(std::make_pair(path, childNode.value()));
            }
            walkNode(childNode, path + "/" + childNode.name(), _indent + 1);
        }
    }


    void SettingsManager::loadSettingsFromFile(const std::string &_fileName)
    {
        logDebug("Loading settings from file '" + _fileName + "'", CURRENT_POSITION);

        // lock the map while we do update it
        std::lock_guard<std::mutex> lock(mutexSettingsMapAccess);

        // start with a nice clean map.
        settingsMap.clear();

        try
        {
            pugi::xml_document doc;
            pugi::xml_node applicationNode;

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

            logDebug("Parsing settings file...", CURRENT_POSITION);

            // go through all nodes and load those values and path into the settings map
            walkNode(applicationNode);

            logDebug("Parsing settings file is done!", CURRENT_POSITION);
            logDebug("Settings loaded!", CURRENT_POSITION);

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
            throw std::runtime_error("Unknown exception! [SettingsManager::loadSettingsFromFile]");
        }

    }

}