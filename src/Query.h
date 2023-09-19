#pragma once

#include <forward_list>
#include <json/value.h>
#include <trantor/utils/Logger.h>
#include "ElasticSearchException.h"

namespace tl::elasticsearch
{

class Query
{
  public:
    virtual Json::Value toJson() const = 0;
};

using QueryPtr = std::shared_ptr<Query>;

class MatchAllQuery : public Query
{
  private:
    MatchAllQuery() = default;

  public:
    class Builder
    {
      public:
        QueryPtr operator()()
        {
            return std::shared_ptr<MatchAllQuery>(new MatchAllQuery());
        }
    };

    Json::Value toJson() const
    {
        Json::Value json;
        json["match_all"]["a"] = 0;
        json["match_all"].removeMember("a");
        return json;
    }
};

class MatchQuery : public Query
{
  private:
    MatchQuery(std::string field, std::string query)
        : field_(field), query_(query)
    {
    }

  public:
    class Builder
    {
      public:
        Builder &field(std::string field)
        {
            field_ = field;
            return *this;
        }
        Builder &query(std::string query)
        {
            query_ = query;
            return *this;
        }
        QueryPtr operator()()
        {
            return std::shared_ptr<MatchQuery>(new MatchQuery(field_, query_));
        }

      private:
        std::string field_;
        std::string query_;
    };

    Json::Value toJson() const
    {
        Json::Value json;
        json["match"][field_] = query_;
        return json;
    }

  private:
    std::string field_;
    std::string query_;
};

class MultiMatchQuery : public Query
{
  private:
    MultiMatchQuery(std::string query, std::vector<std::string> fields)
        : query_(query), fields_(fields)
    {
    }

  public:
    class Builder
    {
      public:
        Builder &fields(std::vector<std::string> fields)
        {
            fields_ = fields;
            return *this;
        }
        Builder &query(std::string query)
        {
            query_ = query;
            return *this;
        }
        QueryPtr operator()()
        {
            auto size = fields_.size();
            if (size == 0)
            {
                std::string errorMessage(
                    "MultiMatchQuery must support fields.");
                throw ElasticSearchException(errorMessage);
            }
            else if (size == 1)
            {
                LOG_WARN << "Only one field is provided. It is recommended to "
                            "use MatchQuery";
            }
            return std::shared_ptr<MultiMatchQuery>(
                new MultiMatchQuery(query_, fields_));
        }

      private:
        std::vector<std::string> fields_;
        std::string query_;
    };

    Json::Value toJson() const
    {
        Json::Value json;
        json["multi_match"]["query"] = query_;
        for (std::string field : fields_)
        {
            json["multi_match"]["fields"].append(field);
        }
        return json;
    }

  private:
    std::string query_;
    std::vector<std::string> fields_;
};

class TermQuery : public Query
{
  private:
    TermQuery(std::string field, std::string query)
        : field_(field), query_(query)
    {
    }

  public:
    class Builder
    {
      public:
        Builder &field(std::string field)
        {
            field_ = field;
            return *this;
        }
        Builder &query(std::string query)
        {
            query_ = query;
            return *this;
        }
        QueryPtr operator()()
        {
            return std::shared_ptr<TermQuery>(new TermQuery(field_, query_));
        }

      private:
        std::string field_;
        std::string query_;
    };

    Json::Value toJson() const
    {
        Json::Value json;
        json["term"][field_]["value"] = query_;
        return json;
    }

  private:
    std::string field_;
    std::string query_;
};

class RangeQuery : public Query
{
  private:
    RangeQuery(std::string field) : field_(field)
    {
    }
    RangeQuery &gt(double gt)
    {
        gt_ = std::make_shared<double>(gt);
        return *this;
    }
    RangeQuery &lt(double lt)
    {
        lt_ = std::make_shared<double>(lt);
        return *this;
    }
    RangeQuery &gte(double gte)
    {
        gte_ = std::make_shared<double>(gte);
        return *this;
    }
    RangeQuery &lte(double lte)
    {
        lte_ = std::make_shared<double>(lte);
        return *this;
    }

