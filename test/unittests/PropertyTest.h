#include "../../src/Property.h"
#include <gtest/gtest.h>

TEST(ToStringTest, Test1)
{
    using namespace tl::elasticsearch;
    EXPECT_STREQ("none", to_string(NONE).c_str());
    EXPECT_STREQ("text", to_string(TEXT).c_str());
    EXPECT_STREQ("keyword", to_string(KEYWORD).c_str());
    EXPECT_STREQ("long", to_string(LONG).c_str());
    EXPECT_STREQ("integer", to_string(INTEGER).c_str());
    EXPECT_STREQ("short", to_string(SHORT).c_str());
    EXPECT_STREQ("byte", to_string(BYTE).c_str());
    EXPECT_STREQ("double", to_string(DOUBLE).c_str());
    EXPECT_STREQ("float", to_string(FLOAT).c_str());
    EXPECT_STREQ("boolean", to_string(BOOLEAN).c_str());
    EXPECT_STREQ("date", to_string(DATE).c_str());
}

TEST(String2PropertyTypeTest, Test1)
{
    using namespace tl::elasticsearch;
    EXPECT_EQ(NONE, string_to_property_type("none"));
    EXPECT_EQ(TEXT, string_to_property_type("text"));
    EXPECT_EQ(KEYWORD, string_to_property_type("keyword"));
    EXPECT_EQ(LONG, string_to_property_type("long"));
    EXPECT_EQ(INTEGER, string_to_property_type("integer"));
    EXPECT_EQ(SHORT, string_to_property_type("short"));
    EXPECT_EQ(BYTE, string_to_property_type("byte"));
    EXPECT_EQ(DOUBLE, string_to_property_type("double"));
    EXPECT_EQ(FLOAT, string_to_property_type("float"));
    EXPECT_EQ(BOOLEAN, string_to_property_type("boolean"));
    EXPECT_EQ(DATE, string_to_property_type("date"));
    EXPECT_THROW(string_to_property_type("xxx"), ElasticSearchException);
}

TEST(PropertyTest, NoneType)
{
    using namespace tl::elasticsearch;
    {
        auto p = Property("p");
        EXPECT_EQ(NONE, p.getType());
        EXPECT_FALSE(p.getIndex());
        EXPECT_STREQ("", p.getAnalyzer().c_str());
        EXPECT_STREQ("p", p.getPropertyName().c_str());
        EXPECT_EQ(0, p.getProperties().size());
    }
    {
        auto p = Property("p", NONE, "ik_smart");  // LOG_WARN
        EXPECT_EQ(NONE, p.getType());
        EXPECT_TRUE(p.getIndex());
        EXPECT_STREQ("ik_smart", p.getAnalyzer().c_str());
        EXPECT_STREQ("p", p.getPropertyName().c_str());
        EXPECT_EQ(0, p.getProperties().size());
    }
}

TEST(PropertyTest, TextType)
{
    using namespace tl::elasticsearch;
    {
        // default analyzer
        auto p = Property("p", TEXT);
        EXPECT_EQ(TEXT, p.getType());
        EXPECT_TRUE(p.getIndex());
        EXPECT_STREQ("standard", p.getAnalyzer().c_str());
        EXPECT_STREQ("p", p.getPropertyName().c_str());
        EXPECT_EQ(0, p.getProperties().size());
    }
    {
        auto p = Property("p", TEXT, "ik_smart");
        EXPECT_EQ(TEXT, p.getType());
        EXPECT_TRUE(p.getIndex());
        EXPECT_STREQ("ik_smart", p.getAnalyzer().c_str());
        EXPECT_STREQ("p", p.getPropertyName().c_str());
        EXPECT_EQ(0, p.getProperties().size());
    }
}

TEST(PropertyTest, AddSubProperty)
{
    using namespace tl::elasticsearch;
    {
        auto p = Property("p", INTEGER);
        EXPECT_THROW(p.addSubProperty({"s", DOUBLE}), ElasticSearchException);
    }
    {
        auto p = Property("p", false);
        p.addSubProperty({"lastname", TEXT, "ik_smart"});
    }
}
