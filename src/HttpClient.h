/**
 *
 *  HttpClient.h
 *
 */

#pragma once

#include "ElasticSearchException.h"
#include <drogon/HttpClient.h>
#include <json/json.h>
#include <memory>

namespace tl::elasticsearch
{

class HttpClient
{
  public:
    HttpClient(std::string url) : url_(url)
    {
    }

  public:
    Json::Value sendRequest(
        const std::string &path,
        drogon::HttpMethod method,
        const Json::Value &requestBody = Json::Value(Json::objectValue));

    void sendRequest(
        const std::string &path,
        drogon::HttpMethod method,
        const std::function<void(const Json::Value &)> &resultCallback,
        const std::function<void(const ElasticSearchException &)>
            &exceptionCallback,
        const Json::Value &requestBody = Json::Value(Json::objectValue));

    // _bulk
    Json::Value sendRequest(const std::string &path,
                            drogon::HttpMethod method,
                            const std::vector<Json::Value> &requestBody);

    void sendRequest(
        const std::string &path,
        drogon::HttpMethod method,
        const std::function<void(const Json::Value &)> &resultCallback,
        const std::function<void(const ElasticSearchException &)>
            &exceptionCallback,
        const std::vector<Json::Value> &requestBody);

  private:
    std::string url_;
};

using HttpClientPtr = std::shared_ptr<HttpClient>;

};  // namespace tl::elasticsearch
