#include <chrono>
#include <drogon/HttpClient.h>
#include <gtest/gtest.h>
#include "../../src/DocumentsClient.h"
#include <fstream>
#include <thread>

class SearchTest : public testing::Test
{
  private:
    static std::string readTxt(std::string file)
    {
        std::ifstream infile;
        infile.open(file.data());
        assert(infile.is_open());

        std::string result;
        std::string s;
        while (getline(infile, s))
        {
            result += s + "\n";
        }
        infile.close();
        return result;
    }

  protected:
    static void SetUpTestCase()
    {
        using namespace tl::elasticsearch;
        using namespace std::chrono_literals;
        HttpClient httpClient("http://localhost:9200");
        httpClient.sendRequest("/ds_index_name", drogon::Put);
        auto req = drogon::HttpRequest::newHttpRequest();
        req->setMethod(drogon::Post);
        req->setPath("/ds_index_name/_doc/_bulk");
        req->setContentTypeCode(drogon::CT_APPLICATION_JSON);
        // Test data source:
        // https://github.com/elastic/elasticsearch/blob/v6.8.23/docs/src/test/resources/accounts.json
        req->setBody(readTxt("../unittests/testdata.json"));

        auto client =
            drogon::HttpClient::newHttpClient("http://localhost:9200");
        client->sendRequest(req, 5);
        // waiting for ES server
        std::this_thread::sleep_for(2s);
    }
    static void TearDownTestCase()
    {
        // using namespace tl::elasticsearch;
        // HttpClient httpClient("http://localhost:9200");
        // httpClient.sendRequest("/ds_index_name", drogon::Delete);
    }
};

class Account : public tl::elasticsearch::Document
{
  public:
    virtual Json::Value toJson() const override
    {
        return json_;
    }
    virtual void setByJson(const Json::Value &json) override
    {
        json_ = json;
    }

  private:
    Json::Value json_;
};

TEST_F(SearchTest, MatchAllTest)
{
    using namespace tl::elasticsearch;
    DocumentsClient dClient(
        std::make_shared<HttpClient>("http://localhost:9200"));

    // mainTest
    auto query = MatchAllQuery::newMatchAllQuery();
    SearchParam param("ds_index_name");
    param.query(MatchAllQuery::newMatchAllQuery());
    auto resp = dClient.search<Account>(param);
    ASSERT_FALSE(resp->getTimedOut());
    EXPECT_EQ(0, resp->getShards()->getFailed());
    EXPECT_EQ(1000, resp->getHitsTotal());
    EXPECT_EQ(10, resp->getHits().size());
    for (const auto &item : resp->getHits())
    {
        EXPECT_STREQ("ds_index_name", item.getIndex().c_str());
        EXPECT_NO_THROW(item.getSource());
    }
}

TEST_F(SearchTest, MatchTest)
{
    using namespace tl::elasticsearch;
    DocumentsClient dClient(
        std::make_shared<HttpClient>("http://localhost:9200"));

    // mainTest
    SearchParam param("ds_index_name");
    param.query(MatchQuery::newMatchQuery()->field("age")->query("28"));
    auto resp = dClient.search<Account>(param);
    ASSERT_FALSE(resp->getTimedOut());
    EXPECT_EQ(0, resp->getShards()->getFailed());
    EXPECT_EQ(51, resp->getHitsTotal());
    EXPECT_EQ(10, resp->getHits().size());
    for (const auto &item : resp->getHits())
    {
        EXPECT_STREQ("ds_index_name", item.getIndex().c_str());
        EXPECT_NO_THROW(item.getSource());
    }
}

TEST_F(SearchTest, MatchPhraseTest)
{
    using namespace tl::elasticsearch;
    DocumentsClient dClient(
        std::make_shared<HttpClient>("http://localhost:9200"));

    // mainTest
    SearchParam param("ds_index_name");
    param.query(
        MatchPhraseQuery::newMatchPhraseQuery()->field("address")->query(
            "mill lane"));
    auto resp = dClient.search<Account>(param);
    ASSERT_FALSE(resp->getTimedOut());
    EXPECT_EQ(0, resp->getShards()->getFailed());
    EXPECT_EQ(1, resp->getHitsTotal());
    EXPECT_EQ(1, resp->getHits().size());
    for (const auto &item : resp->getHits())
    {
        EXPECT_STREQ("ds_index_name", item.getIndex().c_str());
        EXPECT_NO_THROW(item.getSource());
    }
}

