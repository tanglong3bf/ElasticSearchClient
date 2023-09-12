#include "../../src/DocumentsClient.h"
#include "../../src/IndicesClient.h"
#include <gtest/gtest.h>

struct Blog : public tl::elasticsearch::Document {
    virtual Json::Value toJson() const {
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
