#include "../../src/HttpClient.h"
#include <cstring>
#include <drogon/drogon.h>
#include <drogon/HttpTypes.h>
#include <gtest/gtest.h>

TEST(HttpClientTest, Test1)
{
    tl::elasticsearch::HttpClient client("http://192.168.85.143:9200");
    auto resp = client.sendRequest("/", drogon::Get);
    ASSERT_STREQ(resp["version"]["number"].asCString(), "6.8.10");
}

TEST(HttpClientTest, Test2)
{
    tl::elasticsearch::HttpClient client("http://192.168.85.143:9200");
    auto resp = client.sendRequest("/", drogon::Post);
    ASSERT_EQ(resp["status"].as<int>(), 405);
}

TEST(HttpClientTest, Test3)
{
    tl::elasticsearch::HttpClient client("http://192.168.85.143:9201");
    ASSERT_THROW(client.sendRequest("/", drogon::Get), tl::elasticsearch::ElasticSearchException);
}
