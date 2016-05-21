#include <updater/updater.h>

namespace Updater
{

    Updater::Updater() : Log::LogBase()
    {
        sync = true;
        doImmediateUpdate = false;
        stopAllThreads = false;
        downloadRequestDone = false; 
        source = "";
        destination = "";
        tmpDestination = "";
    }


    Updater::~Updater()
    {
        stopThreads();
    }


    void Updater::stopThreads()
    {
        stopAllThreads = true;
        if (getBinaryThreadObject.joinable())                    
            getBinaryThreadObject.join();       
        stopAllThreads = false;

        httpClient.abortAllRequests();
    }


    void Updater::init()
    {
        httpClient.init();
    }


    void Updater::run(bool _sync, bool _doUpdate)
    {
        sync = _sync;
        doImmediateUpdate = _doUpdate;
        getBinary(_sync);
    }


    void Updater::getBinary(bool _sync)
    {
        getBinaryThreadObject = std::thread(&Updater::getBinaryThread, this);
        if (_sync)
            getBinaryThreadObject.join();
    }


    void Updater::getBinaryThread()
    {
        if (doGetBinary())
            binaryReady();
    }


    bool Updater::doGetBinary()
    {
        // has to be overwritten
        return false;
    }


    void Updater::binaryReady()
    {
        sigBinaryReady.fire();
        if (doImmediateUpdate)
            update(sync);        
    }


    void Updater::update(bool _sync)
    {
        sync = _sync;
        updateThreadObject = std::thread(&Updater::updateThread, this);
        if (_sync)
            updateThreadObject.join();
    }


    void Updater::updateThread()
    {
        if (doUpdate())
            updateDone();
    }


    bool Updater::doUpdate()
    {
        // has to be overwritten
        return false;
    }


    void Updater::updateDone()
    {
        sigBinaryUpdated.fire();
    }


    bool Updater::doGetBinary_HTTP()
    { 
        progressInfo("Downloading " + source, CURRENT_POSITION);
        downloadRequestDone = false;
        httpClient.request(source, nullptr, nullptr, nullptr, std::bind(&Updater::doGetBinary_HTTP_RequestResult, this, std::placeholders::_1), std::bind(&Updater::doGetBinary_HTTP_RequestProcess, this, std::placeholders::_1));
        // wait until download is complete!
        while (!downloadRequestDone)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }

        return false;
    }


    void Updater::doGetBinary_HTTP_RequestProcess(HttpClient::HttpRequest *_request)
    {
        auto downloadedSize = _request->getResponse()->getData().size();
        auto size = _request->getResponse()->getContentSize();        
        progressInfo("Prgress: " + std::to_string(downloadedSize) + "/" + std::to_string(size), CURRENT_FUNCTION, "DOWNLOADPROG");
    }


    void Updater::doGetBinary_HTTP_RequestResult(HttpClient::HttpRequest *_request)
    {
        if (_request->getResponse()->getErrorCode() == 0)
        {
            progressInfo("Creating file " + destination, CURRENT_POSITION);
            std::ofstream downloadedFile(destination, std::ios::out | std::ios::binary);
            if (downloadedFile.good())
            {
                auto responseData = _request->getResponse()->getData();
                std::size_t size = responseData.size();
                downloadedFile.write(reinterpret_cast<const char*>(&size), sizeof(size));
                downloadedFile.write(reinterpret_cast<const char*>(&responseData[0]), size * sizeof(responseData[0]));
                downloadedFile.close();
                progressInfo("Download finished", CURRENT_POSITION);
            }
            else
            {
                progressError("Error when creating file " + destination, CURRENT_POSITION);
            }
        }
        else
        {
            progressError("Error " + std::to_string(_request->getResponse()->getErrorCode())  + " when downloading file " + source, CURRENT_POSITION);
        }

        // Set file downloaded so the updater knows he can go on...
        downloadRequestDone = true;
    }


    bool Updater::extractFile(std::string _file, std::string destination)       
    {
        // TODO: @@@
        return false;
    }


    void Updater::progressDebug(const std::string &_progressInfo, const std::string &_location, const std::string &_id)
    {
        logDebug(_progressInfo, _location);
        //sigUpdateProgress.fire(ProgressInfo(_progressInfo, _id, false));
    }


    void Updater::progressWarning(const std::string &_progressInfo, const std::string &_location, const std::string &_id)
    {
        logWarning(_progressInfo, _location);
        sigUpdateProgress.fire(ProgressInfo(_progressInfo, _id, false));
    }


    void Updater::progressInfo(const std::string &_progressInfo, const std::string &_location, const std::string &_id)
    {
        logInfo(_progressInfo, _location);
        sigUpdateProgress.fire(ProgressInfo(_progressInfo, _id, false));
    }


    void Updater::progressError(const std::string &_progressInfo, const std::string &_location, const std::string &_id)
    {
        logError(_progressInfo, _location);
        sigUpdateProgress.fire(ProgressInfo(_progressInfo, _id, true));
    }




    ProgressInfo::ProgressInfo(const std::string &_info, const std::string &_id, const bool &_error)
    {
        id = _id;
        info = _info;
        error = _error;
    }

}
