#pragma once

#include <json/value.h>
#include <string>
#include <unordered_map>

namespace tl::elasticsearch
{

class Aggregations;
using AggPtr = std::shared_ptr<Aggregations>;

class Aggregations
{
  public:
    virtual Json::Value toJson() const = 0;

  protected:
    std::string name_;
};

class BucketAggregations : public Aggregations
{
  protected:
    std::unordered_map<std::string, AggPtr> nestedAggregations_;
};

class TermsAggregations : public BucketAggregations,
                          public std::enable_shared_from_this<TermsAggregations>
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
        return result;
    }

  private:
    std::string field_;
    std::shared_ptr<int32_t> size_;
};

};  // namespace tl::elasticsearch
