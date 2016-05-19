#include "logger/loggerBase.h"
#include "logger/logger.h"


namespace Log
{

    LogBase::LogBase()
    {
        logObject = nullptr;
    }


    LogBase::~LogBase()
    {
    }

    void LogBase::setLogObject(std::shared_ptr<Log> _logObject)
    {
        logObject = _logObject;
    }


    std::shared_ptr<Log> LogBase::getLogObject()
    {
        return logObject;
    }


    void LogBase::logDebug(const std::string &_log, const std::string &_location)
    {
        if (logObject == nullptr)
            return;
        logObject->debug(_log, _location);
    }


    void LogBase::logWarning(const std::string &_log, const std::string &_location)
    {
        if (logObject == nullptr)
            return;
        logObject->warning(_log, _location);
    }


    void LogBase::logInfo(const std::string &_log, const std::string &_location)
    {
        if (logObject == nullptr)
            return;
        logObject->info(_log, _location);
    }


    void LogBase::logError(const std::string &_log, const std::string &_location)
    {
        if (logObject == nullptr)
            return;
        logObject->error(_log, _location);
    }


    void LogBase::logCritical(const std::string &_log, const std::string &_location)
    {
        if (logObject == nullptr)
            return;
        logObject->critical(_log, _location);
    }
}