TEST_F(SearchTest, MultiMatchQueryTest)
{
    using namespace tl::elasticsearch;
    EXPECT_THROW(MultiMatchQuery::newMultiMatchQuery()->fields({})->toJson(),
                 ElasticSearchException);
    EXPECT_NO_THROW(
        MultiMatchQuery::newMultiMatchQuery()->fields({"aaa"})->toJson());
}

TEST_F(SearchTest, MultiMatchTest)
{
    using namespace tl::elasticsearch;
    DocumentsClient dClient(
        std::make_shared<HttpClient>("http://localhost:9200"));

    // mainTest
    SearchParam param("ds_index_name");
    param.query(MultiMatchQuery::newMultiMatchQuery()->query("River")->fields(
        {"firstname", "lastname", "address", "city"}));
    auto resp = dClient.search<Account>(param);
    ASSERT_FALSE(resp->getTimedOut());
    EXPECT_EQ(0, resp->getShards()->getFailed());
    EXPECT_EQ(1, resp->getHitsTotal());
    EXPECT_EQ(1, resp->getHits().size());
    for (const auto &item : resp->getHits())
    {
        EXPECT_STREQ("ds_index_name", item.getIndex().c_str());
        EXPECT_NO_THROW(item.getSource());
    }
}

TEST_F(SearchTest, TermTest)
{
    using namespace tl::elasticsearch;
    DocumentsClient dClient(
        std::make_shared<HttpClient>("http://localhost:9200"));

    SearchParam param("ds_index_name");
    param.query(TermQuery::newTermQuery()->field("address")->query("street"));
    auto resp = dClient.search<Account>(param);
    ASSERT_FALSE(resp->getTimedOut());
    EXPECT_EQ(0, resp->getShards()->getFailed());
    EXPECT_EQ(385, resp->getHitsTotal());
    EXPECT_EQ(10, resp->getHits().size());
    for (const auto &item : resp->getHits())
    {
        EXPECT_STREQ("ds_index_name", item.getIndex().c_str());
        EXPECT_NO_THROW(item.getSource());
    }
}

TEST_F(SearchTest, RangeParam)
{
    using namespace tl::elasticsearch;
    EXPECT_THROW(RangeQuery::newRangeQuery()->field("balance")->toJson(),
                 ElasticSearchException);
    EXPECT_THROW(RangeQuery::newRangeQuery()
                     ->field("balance")
                     ->gte(20000)
                     ->gt(10000)
                     ->toJson(),
                 ElasticSearchException);
    EXPECT_THROW(RangeQuery::newRangeQuery()
                     ->field("balance")
                     ->lte(20000)
                     ->lt(10000)
                     ->toJson(),
                 ElasticSearchException);
    EXPECT_THROW(RangeQuery::newRangeQuery()
                     ->field("balance")
                     ->gte(20000)
                     ->lte(10000)
                     ->toJson(),
                 ElasticSearchException);
    EXPECT_THROW(RangeQuery::newRangeQuery()
                     ->field("balance")
                     ->gte(20000)
                     ->lt(10000)
                     ->toJson(),
                 ElasticSearchException);
    EXPECT_THROW(RangeQuery::newRangeQuery()
                     ->field("balance")
                     ->gt(20000)
                     ->lte(10000)
                     ->toJson(),
                 ElasticSearchException);
    EXPECT_THROW(RangeQuery::newRangeQuery()
                     ->field("balance")
                     ->gt(20000)
                     ->lt(10000)
                     ->toJson(),
                 ElasticSearchException);
    EXPECT_NO_THROW(RangeQuery::newRangeQuery()
                        ->field("balance")
                        ->gte(10000)
                        ->lte(20000)
                        ->toJson());
    EXPECT_NO_THROW(RangeQuery::newRangeQuery()
                        ->field("balance")
                        ->gte(10000)
                        ->lt(20000)
                        ->toJson());
    EXPECT_NO_THROW(RangeQuery::newRangeQuery()
                        ->field("balance")
                        ->gt(10000)
                        ->lte(20000)
                        ->toJson());
    EXPECT_NO_THROW(RangeQuery::newRangeQuery()
                        ->field("balance")
                        ->gt(10000)
                        ->lt(20000)
                        ->toJson());
    EXPECT_NO_THROW(
        RangeQuery::newRangeQuery()->field("balance")->lte(10000)->toJson());
    EXPECT_NO_THROW(
        RangeQuery::newRangeQuery()->field("balance")->lt(10000)->toJson());
}

