/**
 *
 *  ElasticSearchClient.h
 *
 */

#pragma once

#include <cstdint>
#include <cstdio>
#include <drogon/HttpTypes.h>
#include <drogon/plugins/Plugin.h>
#include <exception>
#include <functional>
#include <json/value.h>
#include <memory>
#include <regex>
#include <trantor/utils/Date.h>
#include <trantor/utils/Logger.h>

namespace tl::elasticsearch {

class CreateIndexResponse;
using CreateIndexResponsePtr = std::shared_ptr<CreateIndexResponse>;
class Property;
using PropertyPtr = std::shared_ptr<Property>;
class CreateIndexParam;
using CreateIndexParamPtr = std::shared_ptr<CreateIndexParam>;
class Settings;
using SettingsPtr = std::shared_ptr<Settings>;
class GetIndexResponse;
using GetIndexResponsePtr = std::shared_ptr<GetIndexResponse>;
class PutMappingResponse;
using PutMappingResponsePtr = std::shared_ptr<PutMappingResponse>;
class PutMappingParam;
using PutMappingParamPtr = std::shared_ptr<PutMappingParam>;
class DeleteIndexResponse;
using DeleteIndexResponsePtr = std::shared_ptr<DeleteIndexResponse>;
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
public:
    void setByJson(const std::shared_ptr<Json::Value> &responseBody);
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

enum PropertyType {
    NONE,
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

inline std::string to_string(const PropertyType &propertyType) {
    if (propertyType == NONE) {
        return "none";
    }
    if (propertyType == TEXT) {
        return "text";
    }
    if (propertyType == KEYWORD) {
        return "keyword";
    }
    if (propertyType == LONG) {
        return "long";
    }
    if (propertyType == INTEGER) {
        return "integer";
    }
    if (propertyType == SHORT) {
        return "short";
    }
    if (propertyType == BYTE) {
        return "byte";
    }
    if (propertyType == DOUBLE) {
        return "double";
    }
    if (propertyType == FLOAT) {
        return "float";
    }
    if (propertyType == BOOLEAN) {
        return "boolean";
    }
    if (propertyType == DATE) {
        return "date";
    }
}

inline PropertyType string2PropertyType(const std::string &str) {
    PropertyType result;
    if (str == "text") {
        result = TEXT;
    } else if (str == "keyword") {
        result = KEYWORD;
    } else if (str == "long") {
        result = LONG;
    } else if (str == "integer") {
        result = INTEGER;
    } else if (str == "short") {
        result = SHORT;
    } else if (str == "byte") {
        result = BYTE;
    } else if (str == "double") {
        result = DOUBLE;
    } else if (str == "float") {
        result = FLOAT;
    } else if (str == "boolean") {
        result = BOOLEAN;
    } else if (str == "date") {
        result = DATE;
    } else if (str == "none") {
        result = NONE;
    } else {
        std::string error_message = "error property type: ";
        error_message += str;
        throw tl::elasticsearch::ElasticSearchException(error_message);
    }
    return result;
}

class Property {
    friend class CreateIndexParam;
public:
    Property(
        std::string property_name,
        bool index = false
    )
        : property_name_(property_name),
        type_(NONE),
        index_(index)
    {}
    Property(
        std::string property_name,
        PropertyType type,
        bool index = true
    )
        : property_name_(property_name),
        type_(type),
        index_(index)
    {
        if (type == TEXT) {
            analyzer_ = "standard";
        }
    }
    Property(
        std::string property_name,
        PropertyType type,
        std::string analyzer,
        bool index = true
    )
        : property_name_(property_name),
        type_(type),
        analyzer_(analyzer),
        index_(index)
    {
        if (type != TEXT) {
            LOG_WARN << "type of " << to_string(type) << " not need analyzer but set.";
        }
    }
    Property &addSubProperty(Property subProperty) {
        if (type_ != NONE) {
            std::string error_message = "Property of ";
            error_message += to_string(type_);
            error_message += " CANNOT have children.";
            throw tl::elasticsearch::ElasticSearchException(error_message);
        }
        properties_.push_back(subProperty);
        return *this;
    }
public:
    std::string getPropertyName() const {
        return property_name_;
    }
    PropertyType getType() const {
        return type_;
    }
    bool getIndex() const {
        return index_;
    }
    std::string getAnalyzer() const {
        return analyzer_;
    }
    std::vector<Property> getProperties() const {
        return properties_;
    }
private:
    std::string property_name_;
    PropertyType type_ = NONE;
    bool index_;
    std::string analyzer_;
    std::vector<Property> properties_;
};

class CreateIndexParam {
public:
    CreateIndexParam(
        int32_t numberOfShards = 5,
        int32_t numberOfReplicas = 1)
        : number_of_shards_(numberOfShards),
        number_of_replicas_(numberOfReplicas)
    {}
    CreateIndexParam &addProperty(Property const &property);
    std::string toJsonString() const;
private:
    int32_t number_of_shards_;
    int32_t number_of_replicas_;
    std::vector<Property> properties_;
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
    std::vector<Property> getProperties() {
        return properties_;
    }
    std::vector<std::string> getAliases() {
        return aliases_;
    }
    SettingsPtr getSettings() {
        return settings_;
    }
private:
    std::vector<Property> properties_;
    std::vector<std::string> aliases_;
    SettingsPtr settings_;
};

class PutMappingResponse {
public:
    void setByJson(const std::shared_ptr<Json::Value>&responseBody);
    bool isAcknowledged() {
        return acknowledged_;
    }
private:
    bool acknowledged_;
};

class PutMappingParam {
public:
    PutMappingParam &addProperty(Property const &property);
    std::string toJsonString() const;
private:
    std::vector<Property> properties_;
};

class DeleteIndexResponse {
public:
    void setByJson(const std::shared_ptr<Json::Value>&responseBody);
    bool isAcknowledged() {
        return acknowledged_;
    }
private:
    bool acknowledged_;
};

class IndicesClient {
public:
    IndicesClient(std::string url)
        : url_(url)
    {}
    ~IndicesClient() {}
public:
    CreateIndexResponsePtr create(
        std::string indexName,
        const CreateIndexParam &param = CreateIndexParam());
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

    PutMappingResponsePtr putMapping(
        std::string indexName,
        const PutMappingParam &param);
    void putMapping(
        std::string indexName,
        std::function<void (PutMappingResponsePtr &)> &&resultCallback,
        std::function<void (ElasticSearchException &&)> &&exceptionCallback,
        const PutMappingParam &param);

    DeleteIndexResponsePtr deleteIndex(std::string indexName);
    void deleteIndex(
        std::string indexName,
        std::function<void (DeleteIndexResponsePtr &)> &&resultCallback,
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
