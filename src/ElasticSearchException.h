/**
 *
 * ElasticSearchException.h
 *
 */

#pragma once

#include <exception>
#include <string>

namespace tl::elasticsearch {
class ElasticSearchException : public std::exception {
public:
    const char *what() const noexcept override
    {
        return message_.data();
    }
    ElasticSearchException(const std::string &message)
        : message_(message)
    {}
    ElasticSearchException(std::string &message)
        : message_(std::move(message))
    {}
    ElasticSearchException() = delete;
private:
    std::string message_;
};

};