TEST_F(SearchTest, RangeTest)
{
    using namespace tl::elasticsearch;
    DocumentsClient dClient(
        std::make_shared<HttpClient>("http://localhost:9200"));

    // mainTest
    SearchParam param("ds_index_name");
    param.query(
        RangeQuery::newRangeQuery()->field("balance")->gte(10000)->lte(30000));
    auto resp = dClient.search<Account>(param);
    ASSERT_FALSE(resp->getTimedOut());
    EXPECT_EQ(0, resp->getShards()->getFailed());
    EXPECT_EQ(430, resp->getHitsTotal());
    EXPECT_EQ(10, resp->getHits().size());
    for (const auto &item : resp->getHits())
    {
        EXPECT_STREQ("ds_index_name", item.getIndex().c_str());
        EXPECT_NO_THROW(item.getSource());
    }
}

TEST_F(SearchTest, BoolTest)
{
    using namespace tl::elasticsearch;
    DocumentsClient dClient(
        std::make_shared<HttpClient>("http://localhost:9200"));

    // mainTest
    SearchParam param("ds_index_name");
    param.query(
        BoolQuery::newBoolQuery()
            ->must(RangeQuery::newRangeQuery()->field("age")->gt(20)->lt(40))
            ->must(RangeQuery::newRangeQuery()->field("balance")->lte(40000))
            ->must(BoolQuery::newBoolQuery()
                       ->should(MatchQuery::newMatchQuery()
                                    ->field("employer")
                                    ->query("Pyrami"))
                       ->should(MatchQuery::newMatchQuery()
                                    ->field("employer")
                                    ->query("Netagy")))
            ->filter(MatchQuery::newMatchQuery()->field("gender")->query("M")));
    auto resp = dClient.search<Account>(param);
    ASSERT_FALSE(resp->getTimedOut());
    EXPECT_EQ(0, resp->getShards()->getFailed());
    EXPECT_EQ(2, resp->getHitsTotal());
    EXPECT_EQ(2, resp->getHits().size());
    for (const auto &item : resp->getHits())
    {
        EXPECT_STREQ("ds_index_name", item.getIndex().c_str());
        EXPECT_NO_THROW(item.getSource());
    }
}

TEST_F(SearchTest, SortTest)
{
    using namespace tl::elasticsearch;
    DocumentsClient dClient(
        std::make_shared<HttpClient>("http://localhost:9200"));

    SearchParam param("ds_index_name");
    param.query(MatchAllQuery::newMatchAllQuery());
    param.sort("balance", DESC);
    auto resp = dClient.search<Account>(param);
    ASSERT_FALSE(resp->getTimedOut());
    EXPECT_EQ(0, resp->getShards()->getFailed());
    EXPECT_EQ(1000, resp->getHitsTotal());
    EXPECT_EQ(10, resp->getHits().size());
    auto hits = resp->getHits();
    EXPECT_EQ(49989, hits[0].getSource().toJson()["balance"].asInt());
    EXPECT_EQ(49795, hits[1].getSource().toJson()["balance"].asInt());
    EXPECT_EQ(49741, hits[2].getSource().toJson()["balance"].asInt());
    EXPECT_EQ(49671, hits[3].getSource().toJson()["balance"].asInt());
    EXPECT_EQ(49587, hits[4].getSource().toJson()["balance"].asInt());
    EXPECT_EQ(49568, hits[5].getSource().toJson()["balance"].asInt());
    EXPECT_EQ(49567, hits[6].getSource().toJson()["balance"].asInt());
    EXPECT_EQ(49433, hits[7].getSource().toJson()["balance"].asInt());
    EXPECT_EQ(49404, hits[8].getSource().toJson()["balance"].asInt());
    EXPECT_EQ(49355, hits[9].getSource().toJson()["balance"].asInt());
}

TEST_F(SearchTest, PaginateTest)
{
    using namespace tl::elasticsearch;
    DocumentsClient dClient(
        std::make_shared<HttpClient>("http://localhost:9200"));

    SearchParam param("ds_index_name");
    param.query(MatchAllQuery::newMatchAllQuery());
    param.sort("balance", DESC).from(10).size(10);
    auto resp = dClient.search<Account>(param);
    ASSERT_FALSE(resp->getTimedOut());
    EXPECT_EQ(0, resp->getShards()->getFailed());
    EXPECT_EQ(1000, resp->getHitsTotal());
    EXPECT_EQ(10, resp->getHits().size());
    auto hits = resp->getHits();
    EXPECT_EQ(49339, hits[0].getSource().toJson()["balance"].asInt());
    EXPECT_EQ(49334, hits[1].getSource().toJson()["balance"].asInt());
    EXPECT_EQ(49252, hits[2].getSource().toJson()["balance"].asInt());
    EXPECT_EQ(49222, hits[3].getSource().toJson()["balance"].asInt());
    EXPECT_EQ(49205, hits[4].getSource().toJson()["balance"].asInt());
    EXPECT_EQ(49159, hits[5].getSource().toJson()["balance"].asInt());
    EXPECT_EQ(49119, hits[6].getSource().toJson()["balance"].asInt());
    EXPECT_EQ(49000, hits[7].getSource().toJson()["balance"].asInt());
    EXPECT_EQ(48997, hits[8].getSource().toJson()["balance"].asInt());
    EXPECT_EQ(48974, hits[9].getSource().toJson()["balance"].asInt());
}

