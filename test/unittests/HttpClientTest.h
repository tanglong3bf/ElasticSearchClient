#include "../../src/HttpClient.h"
#include <cstring>
#include <drogon/drogon.h>
#include <drogon/HttpTypes.h>
#include <gtest/gtest.h>

TEST(HttpClientTest, Test1)
{
    tl::elasticsearch::HttpClient client("http://localhost:9200");
    ASSERT_NO_THROW(client.sendRequest("/", drogon::Get));
}

TEST(HttpClientTest, Test2)
{
    tl::elasticsearch::HttpClient client("http://localhost:9200");
    auto resp = client.sendRequest("/", drogon::Post);
    ASSERT_EQ(resp["status"].as<int>(), 405);
}

TEST(HttpClientTest, Test3)
{
    tl::elasticsearch::HttpClient client("http://localhost:9201");
    ASSERT_THROW(client.sendRequest("/", drogon::Get),
                 tl::elasticsearch::ElasticSearchException);
}

TEST(HttpClientTest, Test4)
{
    tl::elasticsearch::HttpClient client("http://localhost:9200");
    std::vector<Json::Value> param;
    Json::Value temp;
    temp["create"]["_index"] = "test";
    temp["create"]["_type"] = "_doc";
    temp["create"]["_id"] = "3";
    param.push_back(temp);
    Json::Value temp2;
    temp2["field1"] = "value3";
    param.push_back(temp2);
    ASSERT_NO_THROW(client.sendRequest("/_bulk", drogon::Post, param));
}

TEST(HttpClientTest, Test5)
{
    tl::elasticsearch::HttpClient client("http://localhost:9201");
    ASSERT_THROW(client.sendRequest("/",
                                    drogon::Get,
                                    std::vector<Json::Value>()),
                 tl::elasticsearch::ElasticSearchException);
}