  public:
    class Builder
    {
      public:
        Builder &field(std::string field)
        {
            field_ = field;
            return *this;
        }
        Builder &gt(double gt)
        {
            gt_ = std::make_shared<double>(gt);
            return *this;
        }
        Builder &lt(double lt)
        {
            lt_ = std::make_shared<double>(lt);
            return *this;
        }
        Builder &gte(double gte)
        {
            gte_ = std::make_shared<double>(gte);
            return *this;
        }
        Builder &lte(double lte)
        {
            lte_ = std::make_shared<double>(lte);
            return *this;
        }
        auto operator()()
        {
            if (lt_ && lte_)
            {
                throw ElasticSearchException(
                    "Cannot set lt and lte at the same time.");
            }
            if (gt_ && gte_)
            {
                throw ElasticSearchException(
                    "Cannot set gt and gte at the same time.");
            }
            auto query = std::shared_ptr<RangeQuery>(new RangeQuery(field_));
            if (gte_)
            {
                if (lte_)
                {
                    if (*gte_ > *lte_)
                    {
                        throw ElasticSearchException(
                            "Gte cannot be greater than lte.");
                    }
                    query->lte(*lte_);
                }
                else if (lt_)
                {
                    if (*gte_ >= *lt_)
                    {
                        throw ElasticSearchException(
                            "Gte cannot be greater than or eaual to lt.");
                    }
                    query->lt(*lt_);
                }
                query->gte(*gte_);
            }
            else if (gt_)
            {
                if (lte_)
                {
                    if (*gt_ >= *lte_)
                    {
                        throw ElasticSearchException(
                            "Gt cannot be greater than or equal to lte.");
                    }
                    query->lte(*lte_);
                }
                else if (lt_)
                {
                    if (*gt_ >= *lt_)
                    {
                        throw ElasticSearchException(
                            "Gt cannot be greater than or equal to lt.");
                    }
                    query->lt(*lt_);
                }
                query->gt(*gt_);
            }
            else if (lte_)
            {
                query->lte(*lte_);
            }
            else if (lt_)
            {
                query->lt(*lt_);
            }
            else
            {
                throw ElasticSearchException(
                    "Please set at least one condition.");
            }
            return query;
        }

      private:
        std::string field_;
        std::shared_ptr<double> gt_;
        std::shared_ptr<double> lt_;
        std::shared_ptr<double> gte_;
        std::shared_ptr<double> lte_;
    };

    Json::Value toJson() const
    {
        Json::Value json;
        if (gt_ != nullptr)
        {
            json["range"][field_]["gt"] = *gt_;
        }
        if (lt_ != nullptr)
        {
            json["range"][field_]["lt"] = *lt_;
        }
        if (gte_ != nullptr)
        {
            json["range"][field_]["gte"] = *gte_;
        }
        if (lte_ != nullptr)
        {
            json["range"][field_]["lte"] = *lte_;
        }
        return json;
    }

  private:
    std::string field_;
    std::shared_ptr<double> gt_;
    std::shared_ptr<double> lt_;
    std::shared_ptr<double> gte_;
    std::shared_ptr<double> lte_;
};

#if 0  // Lack of test data, still need to be adjusted
class GeoPoint
{
  public:
    GeoPoint(double lat, double lon) : lat_(lat), lon_(lon)
    {
    }

    Json::Value toJson() const
    {
        Json::Value json;
        json["lat"] = lat_;
        json["lon"] = lon_;
        return json;
    }

  private:
    double lat_;
    double lon_;
};

class GeoBoundingBoxQuery : public Query
{
  private:
    GeoBoundingBoxQuery(const std::string &field,
                        std::shared_ptr<GeoPoint> topLeft,
                        std::shared_ptr<GeoPoint> bottomRight)
        : field_(field), topLeft_(topLeft), bottomRight_(bottomRight)
    {
    }

  public:
    class Builder
    {
      public:
        Builder &field(const std::string &field)
        {
            field_ = field;
            return *this;
        }
        Builder &topLeft(const GeoPoint &topLeft)
        {
            topLeft_ = std::make_shared<GeoPoint>(topLeft);
            return *this;
        }
        Builder &bottomRight(const GeoPoint &bottomRight)
        {
            bottomRight_ = std::make_shared<GeoPoint>(bottomRight);
            return *this;
        }
        QueryPtr operator()()
        {
            return std::shared_ptr<GeoBoundingBoxQuery>(
                new GeoBoundingBoxQuery(field_, topLeft_, bottomRight_));
        }

      private:
        std::string field_;
        std::shared_ptr<GeoPoint> topLeft_;
        std::shared_ptr<GeoPoint> bottomRight_;
    };

    Json::Value toJson() const
    {
        Json::Value json;
        json["geo_bounding_box"][field_]["top_left"] = topLeft_->toJson();
        json["geo_bounding_box"][field_]["bottom_right"] =
            bottomRight_->toJson();
        return json;
    }

  private:
    std::string field_;
    std::shared_ptr<GeoPoint> topLeft_;
    std::shared_ptr<GeoPoint> bottomRight_;
};

class GeoDistanceQuery : public Query
{
  private:
    GeoDistanceQuery(std::string field,
                     std::string distance,
                     std::shared_ptr<GeoPoint> pos)
        : field_(field), distance_(distance), pos_(pos)
    {
    }

  public:
    class Builder
    {
      public:
        Builder &field(std::string field)
        {
            field_ = field;
            return *this;
        }
        Builder &distance(const std::string &distance)
        {
            distance_ = distance;
            return *this;
        }
        Builder &pos(const GeoPoint &pos)
        {
            pos_ = std::make_shared<GeoPoint>(pos);
            return *this;
        }
        QueryPtr operator()()
        {
            return std::shared_ptr<GeoDistanceQuery>(
                new GeoDistanceQuery(field_, distance_, pos_));
        }

      private:
        std::string field_;
        std::string distance_;
        std::shared_ptr<GeoPoint> pos_;
    };

