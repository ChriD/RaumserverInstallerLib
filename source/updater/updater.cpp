#include <updater/updater.h>
#include <zip/zip_file.hpp>

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
        downloadDestination = "";
        getBinaryError = false;
        logProgress = true;
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
        httpClient.setLogObject(this->getLogObject());
        httpClient.init();
    }


    void Updater::setSource(const std::string &_source)
    {
        source = _source;
    }


    void Updater::setLogProgress(bool _logProgress)
    {
        logProgress = _logProgress;
    }


    void Updater::run(bool _sync, bool _doUpdate)
    {
        httpClient.setLogObject(this->getLogObject());
        sync = _sync;
        doImmediateUpdate = _doUpdate;
        getBinary(_sync);
    }


    bool Updater::needsBinaryUpdate()
    {             
        return true;
    }


    void Updater::getBinary(bool _sync)
    {
        getBinaryThreadObject = std::thread(&Updater::getBinaryThread, this);
        if (_sync)
            getBinaryThreadObject.join();
    }


    void Updater::getBinaryThread()
    {
        if (!needsBinaryUpdate())        
            binaryReady();                    
        else if (doGetBinary())
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
        
        return !getBinaryError;
    }


    void Updater::doGetBinary_HTTP_RequestProcess(HttpClient::HttpRequest *_request)
    {
        auto downloadedSize = _request->getResponse()->getData().size();
        auto size = _request->getResponse()->getContentSize();        
        progressInfo("Progress: " + std::to_string(downloadedSize) + "/" + std::to_string(size), CURRENT_FUNCTION, "DOWNLOADPROG");
    }


    void Updater::doGetBinary_HTTP_RequestResult(HttpClient::HttpRequest *_request)
    {
        if (_request->getResponse()->getErrorCode() == 0)
        {
            progressInfo("Creating file " + downloadDestination, CURRENT_POSITION);
            std::ofstream downloadedFile(downloadDestination, std::ios::out | std::ios::binary);
            if (downloadedFile.good())
            {
                auto responseData = _request->getResponse()->getData();
                std::size_t size = responseData.size();
                //downloadedFile.write(reinterpret_cast<const char*>(&size), sizeof(size));
                downloadedFile.write(reinterpret_cast<const char*>(&responseData[0]), size * sizeof(responseData[0]));
                downloadedFile.close();
                progressInfo("Download finished", CURRENT_POSITION);
                getBinaryError = false;
            }
            else
            {
                progressError("Error when creating file " + downloadDestination, CURRENT_POSITION);
                getBinaryError = true;
            }
        }
        else
        {
            progressError("Error " + std::to_string(_request->getResponse()->getErrorCode())  + " when downloading file " + source, CURRENT_POSITION);
            getBinaryError = true;
        }

        // Set file downloaded so the updater knows he can go on...
        downloadRequestDone = true;
    }


    bool Updater::extractFile(std::string _file, std::string destination)       
    {        
        if (!destination.empty() && (destination.back() == '/' || destination.back() == '\\'))
            destination.pop_back();

        progressInfo("Reading ZIP file. Please be patient!", CURRENT_POSITION);
        try
        {
            zip_file f(_file);
            progressInfo("Extracting ZIP file. This may take a while!", CURRENT_POSITION);
            f.extractall(destination);
            progressInfo("Extracting done!", CURRENT_POSITION);
        }
        catch (std::exception _ex)
        {
            progressError(_ex.what(), CURRENT_POSITION);
            return false;
        }
        catch ( ... )
        {
            progressError("Unknown error handling ZIP file!", CURRENT_POSITION);
            return false;
        }

        return true;
    }


    void Updater::progressDebug(const std::string &_progressInfo, const std::string &_location, const std::string &_id)
    {
        if (logProgress)
            logDebug(_progressInfo, _location);
        //sigUpdateProgress.fire(ProgressInfo(_progressInfo, _id, false));
    }


    void Updater::progressWarning(const std::string &_progressInfo, const std::string &_location, const std::string &_id)
    {
        if (logProgress)
            logWarning(_progressInfo, _location);
        sigUpdateProgress.fire(ProgressInfo(_progressInfo, _id, false));
    }


    void Updater::progressInfo(const std::string &_progressInfo, const std::string &_location, const std::string &_id)
    {
        if (logProgress)
            logInfo(_progressInfo, _location);
        sigUpdateProgress.fire(ProgressInfo(_progressInfo, _id, false));
    }


    void Updater::progressError(const std::string &_progressInfo, const std::string &_location, const std::string &_id)
    {
        if (logProgress)
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
