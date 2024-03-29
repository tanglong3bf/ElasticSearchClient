#pragma once

#include <functional>
#include <json/value.h>
#include <memory>
#include "Aggregation.h"
#include "ElasticSearchException.h"
#include "HttpClient.h"
#include "Query.h"

namespace tl::elasticsearch
{

class Document
{
  public:
    virtual Json::Value toJson() const = 0;
    virtual void setByJson(const Json::Value &) = 0;
};

template <typename Tp>
concept isDocumentType = std::derived_from<Tp, Document>;

class Shards
{
  public:
    int getFailed() const
    {
        return failed_;
    }

    int getSuccessful() const
    {
        return successful_;
    }

    int getTotal() const
    {
        return total_;
    }

    void setByJson(const Json::Value &json);

  private:
    int failed_;
    int successful_;
    int total_;
};

using ShardsPtr = std::shared_ptr<Shards>;

class IndexResponse
{
  public:
    std::string getId() const
    {
        return id_;
    }

    const std::string &getIndex() const
    {
        return index_;
    }

    int getPrimaryTerm() const
    {
        return primary_term_;
    }

    int getSeqNo() const
    {
        return seq_no_;
    }

    const std::string &getType() const
    {
        return type_;
    }

    ShardsPtr getShards() const
    {
        return shards_;
    }

    int getVersion() const
    {
        return version_;
    }

    const std::string &getResult() const
    {
        return result_;
    }

    void setByJson(const Json::Value &json);

  private:
    std::string id_;
    std::string index_;
    int primary_term_;
    int seq_no_;
    std::string type_;
    ShardsPtr shards_;
    int version_;
    // TODO:
    std::string result_;
};

using IndexResponsePtr = std::shared_ptr<IndexResponse>;

class IndexParam
{
    friend class DocumentsClient;

  public:
    IndexParam(std::string index) : index_(index)
    {
    }

    void setId(std::string id)
    {
        id_ = id;
    }

  private:
    std::string index_;
    std::string id_;
};

class DeleteResponse
{
  public:
    std::string getId() const
    {
        return id_;
    }

    const std::string &getIndex() const
    {
        return index_;
    }

    int getPrimaryTerm() const
    {
        return primary_term_;
    }

    int getSeqNo() const
    {
        return seq_no_;
    }

    const std::string &getType() const
    {
        return type_;
    }

    ShardsPtr getShards() const
    {
        return shards_;
    }

    int getVersion() const
    {
        return version_;
    }

    const std::string &getResult() const
    {
        return result_;
    }

    void setByJson(const Json::Value &json);

  private:
    std::string id_;
    std::string index_;
    int primary_term_;
    int seq_no_;
    std::string type_;
    ShardsPtr shards_;
    int version_;
    // TODO:
    std::string result_;
};

using DeleteResponsePtr = std::shared_ptr<DeleteResponse>;

class DeleteParam
{
    friend class DocumentsClient;

  public:
    DeleteParam(std::string index) : index_(index)
    {
    }

    void setId(std::string id)
    {
        id_ = id;
    }

  private:
    std::string index_;
    std::string id_;
};

class UpdateResponse
{
  public:
    std::string getId() const
    {
        return id_;
    }

    const std::string &getIndex() const
    {
        return index_;
    }

    int getPrimaryTerm() const
    {
        return primary_term_;
    }

    int getSeqNo() const
    {
        return seq_no_;
    }

    const std::string &getType() const
    {
        return type_;
    }

    ShardsPtr getShards() const
    {
        return shards_;
    }

    int getVersion() const
    {
        return version_;
    }

    const std::string &getResult() const
    {
        return result_;
    }

    void setByJson(const Json::Value &json);

  private:
    std::string id_;
    std::string index_;
    int primary_term_;
    int seq_no_;
    std::string type_;
    ShardsPtr shards_;
    int version_;
    // TODO:
    std::string result_;
};

using UpdateResponsePtr = std::shared_ptr<UpdateResponse>;

class UpdateParam
{
    friend class DocumentsClient;

  public:
    UpdateParam(std::string index) : index_(index)
    {
    }

    void setId(std::string id)
    {
        id_ = id;
    }

  private:
    std::string index_;
    std::string id_;
};

class GetResponse
{
  public:
    std::string getId() const
    {
        return id_;
    }

    const std::string &getIndex() const
    {
        return index_;
    }

    int getPrimaryTerm() const
    {
        return primary_term_;
    }

    int getSeqNo() const
    {
        return seq_no_;
    }

    Json::Value getSource() const
    {
        return source_;
    }

    const std::string &getType() const
    {
        return type_;
    }

    int getVersion() const
    {
        return version_;
    }

    const bool getFound() const
    {
        return found_;
    }

    void setByJson(const Json::Value &json);

