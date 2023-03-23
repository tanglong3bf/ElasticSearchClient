/**
 *
 *  ElasticSearchClient.cc
 *
 */

#include "ElasticSearchClient.h"
#include <cstdlib>
#include <drogon/HttpRequest.h>
#include <drogon/HttpTypes.h>
#include <drogon/drogon_callbacks.h>
#include <exception>
#include <future>
#include <json/value.h>
#include <drogon/HttpClient.h>
#include <memory>
#include <trantor/utils/Date.h>
#include <trantor/utils/Logger.h>

using namespace std;

using namespace drogon;

using namespace tl::elasticsearch;

void CreateIndexResponse::setByJson(const std::shared_ptr<Json::Value> &response) {
    this->acknowledged_ = response->get("acknowledged", Json::Value(false)).asBool();
    this->shards_acknowledged_ = response->get("shards_acknowledged", Json::Value(false)).asBool();
    this->index_ = response->get("index", Json::Value("")).asString();
}

CreateIndexParam &CreateIndexParam::addProperty(Property const &property) {
    this->properties_.push_back(property);
    return *this;
}

std::string CreateIndexParam::toJsonString() const {
    Json::Value result;
    Json::Value settings;
    settings["number_of_shards"] = number_of_shards_;
    settings["number_of_replicas"] = number_of_replicas_;
    result["settings"] = settings;

    Json::Value properties;
    for (auto &item : properties_) {
        if (item.type_ != NONE) {
            Json::Value propertyValue;
            propertyValue["type"] = to_string(item.type_);
            if (item.type_ == TEXT) {
                propertyValue["analyzer"] = item.analyzer_;
            }
            properties[item.property_name_] = propertyValue;
        } else {
            Json::Value subProperties;
            for (auto &sub_item : item.properties_) {
                Json::Value subProperty;
                subProperty["type"] = to_string(sub_item.type_);
                if (sub_item.type_ == TEXT) {
                    subProperty["analyzer"] = sub_item.analyzer_;
                }
                subProperties[sub_item.property_name_] = subProperty;
            }
            properties[item.property_name_]["properties"] = subProperties;
        }
    }

    Json::Value _doc;
    _doc["properties"] = properties;

    Json::Value mappings;
    mappings["_doc"] = _doc;
    result["mappings"] = mappings;

    return result.toStyledString();
}

void Settings::setByJson(const std::shared_ptr<Json::Value> &json) {
    auto index = json->get("index", {});
    auto timeStampStr = index.get("creation_date", {}).asString();
    auto timeStamp = std::atol(timeStampStr.c_str());
    this->creation_date_ = trantor::Date(timeStamp * 1000);

    auto number_of_shards = index.get("number_of_shards", "5").asString();
    this->number_of_shards_ = std::atoi(number_of_shards.c_str());

    auto number_of_replicas = index.get("number_of_replicas", "1").asString();
    this->number_of_replicas_ = std::atoi(number_of_replicas.c_str());

    this->uuid_ = index.get("uuid", {}).asString();
    this->version_created_ = index.get("version", {}).get("created", "").asString();
    this->provided_name_ = index.get("provided_name", {}).asString();
}

void GetIndexResponse::setByJson(const std::shared_ptr<Json::Value> &responseBody) {
    auto aliasesNodes = responseBody->get("aliases", {});
    this->aliases_ = aliasesNodes.getMemberNames();

    auto mappings = responseBody->get("mappings", {});
    auto _doc = mappings.get("_doc", {});
    auto properties = _doc.get("properties", {});
    auto propertiesNames = properties.getMemberNames();
    for (auto &name : propertiesNames) {
        auto propertyNode = properties.get(name, {});

        auto type = propertyNode.get("type", {}).asString();
        PropertyType propertyType = string2PropertyType(type);

        auto analyzer = propertyNode.get("analyzer", {}).asString();
        this->properties_.push_back(make_shared<Property>(name, propertyType, analyzer));
    }

    this->settings_ = make_shared<Settings>();
    auto settings = responseBody->get("settings", {});
    this->settings_->setByJson(make_shared<Json::Value>(settings));
}

