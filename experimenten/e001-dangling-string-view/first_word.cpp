#include "first_word.hpp"

namespace e001 {

std::string_view first_word(std::string_view text)
{
    const auto space = text.find(' ');
    return text.substr(0, space);  // npos means "to the end"
}

std::string_view first_word_annotated(std::string_view text LEREN_LIFETIMEBOUND)
{
    return first_word(text);
}

std::string first_word_copy(std::string_view text)
{
    return std::string{first_word(text)};
}

// The experiment needs an owning std::string that frees its memory, so the
// return type stays std::string on purpose.
std::string make_greeting()  // NOLINT(modernize-use-string-view)
{
    return "greetings-from-a-heap-allocated-string and the rest";
}

std::string make_short_greeting()  // NOLINT(modernize-use-string-view)
{
    return "hi there";
}

}  // namespace e001
