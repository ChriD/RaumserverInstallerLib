//
// The MIT License (MIT)
//
// Copyright (c) 2015 by ChriD
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
#ifndef VERSIONINFO_H
#define VERSIONINFO_H


#include <string>
#include "versionNumber.h"
#include "xml/pugixml/pugixml.hpp"
#include "httpClient/httpClient.h"

namespace VersionInfo
{
    struct VersionInfo
    {
        public:
            std::string appName = AppName;
            std::string appVersion = AppVersionNumber;
            std::string appVersionName = AppVersionName;
            std::uint32_t appVersionBuild = AppVersionBuild;

            EXPORT void loadFromVersionInfo(std::string _versionInfoXMLString);
            EXPORT bool loadFromXMLFile(std::string _xmlFile);
            EXPORT void loadFromUrl(std::string _url);
            EXPORT void clear();

        protected:
            void loadFromXMLDoc(pugi::xml_document &_doc);

    };           
}


#endif
