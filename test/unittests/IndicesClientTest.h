#include "../../src/IndicesClient.h"
#include <drogon/HttpTypes.h>
#include <gtest/gtest.h>

TEST(CreateIndexParamTest, Test1)
{
    using namespace tl::elasticsearch;
    CreateIndexParam param(2, 3);
    param.addProperty({"name", TEXT});
    auto json = param.toJson();
    EXPECT_EQ(2, json["settings"]["number_of_shards"].asInt());
    EXPECT_EQ(3, json["settings"]["number_of_replicas"].asInt());
    ASSERT_TRUE(json["mappings"]["_doc"]["properties"].isMember("name"));
    EXPECT_STREQ(
        "text",
        json["mappings"]["_doc"]["properties"]["name"]["type"].asCString());
    EXPECT_STREQ(
        "standard",
        json["mappings"]["_doc"]["properties"]["name"]["analyzer"].asCString());
};

TEST(CreateIndexTest, Test1)
{
    using namespace tl::elasticsearch;
    // create an index
    HttpClient client("http://192.168.85.143:9200");
    client.sendRequest("/c1_index_name", drogon::Put);

    // mainTest
    IndicesClient indicesClient(std::make_shared<HttpClient>(client));
    EXPECT_THROW(indicesClient.create("c1_index_name"), ElasticSearchException);

    // delete an index
    client.sendRequest("/c1_index_name", drogon::Delete);
}

TEST(CreateIndexTest, Test2)
{
    using namespace tl::elasticsearch;
    // delete an index
    HttpClient client("http://192.168.85.143:9200");
    client.sendRequest("/c2_index_name", drogon::Delete);

    // mainTest
    IndicesClient indicesClient(std::make_shared<HttpClient>(client));
    auto resp = indicesClient.create("c2_index_name");
    EXPECT_TRUE(resp->isAcknowledged());
    EXPECT_TRUE(resp->isShardsAcknowledged());
    EXPECT_STREQ("c2_index_name", resp->getIndex().c_str());

    // delete an index
    client.sendRequest("/c2_index_name", drogon::Delete);
}

TEST(CreateIndexTest, Test3)
{
    using namespace tl::elasticsearch;
    // delete an index
    HttpClient client("http://192.168.85.143:9200");
    client.sendRequest("/c3_index_name", drogon::Delete);

    // mainTest
    IndicesClient indicesClient(std::make_shared<HttpClient>(client));
    tl::elasticsearch::CreateIndexParam param;
    param.addProperty(Property("info", TEXT, "ik_smart"));
    param.addProperty(Property("email", KEYWORD, false));
    param.addProperty(
        Property("name")
            .addSubProperty(Property("firstName", TEXT, "ik_smart", false))
            .addSubProperty(Property("lastName", KEYWORD, true)));
    auto resp = indicesClient.create("c3_index_name", param);
    EXPECT_TRUE(resp->isAcknowledged());
    EXPECT_TRUE(resp->isShardsAcknowledged());
    EXPECT_STREQ("c3_index_name", resp->getIndex().c_str());

    // delete an index
    client.sendRequest("/c3_index_name", drogon::Delete);
}
