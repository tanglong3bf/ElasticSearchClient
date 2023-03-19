/**
 *
 *  ElasticSearchClient.h
 *
 */

#pragma once

#include <cstdint>
#include <cstdio>
#include <drogon/plugins/Plugin.h>
#include <exception>
#include <json/value.h>
#include <memory>
#include <regex>
#include <trantor/utils/Date.h>
#include <trantor/utils/Logger.h>
#include <unordered_map>

namespace tl {
namespace elasticsearch {

class CreateIndexResponse;
using CreateIndexResponsePtr = std::shared_ptr<CreateIndexResponse>;
class CreateIndexSuccessResponse;
using CreateIndexSuccessResponsePtr = std::shared_ptr<CreateIndexSuccessResponse>;
class ResponseErrorData;
using ResponseErrorDataPtr  = std::shared_ptr<ResponseErrorData>;
class CreateIndexFailedResponse;
using CreateIndexFailedResponsePtr = std::shared_ptr<CreateIndexFailedResponse>;
class Property;
using PropertyPtr = std::shared_ptr<Property>;
class CreateIndexParam;
using CreateIndexParamPtr = std::shared_ptr<CreateIndexParam>;
class Settings;
using SettingsPtr = std::shared_ptr<Settings>;
class GetIndexResponse;
using GetIndexResponsePtr = std::shared_ptr<GetIndexResponse>;
class IndicesClient;
using IndicesClientPtr = std::shared_ptr<IndicesClient>;

class ElasticSearchException : public std::exception {
public:
    const char *what() const noexcept override
    {
        return message_.data();
    }
    ElasticSearchException(const std::string &message)
        : message_(message)
    {}
    ElasticSearchException(std::string &&message)
        : message_(std::move(message))
    {}
    ElasticSearchException() = delete;
private:
    std::string message_;
};

class CreateIndexResponse {
protected:
    CreateIndexResponse(std::string className)
    {}
public:
    virtual ~CreateIndexResponse() {}
public:
    virtual bool isSuccess() = 0;
    virtual void setByJson(const std::shared_ptr<Json::Value>&) = 0;
    virtual std::string className() {
        return class_name_;
    }
    static CreateIndexResponsePtr makeResponseFromJson(const std::shared_ptr<Json::Value> &responseBody);
private:
    std::string class_name_;
};

class CreateIndexSuccessResponse : public CreateIndexResponse {
public:
    CreateIndexSuccessResponse()
        : CreateIndexResponse("CreateIndexSuccessResponse")
    {}
public:
    bool isSuccess() override {
        return true;
    }
    virtual void setByJson(const std::shared_ptr<Json::Value>&) override;
    bool isAcknowledged() {
        return acknowledged_;
    }
    bool isShardsAcknowledged() {
        return shards_acknowledged_;
    }
    std::string getIndex() {
        return index_;
    }
private:
    bool acknowledged_;
    bool shards_acknowledged_;
    std::string index_;
};

class ResponseErrorData {
    friend class CreateIndexFailedResponse;
public:
    std::string getType() {
        return type_;
    }
    std::string getReason() {
        return reason_;
    }
    std::string getIndexUuid() {
        return index_uuid_;
    }
    std::string getIndex() {
        return index_;
    }
private:
    std::string type_;
    std::string reason_;
    std::string index_uuid_;
    std::string index_;
};

class CreateIndexFailedResponse : public CreateIndexResponse {
public:
    CreateIndexFailedResponse()
        : CreateIndexResponse("CreateIndexFailedResponse")
    {}
public:
    bool isSuccess() override {
        return false;
    }
    virtual void setByJson(const std::shared_ptr<Json::Value>&) override;
    ResponseErrorDataPtr getError() {
        return error_;
    }
    int32_t getStatus() {
        return status_;
    }
private:
    ResponseErrorDataPtr error_;
    int32_t status_;
};

enum PropertyType {
    TEXT,
    KEYWORD,
    LONG,
    INTEGER,
    SHORT,
    BYTE,
    DOUBLE,
    FLOAT,
    BOOLEAN,
    DATE,
};

inline PropertyType string2PropertyType(std::string &str) {
    PropertyType result;
    if (str == "TEXT") {
        result = TEXT;
    }
    if (str == "KEYWORD") {
        result = KEYWORD;
    }
    if (str == "LONG") {
        result = LONG;
    }
    if (str == "INTEGER") {
        result = INTEGER;
    }
    if (str == "SHORT") {
        result = SHORT;
    }
    if (str == "BYTE") {
        result = BYTE;
    }
    if (str == "DOUBLE") {
        result = DOUBLE;
    }
    if (str == "FLOAT") {
        result = FLOAT;
    }
    if (str == "BOOLEAN") {
        result = BOOLEAN;
    }
    if (str == "DATE") {
        result = DATE;
    }
    return result;
}

class Property {
    friend class CreateIndexParam;
public:
    Property(std::string property_name,
        PropertyType type,
        std::string analyzer = "standard"
    )
        : property_name_(property_name),
        type_(type),
        analyzer_(analyzer)
    {}
public:
    std::string getPropertyName() {
        return property_name_;
    }
    PropertyType getType() {
        return type_;
    }
    std::string getAnalyzer() {
        return analyzer_;
    }
private:
    std::string property_name_;
    PropertyType type_;
    std::string analyzer_;
};

class CreateIndexParam {
public:
    CreateIndexParam(int32_t numberOfShards = 5, int32_t numberOfReplicas = 1)
        : number_of_shards_(numberOfShards),
        number_of_replicas_(numberOfReplicas)
    {}
    CreateIndexParam &addProperty(Property* const property);
    std::string toJsonString() const;
private:
    int32_t number_of_shards_;
    int32_t number_of_replicas_;
    std::vector<PropertyPtr> properties_;
};

class Settings {
public:
    void setByJson(const std::shared_ptr<Json::Value> &json);
    trantor::Date getCreationDate() {
        return creation_date_;
    }
    int32_t getNumberOfShards() {
        return number_of_shards_;
    }
    int32_t getNumberOfReplicas() {
        return number_of_replicas_;
    }
    std::string getUuid() {
        return uuid_;
    }
    std::string getVersionCreated() {
        return version_created_;
    }
    std::string getProvidedName() {
        return provided_name_;
    }
private:
    trantor::Date creation_date_;
    int32_t number_of_shards_;
    int32_t number_of_replicas_;
    std::string uuid_;
    std::string version_created_;
    std::string provided_name_;
};

class GetIndexResponse {
public:
    void setByJson(const std::shared_ptr<Json::Value>&responseBody);
    std::vector<PropertyPtr> getProperties() {
        return properties_;
    }
    std::vector<std::string> getAliases() {
        return aliases_;
    }
    SettingsPtr getSettings() {
        return settings_;
    }
private:
    std::vector<PropertyPtr> properties_;
    std::vector<std::string> aliases_;
    SettingsPtr settings_;
};

class IndicesClient {
public:
    IndicesClient(std::string url)
        : url_(url)
    {}
    ~IndicesClient() {}
public:
    CreateIndexResponsePtr create(std::string indexName, const CreateIndexParam &param = CreateIndexParam());
    void create(
        std::string indexName,
        std::function<void (CreateIndexResponsePtr &)> &&resultCallback,
        std::function<void (ElasticSearchException &&)> &&exceptionCallback,
        const CreateIndexParam &param = CreateIndexParam());

    GetIndexResponsePtr get(std::string indexName);
    void get(
        std::string indexName,
        std::function<void (GetIndexResponsePtr &)> &&resultCallback,
        std::function<void (ElasticSearchException &&)> &&exceptionCallback);
private:
    std::string url_;
};

class ElasticSearchClient : public drogon::Plugin<ElasticSearchClient>
{
public:
    /// This method must be called by drogon to initialize and start the plugin.
    /// It must be implemented by the user.
    void initAndStart(const Json::Value &config) override;

    /// This method must be called by drogon to shutdown the plugin.
    /// It must be implemented by the user.
    void shutdown() override;
public:
    IndicesClientPtr indices();
private:
    IndicesClientPtr indices_;
private:
    std::string host_;
    int16_t port_;
};

}; // namespace: tl::elasticsearch
}; // namespace: tl
