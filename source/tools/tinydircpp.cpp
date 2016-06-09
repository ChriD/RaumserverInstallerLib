
#include "tools/stringUtil.h"
#include "tools/tinydircpp.h"
#ifdef __linux__ 
#elif _WIN32
    #include <direct.h>
#endif


namespace TinyDirCpp
{
    TinyDirCpp::TinyDirCpp()
    {
    }


    TinyDirCpp::~TinyDirCpp()
    {
    }


    void TinyDirCpp::createDirectory(std::string _directory)
    {
        std::string seperator = "/";

        std::vector<std::string> explodedPath = Tools::StringUtil::explodeString(_directory, seperator);
        std::string path = "";
        
        for (auto pathPart : explodedPath)
        {
            if (!pathPart.empty())
            {
                path += path.empty() ? "" : "/";
                path += pathPart;
                #if defined(_WIN32)
                    mkdir(path.c_str());
                #else 
                     mkdir(path.c_str(), 0777); // notice that 777 is different than 0777
                #endif
            }
        }
    }


    std::vector<std::string> TinyDirCpp::getFiles(std::string _folder, std::string _folderRel, std::uint32_t _level, bool _recursive)
    {
        tinydir_dir dir;
        std::vector<std::string> files;

        if (tinydir_open(&dir, _folder.c_str()) == -1)
        {
            // oje.....
        }

        while (dir.has_next)
        {
            tinydir_file file;
            if (tinydir_readfile(&dir, &file) == -1)
            {
                // ujee
            }

            std::string temp = std::string(file.name);
            if (temp != "." && temp != "..")
            {       
                if (file.is_dir)
                {
                    if (_recursive)
                    {                                
                        auto filesSub = getFiles(_folder + file.name + "/", _folderRel + file.name + "/", _level + 1, _recursive);
                        files.insert(files.end(), filesSub.begin(), filesSub.end());
                    }
                }
                else
                {
                    files.emplace_back(_folderRel + std::string(file.name));
                }
            }

            tinydir_next(&dir);
        }


        tinydir_close(&dir);

        return files;
    }

}
