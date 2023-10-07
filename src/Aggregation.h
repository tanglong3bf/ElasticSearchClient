#pragma once

#include <iostream>
#include <json/value.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <trantor/utils/Logger.h>

#include "./ElasticSearchException.h"

namespace tl::elasticsearch
{

class Aggregations
{
  public:
    virtual Json::Value toJson() const = 0;

    std::string name()
    {
        return name_;
    }

  protected:
    std::string name_;
};

using AggPtr = std::shared_ptr<Aggregations>;

template <typename AggType>
class BucketAggregations : public Aggregations,
                           public std::enable_shared_from_this<AggType>
{
  public:
    virtual AggPtr addSubAggregations(AggPtr newSubAggregations)
    {
        subAggregations_[newSubAggregations->name()] = newSubAggregations;
        return std::enable_shared_from_this<AggType>::shared_from_this();
    }

  protected:
    Json::Value subAggregationsToJson() const
    {
        Json::Value json;
        for (const auto &[name, value] : subAggregations_)
        {
            json[name] = value->toJson()[name];
        }
        return json;
    }

  protected:
    std::unordered_map<std::string, AggPtr> subAggregations_;
};

class TermsAggregations : public BucketAggregations<TermsAggregations>
{
  private:
    TermsAggregations()
    {
    }

  public:
    static auto newTermsAgg()
    {
        return std::shared_ptr<TermsAggregations>(new TermsAggregations());
    }

    auto name(const std::string &name)
    {
        name_ = name;
        return shared_from_this();
    }

    auto field(const std::string &field)
    {
        this->field_ = field;
        return shared_from_this();
    }

    auto size(const int32_t &size)
    {
        this->size_ = std::make_shared<int32_t>(size);
        return shared_from_this();
    }

    virtual Json::Value toJson() const override
    {
        Json::Value result;
        result[name_]["terms"]["field"] = field_;
        if (size_)
        {
            result[name_]["terms"]["size"] = *size_;
        }
        if (subAggregations_.size() > 0)
        {
            result[name_]["aggs"] = subAggregationsToJson();
        }
        return result;
    }

  private:
    std::string field_;
    std::shared_ptr<int32_t> size_;
};

template <typename AggType>
class MetricsAggregations : public Aggregations,
                            public std::enable_shared_from_this<AggType>
{
};

class AvgAggregations : public MetricsAggregations<AvgAggregations>
{
  private:
    AvgAggregations()
    {
    }

  public:
    static auto newAvgAgg()
    {
        return std::shared_ptr<AvgAggregations>(new AvgAggregations());
    }
    auto name(const std::string &name)
    {
        name_ = name;
        return shared_from_this();
    }

    auto field(const std::string &field)
    {
        this->field_ = field;
        return shared_from_this();
    }

    virtual Json::Value toJson() const override
    {
        Json::Value result;
        result[name_]["avg"]["field"] = field_;
        return result;
    }

  private:
    std::string field_;
};

class AggregationsResponse
{
  public:
    static std::shared_ptr<AggregationsResponse> newAggregationsResponse(
        const Json::Value &json);
    std::string name() const
    {
        return name_;
    }

  protected:
    virtual void setByJson(const Json::Value &json) = 0;

  protected:
    std::string name_;
};

class Bucket
{
  public:
    void setByJson(const Json::Value &json)
    {
        for (const auto &name : json.getMemberNames())
        {
            if (name == "key")
            {
                key_ = json["key"].asString();
            }
            else if (name == "doc_count")
            {
                docCount_ = json["doc_count"].asInt();
            }
            else
            {
                Json::Value temp;
                temp[name] = json[name];
                subAggregationsResponses_[name] =
                    AggregationsResponse::newAggregationsResponse(temp);
            }
        }
    }
    std::string key() const
    {
        return key_;
    }
    std::size_t docCount() const
    {
        return docCount_;
    }
    auto subAggregationsResponses() const
    {
        return subAggregationsResponses_;
    }

  private:
    std::string key_;
    std::size_t docCount_;
    std::unordered_map<std::string, std::shared_ptr<AggregationsResponse>>
        subAggregationsResponses_;
};

class BucketAggregationsResponse : public AggregationsResponse
{
  public:
    std::size_t docCountErrorUpperBound() const
    {
        return docCountErrorUpperBound_;
    }
    std::size_t sumOtherDocCount() const
    {
        return sumOtherDocCount_;
    }
    const auto &buckets() const
    {
        return buckets_;
    }

  protected:
    void setByJson(const Json::Value &json) override
    {
        name_ = json.getMemberNames()[0];
        auto content = json[name_];
        if (content.isMember("doc_count_error_upper_bound"))
        {
            docCountErrorUpperBound_ =
                content["doc_count_error_upper_bound"].asInt();
        }
        if (content.isMember("sum_other_doc_count"))
        {
            sumOtherDocCount_ = content["sum_other_doc_count"].asInt();
        }
        for (const auto &item : content["buckets"])
        {
            Bucket bucket;
            bucket.setByJson(item);
            buckets_.push_back(bucket);
        }
    }

  private:
    std::size_t docCountErrorUpperBound_;
    std::size_t sumOtherDocCount_;
    std::vector<Bucket> buckets_;
};

class MetricsAggregationsResponse : public AggregationsResponse
{
  public:
    double value() const
    {
        return value_;
    }

  protected:
    void setByJson(const Json::Value &json) override
    {
        name_ = json.getMemberNames()[0];
        auto content = json[name_];
        value_ = content["value"].asDouble();
    }

  private:
    double value_;
};

inline std::shared_ptr<AggregationsResponse> AggregationsResponse::
    newAggregationsResponse(const Json::Value &json)
{
    auto names = json.getMemberNames();
    if (names.size() != 1)
    {
        throw ElasticSearchException(
            "AggregationsResponse only allows receiving a json object with "
            "one key-value pair");
    }
    std::shared_ptr<AggregationsResponse> result;
    auto content = json[names[0]];
    if (content.isMember("doc_count_error_upper_bound") &&
        content.isMember("sum_other_doc_count") && content.isMember("buckets"))
    {
        result = std::make_shared<BucketAggregationsResponse>();
    }
    else
    {
        result = std::make_shared<MetricsAggregationsResponse>();
    }
    result->setByJson(json);
    return result;
}

};  // namespace tl::elasticsearch
