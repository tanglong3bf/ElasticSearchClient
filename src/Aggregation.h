#pragma once

#include <json/value.h>

namespace tl::elasticsearch
{

class AggregationParam
{
  public:
    virtual Json::Value toJson() const = 0;
};

using Agg = AggregationParam;
using AggPtr = std::shared_ptr<Agg>;

class TermsAgg : public Agg, public std::enable_shared_from_this<TermsAgg>
{
  private:
    TermsAgg()
    {
    }

  public:
    static auto newTermsAgg()
    {
        return std::shared_ptr<TermsAgg>(new TermsAgg());
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
        result["terms"]["field"] = field_;
        if (size_)
        {
            result["terms"]["size"] = *size_;
        }
        return result;
    }

  private:
    std::string field_;
    std::shared_ptr<int32_t> size_;
};

};  // namespace tl::elasticsearch
