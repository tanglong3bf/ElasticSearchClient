#include "../../src/DocumentsClient.h"
#include "../../src/IndicesClient.h"
#include <gtest/gtest.h>

struct Blog : public tl::elasticsearch::Document
{
    virtual Json::Value toJson() const
    {
        Json::Value result;
        result["title"] = "title";
        result["summary"] = "summary";
        result["view"] = 233;
        return result;
    }
};

TEST(DocumentsClientTest, Index1)
{
    using namespace tl::elasticsearch;
    HttpClient httpClient("http://192.168.85.143:9200");
    DocumentsClient dClient(std::make_shared<HttpClient>(httpClient));

    // mainTest
    auto param = IndexParam("di1_index_name");
    param.setId("1");
    auto resp = dClient.index(param, Blog());
    EXPECT_STREQ("1", resp->getId().c_str());
    EXPECT_STREQ("di1_index_name", resp->getIndex().c_str());
    EXPECT_STREQ("_doc", resp->getType().c_str());
    // EXPECT_STREQ("created", resp->getResult().c_str());
    // EXPECT_EQ(0, resp->getShards()->getFailed());

    IndicesClient iClient(std::make_shared<HttpClient>(httpClient));
    iClient.deleteIndex("di1_index_name");
}

TEST(DocumentsClientTest, Index2)
{
    using namespace tl::elasticsearch;
    HttpClient httpClient("http://192.168.85.143:9200");
    DocumentsClient dClient(std::make_shared<HttpClient>(httpClient));

    // prepare
    IndicesClient iClient(std::make_shared<HttpClient>(httpClient));
    iClient.create("di2_index_name");

    // mainTest
    auto param = IndexParam("di2_index_name");
    param.setId("1");
    auto resp = dClient.index(param, Blog());
    EXPECT_STREQ("1", resp->getId().c_str());
    EXPECT_STREQ("di2_index_name", resp->getIndex().c_str());

    iClient.deleteIndex("di2_index_name");
}

TEST(DocumentsClientTest, DeleteDocument1)
{
    using namespace tl::elasticsearch;
    HttpClient httpClient("http://192.168.85.143:9200");
    DocumentsClient dClient(std::make_shared<HttpClient>(httpClient));
    // prepare
    httpClient.sendRequest("/dd1_index_name", drogon::Delete);

    DeleteParam param("dd1_index_name");
    param.setId("1");
    EXPECT_THROW(dClient.deleteDocument(param), ElasticSearchException);
}

TEST(DocumentsClientTest, DeleteDocument2)
{
    using namespace tl::elasticsearch;
    HttpClient httpClient("http://192.168.85.143:9200");
    DocumentsClient dClient(std::make_shared<HttpClient>(httpClient));
    // prepare
    httpClient.sendRequest("/dd2_index_name", drogon::Put);

    DeleteParam param("dd2_index_name");
    param.setId("1");
    EXPECT_THROW(dClient.deleteDocument(param), ElasticSearchException);

    httpClient.sendRequest("/dd2_index_name", drogon::Delete);
}

TEST(DocumentsClientTest, DeleteDocument3)
{
    using namespace tl::elasticsearch;
    HttpClient httpClient("http://192.168.85.143:9200");
    DocumentsClient dClient(std::make_shared<HttpClient>(httpClient));
    // prepare
    httpClient.sendRequest("/dd3_index_name", drogon::Put);
    Json::Value json;
    json["title"] = "title";
    httpClient.sendRequest("/dd3_index_name/_doc/1", drogon::Post, json);

    DeleteParam param("dd3_index_name");
    param.setId("1");
    auto resp = dClient.deleteDocument(param);
    EXPECT_STREQ("1", resp->getId().c_str());
    EXPECT_STREQ("dd3_index_name", resp->getIndex().c_str());
    EXPECT_STREQ("deleted", resp->getResult().c_str());
    EXPECT_STREQ("_doc", resp->getType().c_str());

    httpClient.sendRequest("/dd3_index_name", drogon::Delete);
}

