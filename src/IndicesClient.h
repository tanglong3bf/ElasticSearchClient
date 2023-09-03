/**
 *
 *  IndicesClient.h
 *
 */

#pragma once

#include "HttpClient.h"
#include "Property.h"
#include <json/value.h>
#include <trantor/utils/Date.h>
#include <trantor/utils/Logger.h>

namespace tl::elasticsearch
{

class CreateIndexResponse
{
  public:
    void setByJson(const Json::Value &responseBody);

    bool isAcknowledged()
    {
        return acknowledged_;
    }

    bool isShardsAcknowledged()
    {
        return shards_acknowledged_;
    }

    const std::string &getIndex()
    {
        return index_;
    }

  private:
    bool acknowledged_;
    bool shards_acknowledged_;
    std::string index_;
};

using CreateIndexResponsePtr = std::shared_ptr<CreateIndexResponse>;

class CreateIndexParam
{
  public:
    CreateIndexParam(int32_t numberOfShards = 5, int32_t numberOfReplicas = 1)
        : number_of_shards_(numberOfShards),
          number_of_replicas_(numberOfReplicas)
    {
    }

    CreateIndexParam &addProperty(const Property &property);
    Json::Value toJson() const;
    std::string toJsonString() const;

  private:
    int32_t number_of_shards_;
    int32_t number_of_replicas_;
    std::vector<Property> properties_;
};

class Settings
{
  public:
    void setByJson(const Json::Value &json);

    const trantor::Date &getCreationDate() const
    {
        return creation_date_;
    }

    int32_t getNumberOfShards() const
    {
        return number_of_shards_;
    }

    int32_t getNumberOfReplicas() const
    {
        return number_of_replicas_;
    }

    const std::string &getUuid() const
    {
        return uuid_;
    }

    const std::string &getVersionCreated() const
    {
        return version_created_;
    }

    const std::string &getProvidedName() const
    {
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

using SettingsPtr = std::shared_ptr<Settings>;

class GetIndexResponse
{
  public:
    void setByJson(const Json::Value &responseBody);

    const std::vector<Property> &getProperties() const
    {
        return properties_;
    }

    const std::vector<std::string> &getAliases() const
    {
        return aliases_;
    }

    const SettingsPtr &getSettings() const
    {
        return settings_;
    }

  private:
    std::vector<Property> properties_;
    std::vector<std::string> aliases_;
    SettingsPtr settings_;
};

using GetIndexResponsePtr = std::shared_ptr<GetIndexResponse>;

class PutMappingResponse
{
  public:
    void setByJson(const Json::Value &responseBody);

    bool isAcknowledged() const
    {
        return acknowledged_;
    }

  private:
    bool acknowledged_;
};

using PutMappingResponsePtr = std::shared_ptr<PutMappingResponse>;

class PutMappingParam
{
  public:
    PutMappingParam &addProperty(const Property &property);
    const Json::Value toJson() const;
    const std::string toJsonString() const;

  private:
    std::vector<Property> properties_;
};

class DeleteIndexResponse
{
  public:
    void setByJson(const Json::Value &responseBody);

    bool isAcknowledged() const
    {
        return acknowledged_;
    }

  private:
    bool acknowledged_;
};

using DeleteIndexResponsePtr = std::shared_ptr<DeleteIndexResponse>;

class IndicesClient
{
  public:
    IndicesClient(std::shared_ptr<HttpClient> httpClient)
        : httpClient_(httpClient)
    {
    }

    ~IndicesClient()
    {
    }

  public:
    CreateIndexResponsePtr create(
        const std::string &indexName,
        const CreateIndexParam &param = CreateIndexParam()) const;
    void create(const std::string &indexName,
                const std::function<void(const CreateIndexResponsePtr &)>
                    &resultCallback,
                const std::function<void(const ElasticSearchException &)>
                    &exceptionCallback,
                const CreateIndexParam &param = CreateIndexParam()) const;

    GetIndexResponsePtr get(const std::string &indexName) const;
    void get(
        const std::string &indexName,
        const std::function<void(const GetIndexResponsePtr &)> &resultCallback,
        const std::function<void(const ElasticSearchException &)>
            &exceptionCallback) const;

    PutMappingResponsePtr putMapping(const std::string &indexName,
                                     const PutMappingParam &param) const;
    void putMapping(const std::string &indexName,
                    const std::function<void(const PutMappingResponsePtr &)>
                        &resultCallback,
                    const std::function<void(const ElasticSearchException &)>
                        &exceptionCallback,
                    const PutMappingParam &param) const;

    DeleteIndexResponsePtr deleteIndex(const std::string &indexName) const;
    void deleteIndex(const std::string &indexName,
                     const std::function<void(const DeleteIndexResponsePtr &)>
                         &resultCallback,
                     const std::function<void(const ElasticSearchException &)>
                         &exceptionCallback) const;

  private:
    HttpClientPtr httpClient_;
};

using IndicesClientPtr = std::shared_ptr<IndicesClient>;

};  // namespace tl::elasticsearch
