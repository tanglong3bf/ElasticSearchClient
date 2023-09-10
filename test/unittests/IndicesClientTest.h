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
    CreateIndexParam param;
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

TEST(GetIndexTest, Test1)
{
    using namespace tl::elasticsearch;
    // create an index
    HttpClient client("http://192.168.85.143:9200");
    client.sendRequest("/g1_index_name", drogon::Put);

    // mainTest
    IndicesClient indicesClient(std::make_shared<HttpClient>(client));
    auto resp = indicesClient.get("g1_index_name");
    EXPECT_EQ(0, resp->getAliases().size());
    EXPECT_EQ(0, resp->getProperties().size());
    auto settings = resp->getSettings();
    EXPECT_EQ(5, settings->getNumberOfShards());
    EXPECT_EQ(1, settings->getNumberOfReplicas());

    // delete an index
    client.sendRequest("/g1_index_name", drogon::Delete);
}

TEST(GetIndexTest, Test2)
{
    using namespace tl::elasticsearch;
    // delete an index
    HttpClient client("http://192.168.85.143:9200");
    client.sendRequest("/g2_index_name", drogon::Delete);

    // mainTest
    IndicesClient indicesClient(std::make_shared<HttpClient>(client));
    EXPECT_THROW(indicesClient.get("g2_index_name"), ElasticSearchException);
}

TEST(GetIndexTest, Test3)
{
    using namespace tl::elasticsearch;
    // create an index
    HttpClient client("http://192.168.85.143:9200");
    IndicesClient indicesClient(std::make_shared<HttpClient>(client));
    CreateIndexParam param;
    param.addProperty(Property("info", TEXT, "ik_smart"));
    param.addProperty(Property("email", KEYWORD, false));
    param.addProperty(
        Property("name")
            .addSubProperty(Property("firstName", TEXT, "ik_smart", false))
            .addSubProperty(Property("lastName", KEYWORD, true)));
    indicesClient.create("g3_index_name", param);

    // mainTest
    auto resp = indicesClient.get("g3_index_name");
    EXPECT_EQ(0, resp->getAliases().size());
    auto properties = resp->getProperties();
    EXPECT_EQ(3, properties.size());

    EXPECT_EQ("email", properties[0].getPropertyName());
    EXPECT_EQ(KEYWORD, properties[0].getType());
    EXPECT_FALSE(properties[0].getIndex());
    EXPECT_STREQ("", properties[0].getAnalyzer().c_str());
    EXPECT_EQ(0, properties[0].getProperties().size());

    EXPECT_EQ("info", properties[1].getPropertyName());
    EXPECT_EQ(TEXT, properties[1].getType());
    EXPECT_TRUE(properties[1].getIndex());
    EXPECT_STREQ("ik_smart", properties[1].getAnalyzer().c_str());
    EXPECT_EQ(0, properties[1].getProperties().size());

    EXPECT_EQ("name", properties[2].getPropertyName());
    EXPECT_EQ(NONE, properties[2].getType());
    EXPECT_EQ(2, properties[2].getProperties().size());

    auto settings = resp->getSettings();
    EXPECT_EQ(5, settings->getNumberOfShards());
    EXPECT_EQ(1, settings->getNumberOfReplicas());
    EXPECT_TRUE(settings->getCreationDate() <= trantor::Date::now());
    EXPECT_STREQ("g3_index_name", settings->getProvidedName().c_str());

    // delete an index
    client.sendRequest("/g3_index_name", drogon::Delete);
}

TEST(PutMappingTest, Test1)
{
    using namespace tl::elasticsearch;
    HttpClient client("http://192.168.85.143:9200");
    client.sendRequest("/p1_index_name", drogon::Delete);

    // mainTest
    IndicesClient indicesClient(std::make_shared<HttpClient>(client));
    PutMappingParam param;
    param.addProperty({"title", TEXT, "ik_smart"});
    EXPECT_THROW(indicesClient.putMapping("p1_index_name", param),
                 ElasticSearchException);
}

TEST(PutMappingTest, Test2)
{
    using namespace tl::elasticsearch;
    // create an index
    HttpClient client("http://192.168.85.143:9200");
    client.sendRequest("/p2_index_name", drogon::Put);

    // mainTest
    IndicesClient indicesClient(std::make_shared<HttpClient>(client));
    PutMappingParam param;
    param.addProperty({"title", TEXT, "ik_smart"});
    auto resp = indicesClient.putMapping("p2_index_name", param);

    EXPECT_TRUE(resp->isAcknowledged());

    // delete an index
    client.sendRequest("/p2_index_name", drogon::Delete);
}

TEST(PutMappingTest, Test3)
{
    using namespace tl::elasticsearch;
    // create an index
    HttpClient client("http://192.168.85.143:9200");
    client.sendRequest("/p3_index_name", drogon::Put);

    // mainTest
    IndicesClient indicesClient(std::make_shared<HttpClient>(client));
    PutMappingParam param;
    param
        .addProperty(Property("name")
                         .addSubProperty({"firstName", TEXT, "ik_smart"})
                         .addSubProperty({"lastName", KEYWORD, false}))
        .addProperty({"age", INTEGER, false});
    auto resp = indicesClient.putMapping("p3_index_name", param);

    EXPECT_TRUE(resp->isAcknowledged());

    // delete an index
    client.sendRequest("/p3_index_name", drogon::Delete);
}

TEST(DeleteIndexTest, Test1) {
    using namespace tl::elasticsearch;
    // delete an index
    HttpClient client("http://192.168.85.143:9200");
    client.sendRequest("/d1_index_name", drogon::Delete);

    // mainTest
    IndicesClient indicesClient(std::make_shared<HttpClient>(client));
    EXPECT_THROW(indicesClient.deleteIndex("d1_index_name"), ElasticSearchException);
}

TEST(DeleteIndexTest, Test2) {
    using namespace tl::elasticsearch;
    // delete an index
    HttpClient client("http://192.168.85.143:9200");
    client.sendRequest("/d2_index_name", drogon::Put);

    // mainTest
    IndicesClient indicesClient(std::make_shared<HttpClient>(client));
    auto resp = indicesClient.deleteIndex("d2_index_name");
    EXPECT_TRUE(resp->isAcknowledged());
    client.sendRequest("/d2_index_name", drogon::Delete);
}
