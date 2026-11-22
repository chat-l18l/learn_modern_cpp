// e001 — first_word: a small API that returns a std::string_view.
//
// The view does not own any characters. It points into the text that the
// caller passes in. The caller must keep that text alive for as long as the
// view is in use. The compiler does not check this contract for you.
#pragma once

#include <string>
#include <string_view>

// LEREN_LIFETIMEBOUND tells Clang that the return value borrows from this
// parameter. Clang can then warn when the caller passes a temporary.
// GCC does not know the attribute, so the macro is empty there.
#ifdef __has_cpp_attribute
#if __has_cpp_attribute(clang::lifetimebound)
#define LEREN_LIFETIMEBOUND [[clang::lifetimebound]]
#endif
#endif
#ifndef LEREN_LIFETIMEBOUND
#define LEREN_LIFETIMEBOUND
#endif

namespace e001 {

// Returns the part of text before the first space, or all of text when it
// has no space. The result points into text.
std::string_view first_word(std::string_view text);

// Same as first_word, but annotated so that Clang can see the borrow.
std::string_view first_word_annotated(std::string_view text LEREN_LIFETIMEBOUND);

// Safe alternative: returns an owning copy. The result does not depend on
// the lifetime of text.
std::string first_word_copy(std::string_view text);

// Returns a greeting long enough to live on the heap (longer than the
// small-string buffer of libstdc++, which holds 15 characters).
std::string make_greeting();

// Returns a greeting short enough for the small-string buffer. The
// characters then live inside the std::string object itself, on the stack.
std::string make_short_greeting();

}  // namespace e001
