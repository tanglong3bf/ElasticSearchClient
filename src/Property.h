/**
 *
 *  Property.h
 *
 */

#pragma once

#include "ElasticSearchException.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <trantor/utils/Logger.h>

namespace tl::elasticsearch
{

enum PropertyType
{
    NONE,
    TEXT,
    KEYWORD,
    LONG,
    INTEGER,
    SHORT,
    BYTE,
    DOUBLE,
    FLOAT,
    BOOLEAN,
    DATE,
};

inline std::string to_string(const PropertyType &propertyType)
{
    if (propertyType == NONE)
    {
        return "none";
    }
    if (propertyType == TEXT)
    {
        return "text";
    }
    if (propertyType == KEYWORD)
    {
        return "keyword";
    }
    if (propertyType == LONG)
    {
        return "long";
    }
    if (propertyType == INTEGER)
    {
        return "integer";
    }
    if (propertyType == SHORT)
    {
        return "short";
    }
    if (propertyType == BYTE)
    {
        return "byte";
    }
    if (propertyType == DOUBLE)
    {
        return "double";
    }
    if (propertyType == FLOAT)
    {
        return "float";
    }
    if (propertyType == BOOLEAN)
    {
        return "boolean";
    }
    if (propertyType == DATE)
    {
        return "date";
    }
    throw std::runtime_error("unkown type");
}

inline PropertyType string2PropertyType(const std::string &str)
{
    PropertyType result;
    if (str == "text")
    {
        result = TEXT;
    }
    else if (str == "keyword")
    {
        result = KEYWORD;
    }
    else if (str == "long")
    {
        result = LONG;
    }
    else if (str == "integer")
    {
        result = INTEGER;
    }
    else if (str == "short")
    {
        result = SHORT;
    }
    else if (str == "byte")
    {
        result = BYTE;
    }
    else if (str == "double")
    {
        result = DOUBLE;
    }
    else if (str == "float")
    {
        result = FLOAT;
    }
    else if (str == "boolean")
    {
        result = BOOLEAN;
    }
    else if (str == "date")
    {
        result = DATE;
    }
    else if (str == "none")
    {
        result = NONE;
    }
    else
    {
        std::string error_message = "error property type: ";
        error_message += str;
        throw tl::elasticsearch::ElasticSearchException(error_message);
    }
    return std::move(result);
}

class Property
{
    // friend class CreateIndexParam;
  public:
    Property(const std::string &property_name, bool index = false)
        : property_name_(property_name), type_(NONE), index_(index)
    {
    }

    Property(const std::string &property_name,
             PropertyType type,
             bool index = true)
        : property_name_(property_name), type_(type), index_(index)
    {
        if (type == TEXT)
        {
            analyzer_ = "standard";
        }
    }

    Property(const std::string &property_name,
             PropertyType type,
             const std::string &analyzer,
             bool index = true)
        : property_name_(property_name),
          type_(type),
          analyzer_(analyzer),
          index_(index)
    {
        if (type != TEXT)
        {
            LOG_WARN << "type of " << to_string(type)
                     << " not need analyzer but set.";
        }
    }

    Property &addSubProperty(const Property &subProperty)
    {
        if (type_ != NONE)
        {
            std::string error_message = "Property of ";
            error_message += to_string(type_);
            error_message += " CANNOT have children.";
            throw tl::elasticsearch::ElasticSearchException(error_message);
        }
        properties_.push_back(subProperty);
        return *this;
    }

  public:
    const std::string &getPropertyName() const
    {
        return property_name_;
    }

    const PropertyType &getType() const
    {
        return type_;
    }

    bool getIndex() const
    {
        return index_;
    }

    const std::string &getAnalyzer() const
    {
        return analyzer_;
    }

    const std::vector<Property> &getProperties() const
    {
        return properties_;
    }

  private:
    std::string property_name_;
    PropertyType type_ = NONE;
    bool index_;
    std::string analyzer_;
    std::vector<Property> properties_;
};

using PropertyPtr = std::shared_ptr<Property>;

};  // namespace tl::elasticsearch