TEST(DocumentsClientTest, Update1)
{
    using namespace tl::elasticsearch;
    HttpClient httpClient("http://192.168.85.143:9200");
    DocumentsClient dClient(std::make_shared<HttpClient>(httpClient));
    // prepare
    httpClient.sendRequest("/du1_index_name", drogon::Delete);

    UpdateParam param("du1_index_name");
    param.setId("1");
    Blog blog;
    EXPECT_THROW(dClient.update(param, blog), ElasticSearchException);

    httpClient.sendRequest("/du1_index_name", drogon::Delete);
}

TEST(DocumentsClientTest, Update2)
{
    using namespace tl::elasticsearch;
    HttpClient httpClient("http://192.168.85.143:9200");
    DocumentsClient dClient(std::make_shared<HttpClient>(httpClient));
    // prepare
    httpClient.sendRequest("/du2_index_name", drogon::Post);

    UpdateParam param("du2_index_name");
    param.setId("1");
    Blog blog;
    EXPECT_THROW(dClient.update(param, blog), ElasticSearchException);

    httpClient.sendRequest("/du2_index_name", drogon::Delete);
}

TEST(DocumentsClientTest, Update3)
{
    using namespace tl::elasticsearch;
    HttpClient httpClient("http://192.168.85.143:9200");
    DocumentsClient dClient(std::make_shared<HttpClient>(httpClient));
    // prepare
    httpClient.sendRequest("/du3_index_name", drogon::Post);
    Json::Value json;
    json["title"] = "title";
    httpClient.sendRequest("/du3_index_name/_doc/1", drogon::Post, json);

    UpdateParam param("du3_index_name");
    param.setId("1");
    Blog blog;
    auto resp = dClient.update(param, blog);
    EXPECT_STREQ("du3_index_name", resp->getIndex().c_str());
    EXPECT_STREQ("1", resp->getId().c_str());
    EXPECT_STREQ("_doc", resp->getType().c_str());
    EXPECT_STREQ("updated", resp->getResult().c_str());

    httpClient.sendRequest("/du3_index_name", drogon::Delete);
}

TEST(DocumentsClientTest, Get1)
{
    using namespace tl::elasticsearch;
    HttpClient httpClient("http://192.168.85.143:9200");
    DocumentsClient dClient(std::make_shared<HttpClient>(httpClient));
    // prepare
    httpClient.sendRequest("/dg1_index_name", drogon::Delete);

    GetParam param("dg1_index_name");
    param.setId("1");
    EXPECT_THROW(dClient.get(param), ElasticSearchException);
}

TEST(DocumentsClientTest, Get2)
{
    using namespace tl::elasticsearch;
    HttpClient httpClient("http://192.168.85.143:9200");
    DocumentsClient dClient(std::make_shared<HttpClient>(httpClient));
    // prepare
    httpClient.sendRequest("/dg2_index_name", drogon::Put);

    GetParam param("dg2_index_name");
    param.setId("1");
    EXPECT_THROW(dClient.get(param), ElasticSearchException);

    httpClient.sendRequest("/dg2_index_name", drogon::Delete);
}

TEST(DocumentsClientTest, Get3)
{
    using namespace tl::elasticsearch;
    HttpClient httpClient("http://192.168.85.143:9200");
    DocumentsClient dClient(std::make_shared<HttpClient>(httpClient));
    // prepare
    httpClient.sendRequest("/dg3_index_name", drogon::Put);
    Json::Value json;
    json["title"] = "title";
    httpClient.sendRequest("/dg3_index_name/_doc/1", drogon::Post, json);

    GetParam param("dg3_index_name");
    param.setId("1");
    auto resp = dClient.get(param);
    EXPECT_STREQ("dg3_index_name", resp->getIndex().c_str());
    EXPECT_STREQ("1", resp->getId().c_str());
    EXPECT_STREQ("_doc", resp->getType().c_str());
    EXPECT_TRUE(resp->getFound());
    auto source = resp->getSource();
    ASSERT_TRUE(source.isMember("title"));
    EXPECT_STREQ("title", source["title"].asCString());

    httpClient.sendRequest("/dg3_index_name", drogon::Delete);
}
