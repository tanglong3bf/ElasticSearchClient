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
    ASSERT_THROW(client.sendRequest("/", drogon::Get), tl::elasticsearch::ElasticSearchException);
}
