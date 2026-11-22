// e001 — demo: use a view after its string is gone.
//
// THIS PROGRAM HAS UNDEFINED BEHAVIOUR ON PURPOSE. Do not fix it: it is the
// object of study. The safe forms are in first_word_test.cpp.
//
// Usage: e001_demo_unsafe [plain|annotated|short]
//   plain      long string (heap), no annotation
//   annotated  long string (heap), parameter marked lifetimebound
//   short      short string (small-string buffer on the stack), no annotation
//
// Exit code 0 means "the program printed the expected word". With undefined
// behaviour that says nothing about correctness.
#include "first_word.hpp"

#include <cstdio>
#include <string_view>

int main(int argc, char **argv)
{
    const std::string_view mode = argc > 1 ? argv[1] : "plain";

    std::string_view word;
    std::string_view expected = "greetings-from-a-heap-allocated-string";
    if (mode == "annotated") {
        // Clang can warn here: the temporary dies at the end of the line.
        word = e001::first_word_annotated(e001::make_greeting());
    } else if (mode == "short") {
        word = e001::first_word(e001::make_short_greeting());
        expected = "hi";
    } else {
        // No compiler warns here: nothing tells it that the view borrows.
        word = e001::first_word(e001::make_greeting());
    }

    // The temporary std::string is destroyed. The next lines read memory
    // that no longer belongs to any object.
    std::printf("first word: '%.*s'\n", static_cast<int>(word.size()), word.data());
    return word == expected ? 0 : 1;
}