    Json::Value toJson() const
    {
        Json::Value json;
        json["geo_distance"]["distance"] = distance_;
        json["geo_distance"][field_] = pos_->toJson();
        return json;
    }

  private:
    std::string field_;
    std::string distance_;
    std::shared_ptr<GeoPoint> pos_;
};

#endif

// enum BoostMode {
//     multiply,
//     replace,
//     sum,
//     avg,
//     max,
//     min
// };
//
// class ScoreFunction {
// public:
//     virtual Json::Value toJson() = 0;
// };
//
// using ScoreFunctionPtr = std::shared_ptr<ScoreFunction>;
//
// class WeightFunction : ScoreFunction {
// private:
//     double weight_;
// };
//
// class FieldValueFactorFunction : ScoreFunction {
// private:
//     std::string field_;
//     double factor_;
//     std::string modifier_;
//     double missing_;
// };
//
// class RandomScoreFunction : ScoreFunction {
// private:
//     double seed_;
//     std::string field_;
// };
//
// class ScriptScoreFunction : ScoreFunction {
// public:
//     Json::Value toJson() {
//         Json::Value json;
//         for (const auto &param : params_) {
//             json["script"]["params"].append(param);
//         }
//         json["script"]["source"] = script_;
//         return json;
//     }
// private:
//     std::vector<std::string> params_;
//     std::string script_;
// };
//
// class FilterFunction {
// private:
//     QueryPtr filter_;
//     ScoreFunctionPtr scoreFunction_;
// };
//
// using FilterFunctionPtr = std::shared_ptr<FilterFunction>;
//
// class FunctionScoreQuery {
// public:
//     FunctionScoreQuery();
//
// private:
//     QueryPtr query_;
//     std::vector<FilterFunctionPtr> functions_;
//     BoostMode boostMode_;
// };

class BoolQuery : public Query
{
  private:
    BoolQuery() = default;
    BoolQuery &must(const QueryPtr &mustQuery)
    {
        must_.push_back(mustQuery);
        return *this;
    }

    BoolQuery &should(const QueryPtr &shouldQuery)
    {
        should_.push_back(shouldQuery);
        return *this;
    }

    BoolQuery &mustNot(const QueryPtr &mustNotQuery)
    {
        mustNot_.push_back(mustNotQuery);
        return *this;
    }

    BoolQuery &filter(const QueryPtr &filterQuery)
    {
        filter_.push_back(filterQuery);
        return *this;
    }

  public:
    class Builder
    {
      public:
        Builder &must(const QueryPtr &mustQuery)
        {
            must_.push_back(mustQuery);
            return *this;
        }

        Builder &should(const QueryPtr &shouldQuery)
        {
            should_.push_back(shouldQuery);
            return *this;
        }

        Builder &mustNot(const QueryPtr &mustNotQuery)
        {
            mustNot_.push_back(mustNotQuery);
            return *this;
        }

        Builder &filter(const QueryPtr &filterQuery)
        {
            filter_.push_back(filterQuery);
            return *this;
        }

        QueryPtr operator()()
        {
            auto query = std::shared_ptr<BoolQuery>(new BoolQuery());
            if (must_.size() > 0)
            {
                for (const auto &must : must_)
                {
                    query->must(must);
                }
            }
            if (should_.size() > 0)
            {
                for (const auto &should : should_)
                {
                    query->should(should);
                }
            }
            if (mustNot_.size() > 0)
            {
                for (const auto &mustNot : mustNot_)
                {
                    query->mustNot(mustNot);
                }
            }
            if (filter_.size() > 0)
            {
                for (const auto &filter : filter_)
                {
                    query->filter(filter);
                }
            }
            return query;
        }

      private:
        std::vector<QueryPtr> must_;
        std::vector<QueryPtr> should_;
        std::vector<QueryPtr> mustNot_;
        std::vector<QueryPtr> filter_;
    };

    Json::Value toJson() const
    {
        Json::Value json;
        if (must_.size() > 0)
        {
            Json::Value must;
            for (const auto &item : must_)
            {
                must.append(item->toJson());
            }
            json["bool"]["must"] = must;
        }
        if (should_.size() > 0)
        {
            Json::Value should;
            for (const auto &item : should_)
            {
                should.append(item->toJson());
            }
            json["bool"]["should"] = should;
        }
        if (mustNot_.size() > 0)
        {
            Json::Value mustNot;
            for (const auto &item : mustNot_)
            {
                mustNot.append(item->toJson());
            }
            json["bool"]["mustNot"] = mustNot;
        }
        if (filter_.size() > 0)
        {
            Json::Value filter;
            for (const auto &item : filter_)
            {
                filter.append(item->toJson());
            }
            json["bool"]["filter"] = filter;
        }
        return json;
    }

  private:
    std::vector<QueryPtr> must_;
    std::vector<QueryPtr> should_;
    std::vector<QueryPtr> mustNot_;
    std::vector<QueryPtr> filter_;
};

};  // namespace tl::elasticsearch
