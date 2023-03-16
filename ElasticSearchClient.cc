/**
 *
 *  ElasticSearchClient.cc
 *
 */

#include "ElasticSearchClient.h"
#include <drogon/HttpRequest.h>
#include <drogon/HttpTypes.h>
#include <drogon/drogon_callbacks.h>
#include <exception>
#include <future>
#include <json/value.h>
#include <drogon/HttpClient.h>
#include <memory>
#include <trantor/utils/Logger.h>

using namespace std;

using namespace drogon;

using namespace tl::elasticsearch;

shared_ptr<CreateIndexResponse> CreateIndexResponse::makeResponseFromJson(const shared_ptr<Json::Value> &responseBody) {
    CreateIndexResponsePtr response;
    if (responseBody->isMember("error")) {
        response = make_shared<CreateIndexFailedResponse>();
    } else {
        response = make_shared<CreateIndexSuccessResponse>();
    }
    response->setByJson(responseBody);
    return response;
}

void CreateIndexSuccessResponse::setByJson(const std::shared_ptr<Json::Value> &response) {
    this->acknowledged_ = response->get("acknowledged", Json::Value(false)).asBool();
    this->shards_acknowledged_ = response->get("shards_acknowledged", Json::Value(false)).asBool();
    this->index_ = response->get("index", Json::Value("")).asString();
}

void CreateIndexFailedResponse::setByJson(const std::shared_ptr<Json::Value> &response) {
    if (!response->isMember("error")) {
        throw new Json::LogicError("missing required child node: ['error']");
    }

    auto error = make_shared<ResponseErrorData>();
    auto errorNode = response->get("error", Json::Value());
    error->type_ = errorNode.get("type", Json::Value("")).asString();
    error->reason_ = errorNode.get("reason", Json::Value("")).asString();
    error->index_uuid_ = errorNode.get("index_uuid", Json::Value("")).asString();
    error->index_ = errorNode.get("index", Json::Value("")).asString();

    this->error_ = error;
    this->status_ = response->get("status", Json::Value(400)).asInt();
}

CreateIndexParam &CreateIndexParam::addProperty(Property* const property) {
    this->properties_.push_back(PropertyPtr(property));
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
        Json::Value propertyValue;
        string type;
        switch (item->type_) {
        case TEXT:
            type = "TEXT";
            break;
        case KEYWORD:
            type = "KEYWORD";
            break;
        case LONG:
            type = "LONG";
            break;
        case INTEGER:
            type = "INTEGER";
            break;
        case SHORT:
            type = "SHORT";
            break;
        case BYTE:
            type = "BYTE";
            break;
        case DOUBLE:
            type = "DOUBLE";
            break;
        case FLOAT:
            type = "FLOAT";
            break;
        case BOOLEAN:
            type = "BOOLEAN";
            break;
        case DATE:
            type = "DATE";
            break;
        }

        propertyValue["type"] = type;
        if (item->type_ == TEXT) {
            propertyValue["analyzer"] = item->analyzer_;
        }
        properties[item->property_name_] = propertyValue;
    }

    Json::Value _doc;
    _doc["properties"] = properties;

    Json::Value mappings;
    mappings["_doc"] = _doc;
    result["mappings"] = mappings;

    return result.toStyledString();
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
    client->sendRequest(req, [this,resultCallback = move(resultCallback), exceptionCallback = move(exceptionCallback)](ReqResult result, const HttpResponsePtr &response) {
        if (result != ReqResult::Ok) {
            LOG_ERROR << "error while sending request to server! url: [" << url_ << "], result: [" << result << "].";
            string errorMessage;

            switch (result) {
            case drogon::ReqResult::BadResponse:
                errorMessage = "bad response";
                break;
            case drogon::ReqResult::NetworkFailure:
                errorMessage = "network failure";
                break;
            case drogon::ReqResult::BadServerAddress:
                errorMessage = "bad server address";
                break;
            case drogon::ReqResult::Timeout:
                errorMessage = "timeout";
                break;
            case drogon::ReqResult::HandshakeError:
                errorMessage = "handshake error";
                break;
            case drogon::ReqResult::InvalidCertificate:
                errorMessage = "invalid certificate";
                break;
            }
            exceptionCallback(ElasticSearchException(errorMessage));

        } else {
            auto responseBody = response->getJsonObject();
            auto ci_result = CreateIndexResponse::makeResponseFromJson(response->getJsonObject());
            resultCallback(ci_result);
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
        url += to_string(this->port_);
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
