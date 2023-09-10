/**
 *
 *  IndicesClient.cc
 *
 */

#include "IndicesClient.h"
#include <iostream>

using namespace std;
using namespace tl::elasticsearch;

void CreateIndexResponse::setByJson(const Json::Value &response)
{
    this->acknowledged_ = response["acknowledged"].asBool();
    this->shards_acknowledged_ = response["shards_acknowledged"].asBool();
    this->index_ = response["index"].asString();
}

CreateIndexParam &CreateIndexParam::addProperty(const Property &property)
{
    this->properties_.push_back(property);
    return *this;
}

Json::Value CreateIndexParam::toJson() const
{
    Json::Value result;
    Json::Value settings;
    settings["number_of_shards"] = number_of_shards_;
    settings["number_of_replicas"] = number_of_replicas_;
    result["settings"] = settings;

    Json::Value properties;
    for (auto &item : properties_)
    {
        if (item.getType() != NONE)
        {
            Json::Value propertyValue;
            propertyValue["type"] = to_string(item.getType());
            if (!item.getIndex())
            {
                propertyValue["index"] = false;
            }
            if (item.getType() == TEXT)
            {
                propertyValue["analyzer"] = item.getAnalyzer();
            }
            properties[item.getPropertyName()] = propertyValue;
        }
        else
        {
            Json::Value subProperties;
            for (auto &sub_item : item.getProperties())
            {
                Json::Value subProperty;
                subProperty["type"] = to_string(sub_item.getType());
                if (!sub_item.getIndex())
                {
                    subProperty["index"] = false;
                }
                if (sub_item.getType() == TEXT)
                {
                    subProperty["analyzer"] = sub_item.getAnalyzer();
                }
                subProperties[sub_item.getPropertyName()] = subProperty;
            }
            properties[item.getPropertyName()]["properties"] = subProperties;
        }
    }

    if (!properties.empty())
    {
        Json::Value mappings;
        Json::Value _doc;
        _doc["properties"] = properties;
        mappings["_doc"] = _doc;
        result["mappings"] = mappings;
    }

    return move(result);
}

string CreateIndexParam::toJsonString() const
{
    return toJson().toStyledString();
}

void Settings::setByJson(const Json::Value &json)
{
    auto index = json["index"];
    auto timeStampStr = index["creation_date"].asCString();
    auto timeStamp = atol(timeStampStr);
    this->creation_date_ = trantor::Date(timeStamp * 1000);

    auto number_of_shards = index.get("number_of_shards", "5").asString();
    this->number_of_shards_ = stoi(number_of_shards);

    auto number_of_replicas = index.get("number_of_replicas", "1").asString();
    this->number_of_replicas_ = stoi(number_of_replicas);

    this->uuid_ = index["uuid"].asString();
    this->version_created_ = index["version"]["created"].asString();
    this->provided_name_ = index["provided_name"].asString();
}

void GetIndexResponse::setByJson(const Json::Value &responseBody)
{
    auto aliasesNodes = responseBody["aliases"];
    this->aliases_ = aliasesNodes.getMemberNames();

    auto mappings = responseBody["mappings"];
    auto properties = mappings["_doc"]["properties"];
    auto propertiesNames = properties.getMemberNames();
    for (auto &name : propertiesNames)
    {
        auto propertyNode = properties[name];

        if (propertyNode.isMember("properties"))
        {
            auto subProperties = propertyNode["properties"];
            auto subPropertiesNames = subProperties.getMemberNames();
            Property property(name);
            for (auto &subName : subPropertiesNames)
            {
                auto subPropertyNode = subProperties[subName];

                auto type = subPropertyNode.get("type", "none").asString();
                PropertyType propertyType = string_to_property_type(type);
                auto index = subPropertyNode.get("index", true).asBool();
                auto analyzer = subPropertyNode["analyzer"].asString();

                switch (propertyType)
                {
                    case NONE:
                        property.addSubProperty(Property(subName));
                        break;
                    case TEXT:
                        property.addSubProperty(
                            Property(subName, TEXT, analyzer.c_str(), index));
                        break;
                    default:
                        property.addSubProperty(
                            Property(subName, propertyType, index));
                }
            }
            this->properties_.push_back(property);
        }
        else
        {
            auto type = propertyNode.get("type", "none").asString();
            PropertyType propertyType = string_to_property_type(type);
            auto index = propertyNode.get("index", true).asBool();
            auto analyzer = propertyNode["analyzer"].asString();
            switch (propertyType)
            {
                case NONE:
                    this->properties_.push_back(Property(name));
                    break;
                case TEXT:
                    this->properties_.push_back(
                        Property(name, TEXT, analyzer.c_str(), index));
                    break;
                default:
                    this->properties_.push_back(
                        Property(name, propertyType, index));
            }
        }
    }

    this->settings_ = make_shared<Settings>();
    auto settings = responseBody["settings"];
    this->settings_->setByJson(settings);
}

