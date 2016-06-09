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
#ifndef LOGGERBASE_H
#define LOGGERBASE_H

#include <string>
#include <iostream>
#include <mutex>
#include <chrono>
#include <ctime>
#include <fstream>
#include <exception>

#include <os/os.h>
#include <tools/stringUtil.h>
#include <tools/dateUtil.h>
#include <signals/signals.hpp>
#include <exception/exception.h>


namespace Log
{

    class Log;

    enum class LogType : int8_t { LOGTYPE_CRITICALERROR = 0, LOGTYPE_ERROR = 1, LOGTYPE_WARNING = 2, LOGTYPE_INFO = 3, LOGTYPE_DEBUG = 4 };

    struct LogData
    {
        LogType type;
        std::string log;
        std::string location;
        Tools::DateTimeStamp logDateTimeStamp;
    };


    class LogBase
    {
        public:
            EXPORT LogBase();
            EXPORT virtual ~LogBase();       

            EXPORT void logDebug(const std::string &_log, const std::string &_location);
            EXPORT void logWarning(const std::string &_log, const std::string &_location);
            EXPORT void logInfo(const std::string &_log, const std::string &_location);
            EXPORT void logError(const std::string &_log, const std::string &_location);
            EXPORT void logCritical(const std::string &_log, const std::string &_location);

            EXPORT virtual void setLogObject(std::shared_ptr<Log> _log);
            EXPORT std::shared_ptr<Log> getLogObject();

        protected:
            std::shared_ptr<Log> logObject;
    };


}


#endif