void DeleteIndexResponse::setByJson(const std::shared_ptr<Json::Value> &response) {
    this->acknowledged_ = response->get("acknowledged", false).asBool();
}

CreateIndexResponsePtr IndicesClient::create(std::string indexName, const CreateIndexParam &param) {
    shared_ptr<promise<CreateIndexResponsePtr>> pro(new promise<CreateIndexResponsePtr>);
    auto f = pro->get_future();
    this->create(indexName, [&pro](CreateIndexResponsePtr &response) {
        try {
            pro->set_value(response);
        }
        catch (...) {
            pro->set_exception(std::current_exception());
        }
    }, [&pro](ElasticSearchException &&err){
        pro->set_exception(std::make_exception_ptr(err));
    }, param);
    return f.get();
}

void IndicesClient::create(
    string indexName,
    function<void (CreateIndexResponsePtr &)> &&resultCallback,
    function<void (ElasticSearchException &&)> &&exceptionCallback,
    const CreateIndexParam &param) {
    string path("/");
    path += indexName;

    auto req = HttpRequest::newHttpRequest();
    req->setMethod(Put);
    req->setPath(path);
    req->setContentTypeCode(CT_APPLICATION_JSON);
    req->setBody(param.toJsonString());

    auto client = HttpClient::newHttpClient(url_);
    client->sendRequest(req, [
        this,
        resultCallback = move(resultCallback),
        exceptionCallback = move(exceptionCallback)
    ] (ReqResult result, const HttpResponsePtr &response) {
        if (result != ReqResult::Ok) {
            string errorMessage = "failed error while sending request to server! url: [";
            errorMessage += url_;
            errorMessage += "], result: [";
            errorMessage += to_string(result);
            errorMessage += "].";

            LOG_WARN << errorMessage;
            exceptionCallback(ElasticSearchException(errorMessage));
        } else {
            auto responseBody = response->getJsonObject();

            LOG_TRACE << responseBody->toStyledString();

            if (responseBody->isMember("error")) {
                auto error = responseBody->get("error", {});
                auto type = error.get("type", {}).asString();
                auto reason = error.get("reason", {}).asString();
                string errorMessage = "ElasticSearchException [type=";
                errorMessage += type;
                errorMessage += ", reason=";
                errorMessage += reason;
                errorMessage += "]";
                exceptionCallback(ElasticSearchException(errorMessage));
            } else {
                CreateIndexResponsePtr ci_result = make_shared<CreateIndexResponse>();
                ci_result->setByJson(responseBody);
                resultCallback(ci_result);
            }
        }
    });
}

GetIndexResponsePtr IndicesClient::get(std::string indexName) {
    shared_ptr<promise<GetIndexResponsePtr>> pro(new promise<GetIndexResponsePtr>);
    auto f = pro->get_future();
    this->get(indexName, [&pro](GetIndexResponsePtr &response) {
        try {
            pro->set_value(response);
        }
        catch (...) {
            pro->set_exception(std::current_exception());
        }
    }, [&pro](ElasticSearchException &&err){
        pro->set_exception(std::make_exception_ptr(err));
    });
    return f.get();
}