void PutMappingResponse::setByJson(const Json::Value &response)
{
    this->acknowledged_ = response["acknowledged"].asBool();
}

PutMappingParam &PutMappingParam::addProperty(const Property &property)
{
    this->properties_.push_back(property);
    return *this;
}

const Json::Value PutMappingParam::toJson() const
{
    Json::Value properties;
    for (auto &item : properties_)
    {
        if (item.getType() != NONE)
        {
            Json::Value propertyValue;
            propertyValue["type"] = to_string(item.getType());
            if (!item.getIndex())
            {
                propertyValue["index"] = false;
            }
            if (item.getType() == TEXT)
            {
                propertyValue["analyzer"] = item.getAnalyzer();
            }
            properties[item.getPropertyName()] = propertyValue;
        }
        else
        {
            Json::Value subProperties;
            for (auto &sub_item : item.getProperties())
            {
                Json::Value subProperty;
                subProperty["type"] = to_string(sub_item.getType());
                if (!sub_item.getIndex())
                {
                    subProperty["index"] = false;
                }
                if (sub_item.getType() == TEXT)
                {
                    subProperty["analyzer"] = sub_item.getAnalyzer();
                }
                subProperties[sub_item.getPropertyName()] = subProperty;
            }
            properties[item.getPropertyName()]["properties"] = subProperties;
        }
    }

    Json::Value result;
    result["properties"] = properties;

    return move(result);
}

const string PutMappingParam::toJsonString() const
{
    return this->toJson().toStyledString();
}

void DeleteIndexResponse::setByJson(const Json::Value &response)
{
    this->acknowledged_ = response["acknowledged"].asBool();
}

CreateIndexResponsePtr IndicesClient::create(
    const string &indexName,
    const CreateIndexParam &param) const
{
    unique_ptr<promise<CreateIndexResponsePtr>> pro(
        new promise<CreateIndexResponsePtr>);
    auto f = pro->get_future();
    this->create(
        indexName,
        [&pro](const CreateIndexResponsePtr &response) {
            try
            {
                pro->set_value(response);
            }
            catch (...)
            {
                pro->set_exception(current_exception());
            }
        },
        [&pro](const ElasticSearchException &err) {
            pro->set_exception(make_exception_ptr(err));
        },
        param);
    return f.get();
}

void IndicesClient::create(
    const string &indexName,
    const function<void(const CreateIndexResponsePtr &)> &resultCallback,
    const function<void(const ElasticSearchException &)> &exceptionCallback,
    const CreateIndexParam &param) const
{
    string path("/");
    path += indexName;

    httpClient_->sendRequest(
        path,
        drogon::Put,
        [resultCallback = std::move(resultCallback),
         exceptionCallback =
             std::move(exceptionCallback)](const Json::Value &responseBody) {
            if (responseBody.isMember("error"))
            {
                auto error = responseBody["error"];
                auto type = error["type"].asString();
                auto reason = error["reason"].asString();
                string errorMessage = "ElasticSearchException [type=";
                errorMessage += type;
                errorMessage += ", reason=";
                errorMessage += reason;
                errorMessage += "]";
                exceptionCallback(ElasticSearchException(errorMessage));
            }
            else
            {
                CreateIndexResponsePtr ci_result =
                    make_shared<CreateIndexResponse>();
                ci_result->setByJson(responseBody);
                resultCallback(ci_result);
            }
        },
        std::move(exceptionCallback),
        param.toJson());
}

GetIndexResponsePtr IndicesClient::get(const string &indexName) const
{
    unique_ptr<promise<GetIndexResponsePtr>> pro(
        new promise<GetIndexResponsePtr>);
    auto f = pro->get_future();
    this->get(
        indexName,
        [&pro](const GetIndexResponsePtr &response) {
            try
            {
                pro->set_value(response);
            }
            catch (...)
            {
                pro->set_exception(current_exception());
            }
        },
        [&pro](const ElasticSearchException &err) {
            pro->set_exception(make_exception_ptr(err));
        });
    return f.get();
}

