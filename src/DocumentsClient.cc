#include "DocumentsClient.h"
#include "ElasticSearchException.h"
#include <future>

using namespace std;
using namespace tl::elasticsearch;

void Shards::setByJson(const Json::Value &json)
{
    if (json.isMember("failed"))
    {
        this->failed_ = json["failed"].asInt();
    }
    if (json.isMember("successful"))
    {
        this->successful_ = json["successful"].asInt();
    }
    if (json.isMember("total"))
    {
        this->total_ = json["total"].asInt();
    }
}

void IndexResponse::setByJson(const Json::Value &json)
{
    if (json.isMember("_id"))
    {
        id_ = json["_id"].asString();
    }
    if (json.isMember("_index"))
    {
        index_ = json["_index"].asString();
    }
    if (json.isMember("_primary_term"))
    {
        primary_term_ = json["_primary_term"].asInt();
    }
    if (json.isMember("_seq_no"))
    {
        seq_no_ = json["_seq_no"].asInt();
    }
    if (json.isMember("_shards"))
    {
        shards_ = make_shared<Shards>();
        shards_->setByJson(json["_shards"]);
    }
    if (json.isMember("_type"))
    {
        type_ = json["_type"].asString();
    }
    if (json.isMember("_version"))
    {
        version_ = json["_version"].asInt();
    }
    if (json.isMember("result"))
    {
        result_ = json["result"].asString();
    }
}

void DeleteResponse::setByJson(const Json::Value &json)
{
    if (json.isMember("_id"))
    {
        id_ = json["_id"].asString();
    }
    if (json.isMember("_index"))
    {
        index_ = json["_index"].asString();
    }
    if (json.isMember("_primary_term"))
    {
        primary_term_ = json["_primary_term"].asInt();
    }
    if (json.isMember("_seq_no"))
    {
        seq_no_ = json["_seq_no"].asInt();
    }
    if (json.isMember("_shards"))
    {
        shards_ = std::make_shared<Shards>();
        shards_->setByJson(json["_shards"]);
    }
    if (json.isMember("_type"))
    {
        type_ = json["_type"].asString();
    }
    if (json.isMember("_version"))
    {
        version_ = json["_version"].asInt();
    }
    if (json.isMember("result"))
    {
        result_ = json["result"].asString();
    }
}

void UpdateResponse::setByJson(const Json::Value &json)
{
    if (json.isMember("_id"))
    {
        id_ = json["_id"].asString();
    }
    if (json.isMember("_index"))
    {
        index_ = json["_index"].asString();
    }
    if (json.isMember("_primary_term"))
    {
        primary_term_ = json["_primary_term"].asInt();
    }
    if (json.isMember("_seq_no"))
    {
        seq_no_ = json["_seq_no"].asInt();
    }
    if (json.isMember("_shards"))
    {
        shards_ = std::make_shared<Shards>();
        shards_->setByJson(json["_shards"]);
    }
    if (json.isMember("_type"))
    {
        type_ = json["_type"].asString();
    }
    if (json.isMember("_version"))
    {
        version_ = json["_version"].asInt();
    }
    if (json.isMember("result"))
    {
        result_ = json["result"].asString();
    }
}

void GetResponse::setByJson(const Json::Value &json)
{
    if (json.isMember("_id"))
    {
        id_ = json["_id"].asString();
    }
    if (json.isMember("_index"))
    {
        index_ = json["_index"].asString();
    }
    if (json.isMember("_primary_term"))
    {
        primary_term_ = json["_primary_term"].asInt();
    }
    if (json.isMember("_seq_no"))
    {
        seq_no_ = json["_seq_no"].asInt();
    }
    if (json.isMember("_source"))
    {
        source_ = json["_source"];
    }
    if (json.isMember("_type"))
    {
        type_ = json["_type"].asString();
    }
    if (json.isMember("_version"))
    {
        version_ = json["_version"].asInt();
    }
    if (json.isMember("found"))
    {
        found_ = json["found"].asBool();
    }
}

IndexResponsePtr DocumentsClient::index(const IndexParam &param,
                                        const Document &doc) const
{
    unique_ptr<promise<IndexResponsePtr>> pro(new promise<IndexResponsePtr>);
    auto f = pro->get_future();
    this->index(
        param,
        doc,
        [&pro](const IndexResponsePtr &response) {
            try
            {
                pro->set_value(response);
            }
            catch (...)
            {
                pro->set_exception(current_exception());
            }
        },
        [&pro](const ElasticSearchException &err) {
            pro->set_exception(make_exception_ptr(err));
        });
    return f.get();
}

