#include "../../src/ElasticSearchException.h"
#include <gtest/gtest.h>

TEST(ElasticSearchExceptionTest, Test1) {
    const std::string message = "test";
    tl::elasticsearch::ElasticSearchException e(message);
    ASSERT_STREQ(e.what(), "test");
}
