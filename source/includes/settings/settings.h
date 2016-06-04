//
// The MIT License (MIT)
//
// Copyright (c) 2016 by ChriD
//
// Permission is hereby granted, free of charge,  to any person obtaining a copy of
// this software and  associated documentation  files  (the "Software"), to deal in
// the  Software  without  restriction,  including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
// the Software,  and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:
//
// The above copyright notice and this  permission notice  shall be included in all
// copies or substantial portions of the Software.
//
// THE  SOFTWARE  IS  PROVIDED  "AS IS",  WITHOUT  WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE  AND NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE  LIABLE FOR ANY CLAIM,  DAMAGES OR OTHER LIABILITY, WHETHER
// IN  AN  ACTION  OF  CONTRACT,  TORT  OR  OTHERWISE,  ARISING  FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

#pragma once
#ifndef SETTINGS_H
#define SETTINGS_H

#include <map>
#include <logger/logger.h>
#include <xml/pugixml/pugixml.hpp>


namespace Settings
{      
    class SettingsManager : public Log::LogBase
    {
        public:
            EXPORT SettingsManager();
            EXPORT virtual ~SettingsManager();
            EXPORT void initSettings();
            EXPORT void setFileName(const std::string &_settingsFileName);
            EXPORT std::string getValue(const std::string &_xPath, const std::string &_defaultValue = "");

        protected:
            void initSettingsFile(const std::string &_fileName);               

            // persistent application node of file
            pugi::xml_node applicationNode;
            pugi::xml_document doc;            

            // a mutex that will secure our settings map 
            std::mutex mutexSettingsAccess;

            // filename and path for settings file
            std::string settingsFileName;
    };
    
}


#endif