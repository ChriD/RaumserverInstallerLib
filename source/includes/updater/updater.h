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
#ifndef UPDATER_H
#define UPDATER_H

#include "logger/logger.h"
#include "signals/signals.hpp"
#include "httpClient/httpClient.h"

namespace Updater
{

    struct ProgressInfo
    {
        ProgressInfo(const std::string &_info, const std::string &_id, const bool &_error);

        std::string id = "";
        std::string info = "";    
        bool error = false;
    };
    

    class Updater : public Log::LogBase
    {
        public:
            Updater();
            ~Updater();

            EXPORT virtual void init();
            EXPORT virtual void run(bool _sync = false, bool _doUpdate = true);
            EXPORT virtual void getBinary(bool _sync = false);
            EXPORT virtual void update(bool _sync = false);   
            
            EXPORT virtual void progressDebug(const std::string &_progressInfo, const std::string &_location, const std::string &_id = "");
            EXPORT virtual void progressWarning(const std::string &_progressInfo, const std::string &_location, const std::string &_id = "");
            EXPORT virtual void progressInfo(const std::string &_progressInfo, const std::string &_location, const std::string &_id = "");
            EXPORT virtual void progressError(const std::string &_progressInfo, const std::string &_location, const std::string &_id = "");

            /**
            * this signal will be fired when the binary is ready for install
            */
            sigs::signal<void(ProgressInfo)> sigUpdateProgress;
            /**
            * this signal will be fired when the binary is ready for install
            */
            sigs::signal<void()> sigBinaryReady;
            /**
            * this signal will be fired when the binary is installed
            */
            sigs::signal<void()> sigBinaryUpdated;

        protected: 
            bool sync;
            bool doImmediateUpdate;
            std::thread getBinaryThreadObject;
            std::thread updateThreadObject;
            std::atomic_bool stopAllThreads;

            HttpClient::HttpClient httpClient;
            bool downloadRequestDone;

            std::string source;
            std::string tmpDestination;
            std::string destination;

            virtual void stopThreads();
            virtual void getBinaryThread(); 
            virtual void updateThread();
            virtual bool doGetBinary(); 
            virtual bool doUpdate();
            virtual void binaryReady();
            virtual void updateDone();
            
            virtual bool doGetBinary_HTTP();
            virtual void doGetBinary_HTTP_RequestResult(HttpClient::HttpRequest *_request);
            virtual void doGetBinary_HTTP_RequestProcess(HttpClient::HttpRequest *_request);

            virtual bool extractFile(std::string _file, std::string destination);
            

    };
    
}


#endif