  private:
    std::string id_;
    std::string index_;
    int primary_term_;
    int seq_no_;
    Json::Value source_;
    std::string type_;
    int version_;
    bool found_;
};

using GetResponsePtr = std::shared_ptr<GetResponse>;

class GetParam
{
    friend class DocumentsClient;

  public:
    GetParam(std::string index) : index_(index)
    {
    }

    void setId(std::string id)
    {
        id_ = id;
    }

  private:
    std::string index_;
    std::string id_;
};

class Sort
{
  public:
    Sort(const std::string &field, SortOrder sortOrder)
        : field_(field), sortOrder_(sortOrder)
    {
    }

  public:
    virtual Json::Value toJson() const
    {
        Json::Value json;
        json[field_] = to_string(sortOrder_);
        return json;
    }

    const std::string &field() const
    {
        return field_;
    }

    const SortOrder &sortOrder() const
    {
        return sortOrder_;
    }

  private:
    std::string field_;
    SortOrder sortOrder_;
};

class SearchParam
{
  public:
    SearchParam(const std::string &index) : index_(index)
    {
    }

  public:
    std::string index() const
    {
        return index_;
    }

    Json::Value toJson() const
    {
        Json::Value json;
        if (query_)
        {
            json["query"] = query_->toJson();
        }
        if (sort_.size() > 0)
        {
            for (const auto &item : sort_)
            {
                json["sort"].append(item.toJson());
            }
        }
        if (from_)
        {
            json["from"] = *from_;
        }
        if (size_)
        {
            json["size"] = *size_;
        }
        if (agg_)
        {
            json["aggs"] = agg_->toJson();
        }
        return json;
    }

    SearchParam &query(QueryPtr query)
    {
        query_ = query;
        return *this;
    }

    SearchParam &sort(std::string field, SortOrder order = ASC)
    {
        for (auto iter = sort_.cbegin(); iter != sort_.cend(); ++iter)
        {
            if (iter->field() == field)
            {
                sort_.erase(iter);
                break;
            }
        }
        sort_.push_back(Sort(field, order));
        return *this;
    }

    SearchParam &from(int32_t from)
    {
        from_ = std::make_shared<int32_t>(from);
        return *this;
    }

    SearchParam &size(int32_t size)
    {
        size_ = std::make_shared<int32_t>(size);
        return *this;
    }

    SearchParam &agg(AggPtr agg)
    {
        agg_ = agg;
        return *this;
    }

  private:
    std::string index_;
    QueryPtr query_;
    std::vector<Sort> sort_;
    std::shared_ptr<int32_t> from_;
    std::shared_ptr<int32_t> size_;
    AggPtr agg_;
};

template <typename Tp>
    requires isDocumentType<Tp>
class Hit
{
  public:
    void setByJson(const Json::Value &json)
    {
        if (json.isMember("_index") && json["_index"].isString())
        {
            index_ = json["_index"].asString();
        }
        if (json.isMember("_type") && json["_type"].isString())
        {
            type_ = json["_type"].asString();
        }
        if (json.isMember("_id") && json["_id"].isString())
        {
            id_ = json["_id"].asString();
        }
        if (json.isMember("_score") && json["_score"].isNumeric())
        {
            score_ = json["_score"].asDouble();
        }
        if (json.isMember("_source") && json["_source"].isObject())
        {
            source_ = std::make_shared<Tp>();
            source_->setByJson(json["_source"]);
        }
    }
    const std::string &getIndex() const
    {
        return index_;
    }
    const std::string &getType() const
    {
        return type_;
    }
    const std::string &getId() const
    {
        return id_;
    }
    double getScore() const
    {
        return score_;
    }
    const Tp &getSource() const
    {
        return *source_;
    }

  private:
    std::string index_;
    std::string type_;
    std::string id_;
    double score_;
    std::shared_ptr<Tp> source_;
};

template <typename Tp>
    requires isDocumentType<Tp>
class SearchResponse
{
  public:
    void setByJson(const Json::Value &json)
    {
        if (json.isMember("took") && json["took"].isInt())
        {
            took_ = json["took"].asInt();
        }
        if (json.isMember("timed_out") && json["timed_out"].isBool())
        {
            timed_out_ = json["timed_out"].asBool();
        }
        if (json.isMember("_shards") && json["_shards"].isObject())
        {
            shards_ = std::make_shared<Shards>();
            shards_->setByJson(json["_shards"]);
        }
        if (json.isMember("hits") && json["hits"].isObject())
        {
            auto hits = json["hits"];
            if (hits.isMember("total") && hits["total"].isInt())
            {
                hits__total_ = hits["total"].asInt();
            }
            if (hits.isMember("max_score") && hits["max_score"].isNumeric())
            {
                hits__max_score_ = std::make_shared<double>();
                *hits__max_score_ = hits["max_score"].asDouble();
            }
            if (hits.isMember("hits") && hits["hits"].isArray())
            {
                for (const auto &item : hits["hits"])
                {
                    Hit<Tp> hit;
                    hit.setByJson(item);
                    hits_.push_back(hit);
                }
            }
        }
        if (json.isMember("aggregations"))
        {
            for (const auto &item : json["aggregations"].getMemberNames())
            {
                Json::Value temp;
                temp[item] = json["aggregations"][item];
                aggregations_[item] =
                    AggregationsResponse::newAggregationsResponse(temp);
            }
        }
    }

