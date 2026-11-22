// @ID@ — tests for the safe form.
#include "example.hpp"

#include <gtest/gtest.h>

namespace {

TEST(Example, ReturnsItsInput)
{
    EXPECT_EQ(@ID@::example(42), 42);
}

}  // namespace