void DocumentsClient::index(
    const IndexParam &param,
    const Document &doc,
    const std::function<void(const IndexResponsePtr &)> &resultCallback,
    const std::function<void(const ElasticSearchException &)>
        &exceptionCallback) const
{
    std::string path = "/";
    path += param.index_;
    path += "/_doc/";
    path += param.id_;
    httpClient_->sendRequest(
        path,
        drogon::Post,
        [resultCallback = std::move(resultCallback),
         exceptionCallback =
             std::move(exceptionCallback)](const Json::Value &responseBody) {
            IndexResponsePtr i_result = make_shared<IndexResponse>();
            i_result->setByJson(responseBody);
            resultCallback(i_result);
        },
        std::move(exceptionCallback),
        doc.toJson());
}

DeleteResponsePtr DocumentsClient::deleteDocument(
    const DeleteParam &param) const
{
    unique_ptr<promise<DeleteResponsePtr>> pro(new promise<DeleteResponsePtr>);
    auto f = pro->get_future();
    this->deleteDocument(
        param,
        [&pro](const DeleteResponsePtr &response) {
            try
            {
                pro->set_value(response);
            }
            catch (...)
            {
                pro->set_exception(current_exception());
            }
        },
        [&pro](const ElasticSearchException &err) {
            pro->set_exception(make_exception_ptr(err));
        });
    return f.get();
}

void DocumentsClient::deleteDocument(
    const DeleteParam &param,
    const std::function<void(const DeleteResponsePtr &)> &resultCallback,
    const std::function<void(const ElasticSearchException &)>
        &exceptionCallback) const
{
    std::string path = "/";
    path += param.index_;
    path += "/_doc/";
    path += param.id_;
    httpClient_->sendRequest(
        path,
        drogon::Delete,
        [resultCallback = std::move(resultCallback),
         exceptionCallback =
             std::move(exceptionCallback)](const Json::Value &responseBody) {
            // index is not exist
            if (responseBody.isMember("error"))
            {
                auto error = responseBody["error"];
                auto type = error["type"].asString();
                auto reason = error["reason"].asString();
                string errorMessage = "ElasticSearchException [type=";
                errorMessage += type;
                errorMessage += ", reason=";
                errorMessage += reason;
                errorMessage += "]";
                exceptionCallback(ElasticSearchException(errorMessage));
            }
            else if (responseBody.isMember("result") &&
                     responseBody["result"].asString() == "not_found")
            {
                string errorMessage =
                    "ElasticSearchException [Delete document failed. Because "
                    "document is not_found.]";
                exceptionCallback(ElasticSearchException(errorMessage));
            }
            else
            {
                DeleteResponsePtr d_result = make_shared<DeleteResponse>();
                d_result->setByJson(responseBody);
                resultCallback(d_result);
            }
        },
        std::move(exceptionCallback));
}

UpdateResponsePtr DocumentsClient::update(const UpdateParam &param,
                                          const Document &doc) const
{
    unique_ptr<promise<UpdateResponsePtr>> pro(new promise<UpdateResponsePtr>);
    auto f = pro->get_future();
    this->update(
        param,
        doc,
        [&pro](const UpdateResponsePtr &response) {
            try
            {
                pro->set_value(response);
            }
            catch (...)
            {
                pro->set_exception(current_exception());
            }
        },
        [&pro](const ElasticSearchException &err) {
            pro->set_exception(make_exception_ptr(err));
        });
    return f.get();
}

