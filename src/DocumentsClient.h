#pragma once

#include "ElasticSearchException.h"
#include "HttpClient.h"
#include "Query.h"
#include <functional>
#include <json/value.h>
#include <memory>

namespace tl::elasticsearch {

class Document {
public:
    virtual Json::Value toJson() const = 0;
};

class Shards {
public:
    int getFailed() const { return failed_; }

    int getSuccessful() const { return successful_; }

    int getTotal() const { return total_; }

    void setByJson(const Json::Value &json);

private:
    int failed_;
    int successful_;
    int total_;
};

using ShardsPtr = std::shared_ptr<Shards>;

class IndexResponse {
public:
    std::string getId() const { return id_; }

    const std::string &getIndex() const { return index_; }

    int getPrimaryTerm() const { return primary_term_; }

    int getSeqNo() const { return seq_no_; }

    const std::string &getType() const { return type_; }

    ShardsPtr getShards() const { return shards_; }

    int getVersion() const { return version_; }

    const std::string &getResult() const { return result_; }

    void setByJson(const Json::Value &json);

private:
    std::string id_;
    std::string index_;
    int         primary_term_;
    int         seq_no_;
    std::string type_;
    ShardsPtr   shards_;
    int         version_;
    // TODO:
    std::string result_;
};

using IndexResponsePtr = std::shared_ptr<IndexResponse>;

class IndexParam {
    friend class DocumentsClient;

public:
    IndexParam(std::string index)
      : index_(index) {}

    void setId(std::string id) { id_ = id; }

private:
    std::string index_;
    std::string id_;
};

class DeleteResponse {
public:
    std::string getId() const { return id_; }

    const std::string &getIndex() const { return index_; }

    int getPrimaryTerm() const { return primary_term_; }

    int getSeqNo() const { return seq_no_; }

    const std::string &getType() const { return type_; }

    ShardsPtr getShards() const { return shards_; }

    int getVersion() const { return version_; }

    const std::string &getResult() const { return result_; }

    void setByJson(const Json::Value &json);

private:
    std::string id_;
    std::string index_;
    int         primary_term_;
    int         seq_no_;
    std::string type_;
    ShardsPtr   shards_;
    int         version_;
    // TODO:
    std::string result_;
};

using DeleteResponsePtr = std::shared_ptr<DeleteResponse>;

class DeleteParam {
    friend class DocumentsClient;

public:
    DeleteParam(std::string index)
      : index_(index) {}

    void setId(std::string id) { id_ = id; }

private:
    std::string index_;
    std::string id_;
};

class UpdateResponse {
public:
    std::string getId() const { return id_; }

    const std::string &getIndex() const { return index_; }

    int getPrimaryTerm() const { return primary_term_; }

    int getSeqNo() const { return seq_no_; }

    const std::string &getType() const { return type_; }

    ShardsPtr getShards() const { return shards_; }

    int getVersion() const { return version_; }

    const std::string &getResult() const { return result_; }

    void setByJson(const Json::Value &json);

private:
    std::string id_;
    std::string index_;
    int         primary_term_;
    int         seq_no_;
    std::string type_;
    ShardsPtr   shards_;
    int         version_;
    // TODO:
    std::string result_;
};

using UpdateResponsePtr = std::shared_ptr<UpdateResponse>;

class UpdateParam {
    friend class DocumentsClient;

public:
    UpdateParam(std::string index)
      : index_(index) {}

    void setId(std::string id) { id_ = id; }

private:
    std::string index_;
    std::string id_;
};

class GetResponse {
public:
    std::string getId() const { return id_; }

    const std::string &getIndex() const { return index_; }

    int getPrimaryTerm() const { return primary_term_; }

    int getSeqNo() const { return seq_no_; }

    Json::Value getSource() const { return source_; }

    const std::string &getType() const { return type_; }

    int getVersion() const { return version_; }

    const bool getFound() const { return found_; }

    void setByJson(const Json::Value &json);

private:
    std::string id_;
    std::string index_;
    int         primary_term_;
    int         seq_no_;
    Json::Value source_;
    std::string type_;
    int         version_;
    bool        found_;
};

using GetResponsePtr = std::shared_ptr<GetResponse>;

class GetParam {
    friend class DocumentsClient;

public:
    GetParam(std::string index)
      : index_(index) {}

    void setId(std::string id) { id_ = id; }

private:
    std::string index_;
    std::string id_;
};

class SearchParam {
public:
    SearchParam(const std::string &index, const QueryPtr query)
      : index_(index),
        query_(query) {}

public:
    std::string index() const { return index_; }

    QueryPtr query() const { return query_; }

private:
    std::string index_;
    QueryPtr    query_;
};

class SearchResponse {
public:
    void setByJson(const Json::Value &json) {}
};

using SearchResponsePtr = std::shared_ptr<SearchResponse>;

class DocumentsClient {
public:
    DocumentsClient(HttpClientPtr httpClient)
      : httpClient_(httpClient) {}

public:
    IndexResponsePtr index(const IndexParam &param, const Document &doc) const;
    void             index(
                  const IndexParam                                      &param,
                  const Document                                        &doc,
                  const std::function<void(IndexResponsePtr &)>        &&resultCallback,
                  const std::function<void(ElasticSearchException &&)> &&exceptionCallback
                ) const;

    DeleteResponsePtr deleteDocument(const DeleteParam &param) const;
    void              deleteDocument(
                   const DeleteParam                                     &param,
                   const std::function<void(DeleteResponsePtr &)>       &&resultCallback,
                   const std::function<void(ElasticSearchException &&)> &&exceptionCallback
                 ) const;

    UpdateResponsePtr
         update(const UpdateParam &param, const Document &doc) const;
    void update(
      const UpdateParam                                     &param,
      const Document                                        &doc,
      const std::function<void(UpdateResponsePtr &)>       &&resultCallback,
      const std::function<void(ElasticSearchException &&)> &&exceptionCallback
    ) const;

    GetResponsePtr get(const GetParam &param) const;
    void           get(
                const GetParam                                        &param,
                const std::function<void(GetResponsePtr &)>          &&resultCallback,
                const std::function<void(ElasticSearchException &&)> &&exceptionCallback
              ) const;

    // search
    SearchResponsePtr search(const SearchParam &param) const;
    void              search(
                   const SearchParam                                     &param,
                   const std::function<void(SearchResponsePtr &)>       &&resultCallback,
                   const std::function<void(ElasticSearchException &&)> &&exceptionCallback
                 ) const;

private:
    std::shared_ptr<HttpClient> httpClient_;
};

using DocumentsClientPtr = std::shared_ptr<DocumentsClient>;

}; // namespace tl::elasticsearch
