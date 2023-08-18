#pragma once

#include <forward_list>
#include <json/value.h>
#include <stdexcept>

class Query {
public:
    virtual Json::Value toJson() const = 0;
};

using QueryPtr = std::shared_ptr<Query>;

class MatchAllQuery : public Query {
public:
    Json::Value toJson() const {
        Json::Value json;
        json["match_all"]["a"] = 0;
        json["match_all"].removeMember("a");
        return json;
    }
};

class MatchQuery : public Query {
public:
    MatchQuery(std::string field, std::string query)
      : field_(field),
        query_(query) {}

    Json::Value toJson() const {
        Json::Value json;
        json["match"][field_] = query_;
        return json;
    }

private:
    std::string field_;
    std::string query_;
};

class MultiMatchQuery : public Query {
public:
    MultiMatchQuery(std::string query, std::vector<std::string> fields)
      : query_(query),
        fields_(fields) {}

    Json::Value toJson() const {
        Json::Value json;
        json["multi_match"]["query"] = query_;
        for (const auto &field: fields_) {
            json["multi_match"]["fields"].append(field);
        }
        return json;
    }

private:
    std::string              query_;
    std::vector<std::string> fields_;
};

class TermQuery : public Query {
public:
    TermQuery(std::string field, std::string query)
      : field_(field),
        query_(query) {}

    Json::Value toJson() const {
        Json::Value json;
        json["term"][field_]["value"] = query_;
        return json;
    }

private:
    std::string field_;
    std::string query_;
};

class RangeQuery : public Query {
public:
    RangeQuery(std::string field, double gte, double lte)
      : field_(field),
        gte_(gte),
        lte_(lte) {}

    Json::Value toJson() const {
        if (gte_ > lte_) {
            throw std::runtime_error("gte_ is greater than lte_");
        }
        Json::Value json;
        json["range"][field_]["gte"] = gte_;
        json["range"][field_]["lte"] = lte_;
        return json;
    }

private:
    std::string field_;
    double      gte_;
    double      lte_;
};

class GeoPoint {
public:
    GeoPoint(double lat, double lon)
      : lat_(lat),
        lon_(lon) {}

    Json::Value toJson() const {
        Json::Value json;
        json["lat"] = lat_;
        json["lon"] = lon_;
        return json;
    }

private:
    double lat_;
    double lon_;
};

class GeoBoundingBoxQuery : public Query {
public:
    GeoBoundingBoxQuery(GeoPoint topLeft, GeoPoint bottomRight)
      : topLeft_(topLeft),
        bottomRight_(bottomRight) {}

    Json::Value toJson() const {
        Json::Value json;
        json["geo_bounding_box"][field_]["top_left"] = topLeft_.toJson();
        json["geo_bounding_box"][field_]["bottom_right"] =
          bottomRight_.toJson();
        return json;
    }

private:
    std::string field_;
    GeoPoint    topLeft_;
    GeoPoint    bottomRight_;
};

class GeoDistanceQuery : public Query {
public:
    GeoDistanceQuery(std::string field, std::string distance, GeoPoint pos)
      : field_(field),
        distance_(distance),
        pos_(pos) {}

    Json::Value toJson() const {
        Json::Value json;
        json["geo_distance"]["distance"] = distance_;
        json["geo_distance"][field_]     = pos_.toJson();
        return json;
    }

private:
    std::string field_;
    std::string distance_;
    GeoPoint    pos_;
};

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

class BoolQuery : Query {
public:
    BoolQuery must(QueryPtr &mustQuery) {
        must_.push_back(mustQuery);
        return *this;
    }

    BoolQuery should(QueryPtr &shouldQuery) {
        should_.push_back(shouldQuery);
        return *this;
    }

    BoolQuery mustNot(QueryPtr &mustNotQuery) {
        mustNot_.push_back(mustNotQuery);
        return *this;
    }

    BoolQuery filter(QueryPtr &filterQuery) {
        filter_.push_back(filterQuery);
        return *this;
    }

    Json::Value toJson() const {
        Json::Value json;
        if (must_.size() > 0) {
            Json::Value must;
            for (const auto &item: must_) {
                must.append(item->toJson());
            }
            json["query"]["bool"]["must"] = must;
        }
        if (should_.size() > 0) {
            Json::Value should;
            for (const auto &item: should_) {
                should.append(item->toJson());
            }
            json["query"]["bool"]["should"] = should;
        }
        if (mustNot_.size() > 0) {
            Json::Value mustNot;
            for (const auto &item: mustNot_) {
                mustNot.append(item->toJson());
            }
            json["query"]["bool"]["mustNot"] = mustNot;
        }
        if (filter_.size() > 0) {
            Json::Value filter;
            for (const auto &item: filter_) {
                filter.append(item->toJson());
            }
            json["query"]["bool"]["filter"] = filter;
        }
        return json;
    }

private:
    std::vector<QueryPtr> must_;
    std::vector<QueryPtr> should_;
    std::vector<QueryPtr> mustNot_;
    std::vector<QueryPtr> filter_;
};
