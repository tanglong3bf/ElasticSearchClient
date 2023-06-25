#pragma once

#include <functional>
#include <json/value.h>
#include <memory>
#include "ElasticSearchException.h"
#include "HttpClient.h"

namespace tl::elasticsearch {

class Document {
public:
    virtual Json::Value toJson() const = 0;
};

class Shards {
public:
    int getFailed() const {
        return failed_;
    }
    int getSuccessful() const {
        return successful_;
    }
    int getTotal() const {
        return total_;
    }
    void setByJson(const Json::Value &json);
private:
    int failed_; 
    int successful_; 
    int total_; 
};
using ShardsPtr = std::shared_ptr<Shards>;

class IndexResponse {
public:
    int getId() const {
        return id_;
    }
    const std::string &getIndex() const {
        return index_;
    }
    int getPrimaryTerm() const {
        return primary_term_;
    }
    int getSeqNo() const {
        return seq_no_;
    }
    const std::string &getType() const {
        return type_;
    }
    ShardsPtr getShards() const {
        return shards_;
    }
    int getVersion() const {
        return version_;
    }
    const std::string &getResult() const {
        return result_;
    }
    void setByJson(const Json::Value &json);
private:
    int id_;
    std::string index_;
    int primary_term_;
    int seq_no_;
    std::string type_;
    ShardsPtr shards_;
    int version_;
    // TODO:
    std::string result_;
};
/*
{
        "_id" : "1",
        "_index" : "sbxd",
        "_primary_term" : 1,
        "_seq_no" : 0,
        "_shards" :
        {
                "failed" : 0,
                "successful" : 1,
                "total" : 2
        },
        "_type" : "_doc",
        "_version" : 1,
        "result" : "created"
}
*/
using IndexResponsePtr = std::shared_ptr<IndexResponse>;

class IndexParam {
public:
    IndexParam(std::string index)
        : index_(index) {}
public:
    std::string index_;
    std::string id_;
};

class DocumentsClient {
public:
    DocumentsClient(HttpClientPtr httpClient)
        :httpClient_(httpClient) {}
public:
    IndexResponsePtr index(const IndexParam &param, const Document &doc) const;
    void index(
        const IndexParam &param,
        const Document &doc,
        const std::function<void (IndexResponsePtr &)> &&resultCallback,
        const std::function<void (ElasticSearchException &&)> &&exceptionCallback
    ) const;
private:
    std::shared_ptr<HttpClient> httpClient_;
};
using DocumentsClientPtr = std::shared_ptr<DocumentsClient>;

};