void DocumentsClient::update(
    const UpdateParam &param,
    const Document &doc,
    const std::function<void(const UpdateResponsePtr &)> &resultCallback,
    const std::function<void(const ElasticSearchException &)>
        &exceptionCallback) const
{
    std::string path = "";
    path += param.index_;
    path += "/_doc/";
    path += param.id_;
    path += "/_update";

    Json::Value requestBody;
    requestBody["doc"] = doc.toJson();

    httpClient_->sendRequest(
        path,
        drogon::Post,
        [resultCallback = std::move(resultCallback),
         exceptionCallback =
             std::move(exceptionCallback)](const Json::Value &responseBody) {
            if (responseBody.isMember("error"))
            {
                auto error = responseBody.get("error", {});
                auto type = error.get("type", {}).asString();
                auto reason = error.get("reason", {}).asString();
                string errorMessage = "ElasticSearchException [type=";
                errorMessage += type;
                errorMessage += ", reason=";
                errorMessage += reason;
                errorMessage += "]";
                exceptionCallback(ElasticSearchException(errorMessage));
            }
            else
            {
                UpdateResponsePtr u_result = make_shared<UpdateResponse>();
                u_result->setByJson(responseBody);
                resultCallback(u_result);
            }
        },
        std::move(exceptionCallback),
        requestBody);
}

GetResponsePtr DocumentsClient::get(const GetParam &param) const
{
    unique_ptr<promise<GetResponsePtr>> pro(new promise<GetResponsePtr>);
    auto f = pro->get_future();
    this->get(
        param,
        [&pro](const GetResponsePtr &response) {
            try
            {
                pro->set_value(response);
            }
            catch (...)
            {
                pro->set_exception(current_exception());
            }
        },
        [&pro](const ElasticSearchException &err) {
            pro->set_exception(make_exception_ptr(err));
        });
    return f.get();
}

void DocumentsClient::get(
    const GetParam &param,
    const std::function<void(const GetResponsePtr &)> &resultCallback,
    const std::function<void(const ElasticSearchException &)>
        &exceptionCallback) const
{
    std::string path = "/";
    path += param.index_;
    path += "/_doc/";
    path += param.id_;
    httpClient_->sendRequest(
        path,
        drogon::Get,
        [resultCallback = std::move(resultCallback),
         exceptionCallback =
             std::move(exceptionCallback)](const Json::Value &responseBody) {
            if (responseBody.isMember("error"))
            {
                auto error = responseBody.get("error", {});
                auto type = error.get("type", {}).asString();
                auto reason = error.get("reason", {}).asString();
                string errorMessage = "ElasticSearchException [type=";
                errorMessage += type;
                errorMessage += ", reason=";
                errorMessage += reason;
                errorMessage += "]";
                exceptionCallback(ElasticSearchException(errorMessage));
            }
            else if (responseBody.isMember("found") &&
                     !responseBody["found"].asBool())
            {
                string errorMessage =
                    "ElasticSearchException [Get document failed. Because "
                    "document is not_found.]";
                exceptionCallback(ElasticSearchException(errorMessage));
            }
            else
            {
                GetResponsePtr d_result = make_shared<GetResponse>();
                d_result->setByJson(responseBody);
                resultCallback(d_result);
            }
        },
        std::move(exceptionCallback));
}

SearchResponsePtr DocumentsClient::search(const SearchParam &param) const
{
    unique_ptr<promise<SearchResponsePtr>> pro(new promise<SearchResponsePtr>);
    auto f = pro->get_future();
    this->search(
        param,
        [&pro](const SearchResponsePtr &response) {
            try
            {
                pro->set_value(response);
            }
            catch (...)
            {
                pro->set_exception(current_exception());
            }
        },
        [&pro](const ElasticSearchException &err) {
            pro->set_exception(make_exception_ptr(err));
        });
    return f.get();
}

void DocumentsClient::search(
    const SearchParam &param,
    const std::function<void(const SearchResponsePtr &)> &resultCallback,
    const std::function<void(const ElasticSearchException &)>
        &exceptionCallback) const
{
    std::string path = "/";
    path += param.index();
    path += "/_search";

    Json::Value requestBody;
    requestBody["query"] = param.query()->toJson();

    httpClient_->sendRequest(
        path,
        drogon::Get,
        [resultCallback = std::move(resultCallback),
         exceptionCallback =
             std::move(exceptionCallback)](const Json::Value &responseBody) {
            if (responseBody.isMember("result") &&
                responseBody["result"].asString() == "not_found")
            {
                string errorMessage =
                    "ElasticSearchException [Get document failed. Because "
                    "document is not_found.]";
                exceptionCallback(ElasticSearchException(errorMessage));
            }
            else
            {
                SearchResponsePtr s_result = make_shared<SearchResponse>();
                s_result->setByJson(responseBody);
                resultCallback(s_result);
            }
        },
        std::move(exceptionCallback),
        requestBody);
}