void IndicesClient::get(
    const string &indexName,
    const function<void(const GetIndexResponsePtr &)> &resultCallback,
    const function<void(const ElasticSearchException &)> &exceptionCallback)
    const
{
    string path("/");
    path += indexName;

    httpClient_->sendRequest(
        path,
        drogon::Get,
        [indexName = std::move(indexName),
         resultCallback = std::move(resultCallback),
         exceptionCallback =
             std::move(exceptionCallback)](const Json::Value &responseBody) {
            if (responseBody.isMember("error"))
            {
                auto error = responseBody["error"];
                auto type = error["type"].asString();
                auto reason = error["reason"].asString();
                string errorMessage = "ElasticSearchException [type=";
                errorMessage += type;
                errorMessage += ", reason=";
                errorMessage += reason;
                errorMessage += "]";
                exceptionCallback(ElasticSearchException(errorMessage));
            }
            else
            {
                GetIndexResponsePtr response = make_shared<GetIndexResponse>();
                response->setByJson(responseBody[indexName]);
                resultCallback(response);
            }
        },
        std::move(exceptionCallback));
}

PutMappingResponsePtr IndicesClient::putMapping(
    const string &indexName,
    const PutMappingParam &param) const
{
    unique_ptr<promise<PutMappingResponsePtr>> pro(
        new promise<PutMappingResponsePtr>);
    auto f = pro->get_future();
    this->putMapping(
        indexName,
        [&pro](const PutMappingResponsePtr &response) {
            try
            {
                pro->set_value(response);
            }
            catch (...)
            {
                pro->set_exception(current_exception());
            }
        },
        [&pro](const ElasticSearchException &err) {
            pro->set_exception(make_exception_ptr(err));
        },
        param);
    return f.get();
}

void IndicesClient::putMapping(
    const string &indexName,
    const function<void(const PutMappingResponsePtr &)> &resultCallback,
    const function<void(const ElasticSearchException &)> &exceptionCallback,
    const PutMappingParam &param) const
{
    string path("/");
    path += indexName;
    path += "/_mapping/_doc";

    httpClient_->sendRequest(
        path,
        drogon::Put,
        [resultCallback = std::move(resultCallback),
         exceptionCallback =
             std::move(exceptionCallback)](const Json::Value &responseBody) {
            if (responseBody.isMember("error"))
            {
                auto error = responseBody["error"];
                auto type = error["type"].asString();
                auto reason = error["reason"].asString();
                string errorMessage = "ElasticSearchException [type=";
                errorMessage += type;
                errorMessage += ", reason=";
                errorMessage += reason;
                errorMessage += "]";
                exceptionCallback(ElasticSearchException(errorMessage));
            }
            else
            {
                PutMappingResponsePtr response =
                    make_shared<PutMappingResponse>();
                response->setByJson(responseBody);
                resultCallback(response);
            }
        },
        std::move(exceptionCallback),
        param.toJson());
}

DeleteIndexResponsePtr IndicesClient::deleteIndex(const string &indexName) const
{
    unique_ptr<promise<DeleteIndexResponsePtr>> pro(
        new promise<DeleteIndexResponsePtr>);
    auto f = pro->get_future();
    this->deleteIndex(
        indexName,
        [&pro](const DeleteIndexResponsePtr &response) {
            try
            {
                pro->set_value(response);
            }
            catch (...)
            {
                pro->set_exception(current_exception());
            }
        },
        [&pro](const ElasticSearchException &err) {
            pro->set_exception(make_exception_ptr(err));
        });
    return f.get();
}

void IndicesClient::deleteIndex(
    const string &indexName,
    const function<void(const DeleteIndexResponsePtr &)> &resultCallback,
    const function<void(const ElasticSearchException &)> &exceptionCallback)
    const
{
    string path("/");
    path += indexName;

    httpClient_->sendRequest(
        path,
        drogon::Delete,
        [resultCallback = std::move(resultCallback),
         exceptionCallback =
             std::move(exceptionCallback)](const Json::Value &responseBody) {
            if (responseBody.isMember("error"))
            {
                auto error = responseBody["error"];
                auto type = error["type"].asString();
                auto reason = error["reason"].asString();
                string errorMessage = "ElasticSearchException [type=";
                errorMessage += type;
                errorMessage += ", reason=";
                errorMessage += reason;
                errorMessage += "]";
                exceptionCallback(ElasticSearchException(errorMessage));
            }
            else
            {
                DeleteIndexResponsePtr response =
                    make_shared<DeleteIndexResponse>();
                response->setByJson(responseBody);
                resultCallback(response);
            }
        },
        std::move(exceptionCallback));
}
