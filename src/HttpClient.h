/**
 *
 *  HttpClient.h
 *
 */

#pragma once

#include <json/json.h>
#include <drogon/HttpClient.h>
#include <memory>
#include "ElasticSearchException.h"

namespace tl::elasticsearch {

class HttpClient {
public:
    HttpClient(std::string url)
        : url_(url)
    {}
public:
    Json::Value sendRequest(
        const std::string &path,
        drogon::HttpMethod method,
        const Json::Value &requestBody = {});

    void sendRequest(const std::string &path,
        drogon::HttpMethod method,
        const std::function<void (Json::Value &)> &&resultCallback,
        const std::function<void (ElasticSearchException &&)> &&exceptionCallback,
        const Json::Value &requestBody = {});
private:
    std::string url_;
};

using HttpClientPtr = std::shared_ptr<HttpClient>;

};
