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
        using namespace tl::elasticsearch;
        HttpClient httpClient("http://localhost:9200");
        httpClient.sendRequest("/ds_index_name", drogon::Delete);
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
    auto query = MatchAllQuery::Builder()();
    SearchParam param("ds_index_name", MatchAllQuery::Builder()());
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
    SearchParam param("ds_index_name",
                      MatchQuery::Builder().field("age").query("28")());
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

TEST_F(SearchTest, MultiMatchQueryTest)
{
    using namespace tl::elasticsearch;
    EXPECT_THROW(MultiMatchQuery::Builder().fields({})(),
                 ElasticSearchException);
    EXPECT_NO_THROW(MultiMatchQuery::Builder().fields({"aaa"})());
}

TEST_F(SearchTest, MultiMatchTest)
{
    using namespace tl::elasticsearch;
    DocumentsClient dClient(
        std::make_shared<HttpClient>("http://localhost:9200"));

    // mainTest
    SearchParam param("ds_index_name",
                      MultiMatchQuery::Builder().query("River").fields(
                          {"firstname", "lastname", "address", "city"})());
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

    SearchParam param("ds_index_name",
                      TermQuery::Builder().field("address").query("street")());
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
    EXPECT_THROW(RangeQuery::Builder().field("balance")(),
                 ElasticSearchException);
    EXPECT_THROW(RangeQuery::Builder().field("balance").gte(20000).gt(10000)(),
                 ElasticSearchException);
    EXPECT_THROW(RangeQuery::Builder().field("balance").lte(20000).lt(10000)(),
                 ElasticSearchException);
    EXPECT_THROW(RangeQuery::Builder().field("balance").gte(20000).lte(10000)(),
                 ElasticSearchException);
    EXPECT_THROW(RangeQuery::Builder().field("balance").gte(20000).lt(10000)(),
                 ElasticSearchException);
    EXPECT_THROW(RangeQuery::Builder().field("balance").gt(20000).lte(10000)(),
                 ElasticSearchException);
    EXPECT_THROW(RangeQuery::Builder().field("balance").gt(20000).lt(10000)(),
                 ElasticSearchException);
    EXPECT_NO_THROW(
        RangeQuery::Builder().field("balance").gte(10000).lte(20000)());
    EXPECT_NO_THROW(
        RangeQuery::Builder().field("balance").gte(10000).lt(20000)());
    EXPECT_NO_THROW(
        RangeQuery::Builder().field("balance").gt(10000).lte(20000)());
    EXPECT_NO_THROW(
        RangeQuery::Builder().field("balance").gt(10000).lt(20000)());
    EXPECT_NO_THROW(RangeQuery::Builder().field("balance").lte(10000)());
    EXPECT_NO_THROW(RangeQuery::Builder().field("balance").lt(10000)());
}

TEST_F(SearchTest, RangeTest)
{
    using namespace tl::elasticsearch;
    DocumentsClient dClient(
        std::make_shared<HttpClient>("http://localhost:9200"));

    // mainTest
    SearchParam param("ds_index_name",
                      RangeQuery::Builder().field("balance").gte(10000).lte(
                          30000)());
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
    SearchParam param(
        "ds_index_name",
        BoolQuery::Builder()
            .must(RangeQuery::Builder().field("age").gt(20).lt(40)())
            .must(RangeQuery::Builder().field("balance").lte(40000)())
            .must(BoolQuery::Builder()
                      .should(MatchQuery::Builder()
                                  .field("employer")
                                  .query("Pyrami")())
                      .should(MatchQuery::Builder()
                                  .field("employer")
                                  .query("Netagy")())())
            .filter(MatchQuery::Builder().field("gender").query("M")())());
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
