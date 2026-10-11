#pragma once
#include "cleo_studio/lexer.hpp"
#include <string_view>
namespace cleo_studio {
// Lexes source and reports duplicate labels plus possibly unresolved @label references.
// Unresolved references are warnings because some target formats allow external labels.
[[nodiscard]] LexResult analyze(std::string_view source);
} // namespace cleo_studio
