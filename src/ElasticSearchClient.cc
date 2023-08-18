/**
 *
 *  ElasticSearchClient.cc
 *
 */

#include "ElasticSearchClient.h"

#include "DocumentsClient.h"
#include "HttpClient.h"
#include "IndicesClient.h"
#include <memory>

using namespace std;

using namespace tl::elasticsearch;

void ElasticSearchClient::initAndStart(const Json::Value &config) {
    /// Initialize and start the plugin
    this->host_ = config.get("host", Json::Value("localhost")).asString();
    this->port_ = config.get("port", Json::Value(9200)).asUInt();

    string url("http://");
    url += this->host_;
    if (this->port_ != 80) {
        url += ":";
        url += std::to_string(this->port_);
    }

    this->httpClient_ = std::shared_ptr<HttpClient>(new HttpClient(url));
    this->indices_    = IndicesClientPtr(new IndicesClient(httpClient_));
    this->documents_  = DocumentsClientPtr(new DocumentsClient(httpClient_));
}

void ElasticSearchClient::shutdown() {
    /// Shutdown the plugin
}

IndicesClientPtr ElasticSearchClient::indices() const {
    return indices_;
}

std::shared_ptr<HttpClient> ElasticSearchClient::httpClient() const {
    LOG_DEBUG << httpClient_.get();
    return httpClient_;
}
