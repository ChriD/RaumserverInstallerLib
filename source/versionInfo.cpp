#include "versionInfo.h"

namespace VersionInfo
{

 
    void VersionInfo::loadFromVersionInfo(std::string _versionInfoXMLString)
    {
        pugi::xml_document doc;
        pugi::xml_parse_result result = doc.load_string(_versionInfoXMLString.c_str());
        loadFromXMLDoc(doc);
    }


    bool VersionInfo::loadFromXMLFile(std::string _xmlFile)
    {
        pugi::xml_document doc;            
        pugi::xml_parse_result result = doc.load_file(_xmlFile.c_str());
        if (result.status != 1) // 1 = file not found
        {
            loadFromXMLDoc(doc);
            return true;
        }
        else
        {
            appVersion = "";
            appVersionName = "";
            appVersionBuild = 0;
            return false;
        }
    }


    void VersionInfo::loadFromUrl(std::string _url)
    {
        HttpClient::HttpClient  httpClient;
       
        auto request = httpClient.request(_url, nullptr, nullptr, nullptr, nullptr, nullptr);
        // the request itself will be handled by the httpClient and will be deleted in there but due we use a shared pointer
        // the request will at least stay as long as the run of this method
        request->waitTillFinished();
        if (request->getResponse()->getErrorCode() == 0)
        {
            auto data = request->getResponse()->getData();
            std::string dataStr(data.begin(), data.end());
            loadFromVersionInfo(dataStr);
        }
        else
        {
            appVersion = "";
            appVersionName = "";
            appVersionBuild = 0;
        }
    }


    void VersionInfo::loadFromXMLDoc(pugi::xml_document &_doc)
    {
        pugi::xml_node rootNode, appNameNode, appVersionNumberNode, appVersionBuildNode, appVersionNameNode;

        rootNode = _doc.child("build");
        if (!rootNode)               
            throw std::runtime_error("Wrong formated version file! Missing 'build' node!");            

        appNameNode = rootNode.child("appName");
        if (!appNameNode)
            throw std::runtime_error("Wrong formated version file! Missing 'appName' node!");
        appName = appNameNode.child_value();

        appVersionNumberNode = rootNode.child("appVersionNumber");
        if (!appVersionNumberNode)
            throw std::runtime_error("Wrong formated version file! Missing 'appVersionNumber' node!");
        appVersion = appVersionNumberNode.child_value();

        appVersionNameNode = rootNode.child("appVersionName");
        if (!appVersionNameNode)
            throw std::runtime_error("Wrong formated version file! Missing 'appVersionName' node!");
        appVersionName = appVersionNameNode.child_value();

        appVersionBuildNode = rootNode.child("appVersionBuild");
        if (!appVersionBuildNode)
            throw std::runtime_error("Wrong formated version file! Missing 'appVersionBuild' node!");
        appVersionBuild = std::stoi(appVersionBuildNode.child_value());
         
    }

   
}
