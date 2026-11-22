// e001 — tests for the safe use of first_word.
//
// These tests pass under every preset. They do not catch the dangling view
// in demo_unsafe.cpp: a unit test only sees the code that it calls.
#include "first_word.hpp"

#include <gtest/gtest.h>

#include <string>

namespace {

TEST(FirstWord, ReturnsTextBeforeFirstSpace)
{
    EXPECT_EQ(e001::first_word("hello world"), "hello");
}

TEST(FirstWord, ReturnsAllTextWhenNoSpace)
{
    EXPECT_EQ(e001::first_word("hello"), "hello");
}

TEST(FirstWord, ReturnsEmptyViewForEmptyText)
{
    EXPECT_TRUE(e001::first_word("").empty());
}

TEST(FirstWord, ReturnsEmptyViewWhenTextStartsWithSpace)
{
    EXPECT_TRUE(e001::first_word(" hello").empty());
}

TEST(FirstWord, ViewPointsIntoTheInput)
{
    // The view borrows: it shares the address of the caller's characters.
    const std::string text = "borrowed characters";
    EXPECT_EQ(e001::first_word(text).data(), text.data());
}

TEST(FirstWord, SafeWhenCallerKeepsTheOwner)
{
    const std::string greeting = e001::make_greeting();  // owner lives on
    const std::string_view word = e001::first_word(greeting);
    EXPECT_EQ(word, "greetings-from-a-heap-allocated-string");
}

TEST(FirstWordCopy, ResultOutlivesTheInput)
{
    const std::string word = e001::first_word_copy(e001::make_greeting());
    EXPECT_EQ(word, "greetings-from-a-heap-allocated-string");
}

}  // namespace
