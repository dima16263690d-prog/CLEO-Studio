#pragma once
#include "cleo_studio/diagnostic.hpp"
#include <cstddef>
#include <string>
#include <string_view>
#include <vector>
namespace cleo_studio {
enum class TokenKind { identifier, number, string, label, directive, comment, punctuation, unknown };
struct Token {
    TokenKind kind{TokenKind::unknown};
    std::string lexeme;
    std::size_t line{1};
    std::size_t column{1};
    std::size_t offset{0};
    std::size_t length{0};
};
struct LexResult {
    std::vector<Token> tokens;
    std::vector<Diagnostic> diagnostics;
    [[nodiscard]] bool hasErrors() const noexcept;
};
// Lexes without modifying source. Offsets and columns currently count UTF-8 bytes.
[[nodiscard]] LexResult lex(std::string_view source);
} // namespace cleo_studio