void IndicesClient::get(
    std::string indexName,
    std::function<void (GetIndexResponsePtr &)> &&resultCallback,
    std::function<void (ElasticSearchException &&)> &&exceptionCallback) {

    string path("/");
    path += indexName;

    auto req = HttpRequest::newHttpRequest();
    req->setMethod(Get);
    req->setPath(path);
    req->setContentTypeCode(CT_APPLICATION_JSON);

    auto client = HttpClient::newHttpClient(url_);
    client->sendRequest(req, [
        this,
        &indexName,
        resultCallback = move(resultCallback),
        exceptionCallback = move(exceptionCallback)
    ] (ReqResult result, const HttpResponsePtr &response) {
        if (result != ReqResult::Ok) {
            string errorMessage = "error while sending request to server! url: [";
            errorMessage +=  url_;
            errorMessage +=  "], result: [";
            errorMessage +=  to_string(result);
            errorMessage +=  "].";

            LOG_WARN << errorMessage;
            exceptionCallback(ElasticSearchException(errorMessage));
        } else {
            auto responseBody = response->getJsonObject();

            LOG_TRACE << responseBody->toStyledString();

            if (responseBody->isMember("error")) {
                auto error = responseBody->get("error", {});
                auto type = error.get("type", {}).asString();
                auto reason = error.get("reason", {}).asString();
                string errorMessage = "ElasticSearchException [type=";
                errorMessage += type;
                errorMessage += ", reason=";
                errorMessage += reason;
                errorMessage += "]";
                exceptionCallback(ElasticSearchException(errorMessage));
            } else {
                GetIndexResponsePtr response = std::make_shared<GetIndexResponse>();
                response->setByJson(std::make_shared<Json::Value>(responseBody->get(indexName, {})));
                resultCallback(response);
            }
        }
    });
}

DeleteIndexResponsePtr IndicesClient::deleteIndex(std::string indexName) {
    shared_ptr<promise<DeleteIndexResponsePtr>> pro(new promise<DeleteIndexResponsePtr>);
    auto f = pro->get_future();
    this->deleteIndex(indexName, [&pro](DeleteIndexResponsePtr &response) {
        try {
            pro->set_value(response);
        }
        catch (...) {
            pro->set_exception(std::current_exception());
        }
    }, [&pro](ElasticSearchException &&err){
        pro->set_exception(std::make_exception_ptr(err));
    });
    return f.get();
}

void IndicesClient::deleteIndex(
    std::string indexName,
    std::function<void (DeleteIndexResponsePtr &)> &&resultCallback,
    std::function<void (ElasticSearchException &&)> &&exceptionCallback) {

    string path("/");
    path += indexName;

    auto req = HttpRequest::newHttpRequest();
    req->setMethod(Delete);
    req->setPath(path);
    req->setContentTypeCode(CT_APPLICATION_JSON);

    auto client = HttpClient::newHttpClient(url_);
    client->sendRequest(req, [
        this,
        &indexName,
        resultCallback = move(resultCallback),
        exceptionCallback = move(exceptionCallback)
    ] (ReqResult result, const HttpResponsePtr &response) {
        if (result != ReqResult::Ok) {
            string errorMessage = "error while sending request to server! url: [";
            errorMessage +=  url_;
            errorMessage +=  "], result: [";
            errorMessage +=  to_string(result);
            errorMessage +=  "].";

            LOG_WARN << errorMessage;
            exceptionCallback(ElasticSearchException(errorMessage));
        } else {
            auto responseBody = response->getJsonObject();

            LOG_TRACE << responseBody->toStyledString();

            if (responseBody->isMember("error")) {
                auto error = responseBody->get("error", {});
                auto type = error.get("type", {}).asString();
                auto reason = error.get("reason", {}).asString();
                string errorMessage = "ElasticSearchException [type=";
                errorMessage += type;
                errorMessage += ", reason=";
                errorMessage += reason;
                errorMessage += "]";
                exceptionCallback(ElasticSearchException(errorMessage));
            } else {
                DeleteIndexResponsePtr response = std::make_shared<DeleteIndexResponse>();
                response->setByJson(responseBody);
                resultCallback(response);
            }
        }
    });
}

void ElasticSearchClient::initAndStart(const Json::Value &config) {
    /// Initialize and start the plugin
    this->host_ = config.get("host", Json::Value("localhost")).asString();
    this->port_ = config.get("port", Json::Value(9200)).asUInt();

    string url("http://");
    url += this->host_;
    if (this->port_ != 80) {
        url += ":";
        url += std::to_string(this->port_);
    }
    LOG_TRACE << url;

    this->indices_ = IndicesClientPtr(new IndicesClient(url));
}

void ElasticSearchClient::shutdown() {
    /// Shutdown the plugin
}

IndicesClientPtr ElasticSearchClient::indices() {
    return indices_;
}
