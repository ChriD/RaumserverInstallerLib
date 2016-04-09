
#include "tools/tinydircpp.h"


namespace TinyDirCpp
{
    TinyDirCpp::TinyDirCpp()
    {
    }


    TinyDirCpp::~TinyDirCpp()
    {
    }


    std::vector<std::string> TinyDirCpp::getFiles(std::string _folder, bool _recursive)
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
                printf("%s", file.name);
                if (file.is_dir)
                {
                    auto filesSub = getFiles(_folder + file.name + "/");
                    files.insert(files.end(), filesSub.begin(), filesSub.end());
                }
                else
                {
                    files.emplace_back(_folder + std::string(file.name));
                }
            }

            tinydir_next(&dir);
        }


        tinydir_close(&dir);

        return files;
    }

}