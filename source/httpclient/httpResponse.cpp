
#include <httpclient/httpResponse.h>
#include <httpclient/httpClient.h>


namespace HttpClient
{

    HttpResponse::HttpResponse() : Log::LogBase()
    {
        errorCode = 0;
        statusCode = 0;
        statusText = "";
        protocol = "";        
    }


    HttpResponse::~HttpResponse()
    {
    }


    void HttpResponse::setErrorCode(std::uint16_t _errorCode)
    {
        errorCode = _errorCode;
    }


    std::uint16_t HttpResponse::getErrorCode()
    {
        return errorCode;
    }


    std::vector<unsigned char> HttpResponse::getData()
    {
        return data;
    }


    void HttpResponse::setData(std::vector<unsigned char> _data, bool _append)
    {
        if (_append)
            data.insert(data.end(), _data.begin(), _data.end());
        else
            data = _data;
    }


    void HttpResponse::setHeaders(std::map<std::string, std::string> _headers)
    {
        headerVars = _headers;
    }


    std::uint16_t HttpResponse::getStatusCode()
    {
        return statusCode;
    }


    std::string HttpResponse::getProtocolInfo()
    {
        return protocol;
    }


    std::string HttpResponse::getStatusText()
    {
        return statusText;
    }


    std::uint64_t HttpResponse::getContentSize()
    {
        auto size = getHeaderVar("content-length");
        if (size.empty())
            return 0;
        else
            return std::stoi(size);
    }


    std::string HttpResponse::getHeaderVar(std::string _varId)
    {
        std::string varId = _varId;
        std::transform(varId.begin(), varId.end(), varId.begin(), ::tolower);

        if (headerVars.find(varId) != headerVars.end())
        {
            return headerVars[varId];
        }
        return "";            
    }

       
}