TEST_F(SearchTest, TermAggTest)
{
    using namespace tl::elasticsearch;
    DocumentsClient dClient(
        std::make_shared<HttpClient>("http://localhost:9200"));

    SearchParam param("ds_index_name");
    param.size(0).agg(TermsAggregations::newTermsAgg()
                          ->name("group_by_state")
                          ->field("state.keyword"));
    auto resp = dClient.search<Account>(param);
    auto aggResp = resp->getAggregationsResponse();
    ASSERT_EQ(1, aggResp.size());
    auto bucketResponse = std::dynamic_pointer_cast<BucketAggregationsResponse>(
        aggResp["group_by_state"]);
    EXPECT_STREQ("group_by_state", bucketResponse->name().c_str());
    EXPECT_EQ(770, bucketResponse->sumOtherDocCount());
    EXPECT_EQ(20, bucketResponse->docCountErrorUpperBound());
    EXPECT_EQ(27, bucketResponse->buckets()[0].docCount());
    EXPECT_STREQ("ID", bucketResponse->buckets()[0].key().c_str());
}

TEST_F(SearchTest, TermAggWithAvgAggTest)
{
    using namespace tl::elasticsearch;
    DocumentsClient dClient(
        std::make_shared<HttpClient>("http://localhost:9200"));

    SearchParam param("ds_index_name");
    param.size(0).agg(TermsAggregations::newTermsAgg()
                          ->name("group_by_state")
                          ->field("state.keyword")
                          ->addSubAggregations(AvgAggregations::newAvgAgg()
                                                   ->name("average_balance")
                                                   ->field("balance")));
    auto resp = dClient.search<Account>(param);
    auto aggResp = resp->getAggregationsResponse();
    ASSERT_EQ(1, aggResp.size());
    auto bucketResponse = std::dynamic_pointer_cast<BucketAggregationsResponse>(
        aggResp["group_by_state"]);
    EXPECT_STREQ("group_by_state", bucketResponse->name().c_str());
    EXPECT_EQ(770, bucketResponse->sumOtherDocCount());
    EXPECT_EQ(20, bucketResponse->docCountErrorUpperBound());
    EXPECT_EQ(27, bucketResponse->buckets()[0].docCount());
    EXPECT_STREQ("ID", bucketResponse->buckets()[0].key().c_str());
    auto subAgg = bucketResponse->buckets()[0]
                      .subAggregationsResponses()["average_balance"];
    auto avgAgg =
        std::dynamic_pointer_cast<MetricsAggregationsResponse>(subAgg);
    EXPECT_EQ(24368.777777777777, avgAgg->value());
}

TEST_F(SearchTest, TermAggWithAvgAggTestAndOrder)
{
    using namespace tl::elasticsearch;
    DocumentsClient dClient(
        std::make_shared<HttpClient>("http://localhost:9200"));

    SearchParam param("ds_index_name");
    param.size(0).agg(TermsAggregations::newTermsAgg()
                          ->name("group_by_state")
                          ->field("state.keyword")
                          ->order({"average_balance", DESC})
                          ->addSubAggregations(AvgAggregations::newAvgAgg()
                                                   ->name("average_balance")
                                                   ->field("balance")));
    auto resp = dClient.search<Account>(param);
    auto aggResp = resp->getAggregationsResponse();
    ASSERT_EQ(1, aggResp.size());
    auto bucketResponse = std::dynamic_pointer_cast<BucketAggregationsResponse>(
        aggResp["group_by_state"]);
    EXPECT_STREQ("group_by_state", bucketResponse->name().c_str());
    EXPECT_EQ(918, bucketResponse->sumOtherDocCount());
    EXPECT_EQ(-1, bucketResponse->docCountErrorUpperBound());
    EXPECT_EQ(6, bucketResponse->buckets()[0].docCount());
    EXPECT_STREQ("AL", bucketResponse->buckets()[0].key().c_str());
    auto subAgg = bucketResponse->buckets()[0]
                      .subAggregationsResponses()["average_balance"];
    auto avgAgg =
        std::dynamic_pointer_cast<MetricsAggregationsResponse>(subAgg);
    EXPECT_EQ(41418.166666666664, avgAgg->value());
}
