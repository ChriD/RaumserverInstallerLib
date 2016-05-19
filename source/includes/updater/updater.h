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
#include "ftplib/ftplib.h"

namespace Updater
{
    
    class Updater
    {
        public:
            Updater();
            ~Updater();

            virtual void init();

            // run(sync)

            // getBinary(sync) // <-- thread
            // getBinary_FTP(url, user, pass, source, destination)

            // overwrite logInfo and logError aso.... and sig progress info
            
            // update() // <-- thread ? only id binary is ready
            /*              
            */

            /**
            * this signal will be fired when progress ....
            */
            //sigs::signal<void(TODO:@@@)> sigUpdateProgress;
            //sigs::signal<void(TODO:@@@)> sigBinaryReady; // <-- will be called if binary is ready

            // TODO: Inherit class --> Updater_RaumserverDaemon
            // TODO: Inherit class --> Updater_RaumserverInstaller

        protected: 
            

    };
    
}


#endif