    auto getTook()
    {
        return took_;
    }
    auto getTimedOut()
    {
        return timed_out_;
    }
    auto getShards()
    {
        return shards_;
    }
    auto getHitsTotal()
    {
        return hits__total_;
    }
    auto getHitsMaxScore()
    {
        return hits__max_score_;
    }
    auto getHits()
    {
        return hits_;
    }
    auto getAggregationsResponse()
    {
        return aggregations_;
    }

  private:
    uint32_t took_;
    bool timed_out_;
    ShardsPtr shards_;
    uint32_t hits__total_;
    std::shared_ptr<double> hits__max_score_;
    std::vector<Hit<Tp>> hits_;
    std::unordered_map<std::string, std::shared_ptr<AggregationsResponse>>
        aggregations_;
};

template <typename Tp>
    requires isDocumentType<Tp>
using SearchResponsePtr = std::shared_ptr<SearchResponse<Tp>>;

class DocumentsClient
{
  public:
    DocumentsClient(HttpClientPtr httpClient) : httpClient_(httpClient)
    {
    }

  public:
    IndexResponsePtr index(const IndexParam &param, const Document &doc) const;
    void index(
        const IndexParam &param,
        const Document &doc,
        const std::function<void(const IndexResponsePtr &)> &resultCallback,
        const std::function<void(const ElasticSearchException &)>
            &exceptionCallback) const;

    DeleteResponsePtr deleteDocument(const DeleteParam &param) const;
    void deleteDocument(
        const DeleteParam &param,
        const std::function<void(const DeleteResponsePtr &)> &resultCallback,
        const std::function<void(const ElasticSearchException &)>
            &exceptionCallback) const;

    UpdateResponsePtr update(const UpdateParam &param,
                             const Document &doc) const;
    void update(
        const UpdateParam &param,
        const Document &doc,
        const std::function<void(const UpdateResponsePtr &)> &resultCallback,
        const std::function<void(const ElasticSearchException &)>
            &exceptionCallback) const;

    GetResponsePtr get(const GetParam &param) const;
    void get(const GetParam &param,
             const std::function<void(const GetResponsePtr &)> &resultCallback,
             const std::function<void(const ElasticSearchException &)>
                 &exceptionCallback) const;

    // search
    template <typename Tp>
        requires isDocumentType<Tp>
    SearchResponsePtr<Tp> search(const SearchParam &param) const
    {
        std::unique_ptr<std::promise<SearchResponsePtr<Tp>>> pro(
            new std::promise<SearchResponsePtr<Tp>>);
        auto f = pro->get_future();
        this->search<Tp>(
            param,
            [&pro](const SearchResponsePtr<Tp> &response) {
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
            });
        return f.get();
    }

    template <typename Tp>
        requires isDocumentType<Tp>
    void search(const SearchParam &param,
                const std::function<void(const SearchResponsePtr<Tp> &)>
                    &resultCallback,
                const std::function<void(const ElasticSearchException &)>
                    &exceptionCallback) const
    {
        std::string path = "/";
        path += param.index();
        path += "/_search";

        Json::Value requestBody = param.toJson();

        httpClient_->sendRequest(
            path,
            drogon::Get,
            [resultCallback = std::move(resultCallback),
             exceptionCallback = std::move(exceptionCallback)](
                const Json::Value &responseBody) {
                if (responseBody.isMember("error") &&
                    responseBody["error"].isObject())
                {
                    auto error = responseBody["error"];
                    auto type = error["type"].asString();
                    auto reason = error["reason"].asString();
                    std::string errorMessage = "ElasticSearchException [type=";
                    errorMessage += type;
                    errorMessage += ", reason=";
                    errorMessage += reason;
                    errorMessage += "]";
                    exceptionCallback(ElasticSearchException(errorMessage));
                }
                else
                {
                    SearchResponsePtr<Tp> s_result =
                        std::make_shared<SearchResponse<Tp>>();
                    s_result->setByJson(responseBody);
                    resultCallback(s_result);
                }
            },
            std::move(exceptionCallback),
            requestBody);
    }

  private:
    std::shared_ptr<HttpClient> httpClient_;
};

using DocumentsClientPtr = std::shared_ptr<DocumentsClient>;

};  // namespace tl::elasticsearch
