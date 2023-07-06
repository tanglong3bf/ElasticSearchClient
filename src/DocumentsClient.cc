#include <future>
#include "DocumentsClient.h"
#include "ElasticSearchException.h"

using namespace std;
using namespace tl::elasticsearch;

void Shards::setByJson(const Json::Value &json) {
    if (json.isMember("_failed")) {
        failed_ = json["_failed"].asInt();
    }
    if (json.isMember("_successful")) {
        successful_ = json["_successful"].asInt();
    }
    if (json.isMember("_total")) {
        total_ = json["_total"].asInt();
    }
}

void IndexResponse::setByJson(const Json::Value &json) {
    if (json.isMember("_id")) {
        id_ = json["_id"].asString();
    }
    if (json.isMember("_index")) {
        index_ = json["_index"].asString();
    }
    if (json.isMember("_primary_term")) {
        primary_term_ = json["_primary_term"].asInt();
    }
    if (json.isMember("_seq_no")) {
        seq_no_ = json["_seq_no"].asInt();
    }
    if (json.isMember("_shards")) {
        shards_->setByJson(json["_shards"]);
    }
    if (json.isMember("_type")) {
        type_ = json["_type"].asString();
    }
    if (json.isMember("_version")) {
        version_ = json["_version"].asInt();
    }
    if (json.isMember("result")) {
        result_ = json["result"].asString();
    }
}

void DeleteResponse::setByJson(const Json::Value &json) {
    if (json.isMember("_id")) {
        id_ = json["_id"].asString();
    }
    if (json.isMember("_index")) {
        index_ = json["_index"].asString();
    }
    if (json.isMember("_primary_term")) {
        primary_term_ = json["_primary_term"].asInt();
    }
    if (json.isMember("_seq_no")) {
        seq_no_ = json["_seq_no"].asInt();
    }
    if (json.isMember("_shards")) {
        shards_->setByJson(json["_shards"]);
    }
    if (json.isMember("_type")) {
        type_ = json["_type"].asString();
    }
    if (json.isMember("_version")) {
        version_ = json["_version"].asInt();
    }
    if (json.isMember("result")) {
        result_ = json["result"].asString();
    }
}

void UpdateResponse::setByJson(const Json::Value &json) {
    if (json.isMember("_id")) {
        id_ = json["_id"].asString();
    }
    if (json.isMember("_index")) {
        index_ = json["_index"].asString();
    }
    if (json.isMember("_primary_term")) {
        primary_term_ = json["_primary_term"].asInt();
    }
    if (json.isMember("_seq_no")) {
        seq_no_ = json["_seq_no"].asInt();
    }
    if (json.isMember("_shards")) {
        shards_->setByJson(json["_shards"]);
    }
    if (json.isMember("_type")) {
        type_ = json["_type"].asString();
    }
    if (json.isMember("_version")) {
        version_ = json["_version"].asInt();
    }
    if (json.isMember("result")) {
        result_ = json["result"].asString();
    }
}

IndexResponsePtr DocumentsClient::index(const IndexParam &param, const Document &doc) const {
    unique_ptr<promise<IndexResponsePtr>> pro(new promise<IndexResponsePtr>);
    auto f = pro->get_future();
    this->index(param, doc, [&pro] (IndexResponsePtr &response) {
        try {
            pro->set_value(response);
        }
        catch (...) {
            pro->set_exception(current_exception());
        }
    }, [&pro] (ElasticSearchException &&err) {
        pro->set_exception(make_exception_ptr(err));
    });
    return f.get();
}

void DocumentsClient::index(
    const IndexParam &param,
    const Document &doc,
    const std::function<void (IndexResponsePtr &)> &&resultCallback,
    const std::function<void (ElasticSearchException &&)> &&exceptionCallback
) const {
    std::string path = "/";
    path += param.index_;
    path += "/_doc/";
    path += param.id_;
    httpClient_->sendRequest(path, drogon::Post, [
        resultCallback = move(resultCallback),
        exceptionCallback = move(exceptionCallback)
    ](Json::Value &responseBody) {
        IndexResponsePtr i_result = make_shared<IndexResponse>();
        i_result->setByJson(responseBody);
        resultCallback(i_result);
    }, move(exceptionCallback) , doc.toJson());
}

DeleteResponsePtr DocumentsClient::deleteDocument(const DeleteParam &param) const {
    unique_ptr<promise<DeleteResponsePtr>> pro(new promise<DeleteResponsePtr>);
    auto f = pro->get_future();
    this->deleteDocument(param, [&pro] (DeleteResponsePtr &response) {
        try {
            pro->set_value(response);
        }
        catch (...) {
            pro->set_exception(current_exception());
        }
    }, [&pro] (ElasticSearchException &&err) {
        pro->set_exception(make_exception_ptr(err));
    });
    return f.get();
}

void DocumentsClient::deleteDocument(
    const DeleteParam &param,
    const std::function<void (DeleteResponsePtr &)> &&resultCallback,
    const std::function<void (ElasticSearchException &&)> &&exceptionCallback
) const {
    std::string path = "/";
    path += param.index_;
    path += "/_doc/";
    path += param.id_;
    httpClient_->sendRequest(path, drogon::Delete, [
        resultCallback = move(resultCallback),
        exceptionCallback = move(exceptionCallback)
    ](Json::Value &responseBody) {
        if (responseBody.isMember("result") && responseBody["result"].asString() == "not_found") {
            string errorMessage = "ElasticSearchException [Delete document failed. Because document is not_found.]";
            exceptionCallback(ElasticSearchException(errorMessage));
        } else {
            DeleteResponsePtr d_result = make_shared<DeleteResponse>();
            d_result->setByJson(responseBody);
            resultCallback(d_result);
        }
    }, move(exceptionCallback));
}

UpdateResponsePtr DocumentsClient::update(const UpdateParam &param, const Document &doc) const {
    unique_ptr<promise<UpdateResponsePtr>> pro(new promise<UpdateResponsePtr>);
    auto f = pro->get_future();
    this->update(param, doc, [&pro] (UpdateResponsePtr &response) {
        try {
            pro->set_value(response);
        }
        catch (...) {
            pro->set_exception(current_exception());
        }
    }, [&pro] (ElasticSearchException &&err) {
        pro->set_exception(make_exception_ptr(err));
    });
    return f.get();
}

void DocumentsClient::update(
    const UpdateParam &param,
    const Document &doc,
    const std::function<void (UpdateResponsePtr &)> &&resultCallback,
    const std::function<void (ElasticSearchException &&)> &&exceptionCallback
) const {
    std::string path = "";
    path += param.index_;
    path += "/_doc/";
    path += param.id_;
    path += "/_update";

    Json::Value requestBody;
    requestBody["doc"] = doc.toJson();

    httpClient_->sendRequest(path, drogon::Post, [
        resultCallback = move(resultCallback),
        exceptionCallback = move(exceptionCallback)
    ](Json::Value &responseBody) {
        if (responseBody.isMember("error")) {
            auto error = responseBody.get("error", {});
            auto type = error.get("type", {}).asString();
            auto reason = error.get("reason", {}).asString();
            string errorMessage = "ElasticSearchException [type=";
            errorMessage += type;
            errorMessage += ", reason=";
            errorMessage += reason;
            errorMessage += "]";
            exceptionCallback(ElasticSearchException(errorMessage));
        } else {
            UpdateResponsePtr u_result = make_shared<UpdateResponse>();
            u_result->setByJson(responseBody);
            resultCallback(u_result);
        }
    }, move(exceptionCallback), requestBody);
}
