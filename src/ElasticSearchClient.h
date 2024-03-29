/**
 *
 *  ElasticSearchClient.h
 *
 */

#pragma once

#include "DocumentsClient.h"
#include "IndicesClient.h"
#include <drogon/HttpTypes.h>
#include <drogon/plugins/Plugin.h>
#include <memory>

namespace tl::elasticsearch
{

class ElasticSearchClient : public drogon::Plugin<ElasticSearchClient>
{
  public:
    /// This method must be called by drogon to initialize and start the plugin.
    /// It must be implemented by the user.
    void initAndStart(const Json::Value &config) override;

    /// This method must be called by drogon to shutdown the plugin.
    /// It must be implemented by the user.
    void shutdown() override;

  public:
    IndicesClientPtr indices() const;
    std::shared_ptr<HttpClient> httpClient() const;

  public:
    // operations of document
    IndexResponsePtr index(const IndexParam &param, const Document &doc) const
    {
        return this->documents_->index(param, doc);
    }

    void index(
        const IndexParam &param,
        const Document &doc,
        const std::function<void(const IndexResponsePtr &)> &resultCallback,
        const std::function<void(const ElasticSearchException &)>
            &exceptionCallback) const
    {
        this->documents_->index(param,
                                doc,
                                std::move(resultCallback),
                                std::move(exceptionCallback));
    }

    DeleteResponsePtr deleteDocument(const DeleteParam &param) const
    {
        return this->documents_->deleteDocument(param);
    }

    void deleteDocument(
        const DeleteParam &param,
        const std::function<void(const DeleteResponsePtr &)> &resultCallback,
        const std::function<void(const ElasticSearchException &)>
            &exceptionCallback) const
    {
        this->documents_->deleteDocument(param,
                                         std::move(resultCallback),
                                         std::move(exceptionCallback));
    }

    UpdateResponsePtr update(const UpdateParam &param,
                             const Document &doc) const
    {
        return this->documents_->update(param, doc);
    }

    void update(
        const UpdateParam &param,
        const Document &doc,
        const std::function<void(const UpdateResponsePtr &)> &resultCallback,
        const std::function<void(const ElasticSearchException &)>
            &exceptionCallback) const
    {
        this->documents_->update(param,
                                 doc,
                                 std::move(resultCallback),
                                 std::move(exceptionCallback));
    }

    GetResponsePtr get(const GetParam &param) const
    {
        return this->documents_->get(param);
    }

    void get(const GetParam &param,
             const std::function<void(const GetResponsePtr &)> &resultCallback,
             const std::function<void(const ElasticSearchException &)>
                 &exceptionCallback) const
    {
        this->documents_->get(param,
                              std::move(resultCallback),
                              std::move(exceptionCallback));
    }

    // operations of search
    template <typename Tp>
        requires isDocumentType<Tp>
    SearchResponsePtr<Tp> search(const SearchParam &param) const
    {
        return this->documents_->search<Tp>(param);
    }

  private:
    IndicesClientPtr indices_;
    std::shared_ptr<HttpClient> httpClient_;
    DocumentsClientPtr documents_;

  private:
    std::string host_;
    int16_t port_;
};

};  // namespace tl::elasticsearch
