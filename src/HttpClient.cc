/**
 *
 *  HttpClient.cc
 *
 */

#include "HttpClient.h"
#include <iostream>

using namespace std;
using namespace tl::elasticsearch;

Json::Value HttpClient::sendRequest(const std::string &path,
                                    drogon::HttpMethod method,
                                    const Json::Value &requestBody)
{
    unique_ptr<promise<Json::Value>> pro(new promise<Json::Value>);
    auto f = pro->get_future();
    this->sendRequest(
        path,
        method,
        [&pro](const Json::Value &response) {
            try
            {
                pro->set_value(response);
            }
            catch (...)
            {
                pro->set_exception(std::current_exception());
            }
        },
        [&pro](const ElasticSearchException &err) {
            pro->set_exception(std::make_exception_ptr(err));
        },
        requestBody);
    return f.get();
}

void HttpClient::sendRequest(
    const std::string &path,
    drogon::HttpMethod method,
    const std::function<void(const Json::Value &)> &resultCallback,
    const std::function<void(const ElasticSearchException &)>
        &exceptionCallback,
    const Json::Value &requestBody)
{
    auto req = drogon::HttpRequest::newHttpRequest();
    req->setMethod(method);
    req->setPath(path);
    req->setContentTypeCode(drogon::CT_APPLICATION_JSON);
    req->setBody(requestBody.toStyledString());

    auto client = drogon::HttpClient::newHttpClient(url_);
    client->sendRequest(
        req,
        [this,
         resultCallback = std::move(resultCallback),
         exceptionCallback = std::move(exceptionCallback)](
            drogon::ReqResult result, const drogon::HttpResponsePtr &response) {
            if (result != drogon::ReqResult::Ok)
            {
                string errorMessage =
                    "failed while sending request to server! url: [";
                errorMessage += url_;
                errorMessage += "], result: [";
                errorMessage += to_string(result);
                errorMessage += "].";

                LOG_WARN << errorMessage;
                exceptionCallback(ElasticSearchException(errorMessage));
            }
            else
            {
                auto responseBody = response->getJsonObject();
                LOG_TRACE << responseBody->toStyledString();
                resultCallback(*responseBody);
            }
        },
        5);
}
