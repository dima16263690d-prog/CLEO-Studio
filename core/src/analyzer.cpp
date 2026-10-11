#include "cleo_studio/analyzer.hpp"
#include <algorithm>
#include <cctype>
#include <string>
#include <unordered_map>
#include <utility>

namespace cleo_studio {
namespace {
std::string normalizeLabel(std::string value) {
    for (char& c : value) {
        const auto byte = static_cast<unsigned char>(c);
        if (byte < 0x80) c = static_cast<char>(std::tolower(byte));
    }
    return value;
}
} // namespace

LexResult analyze(std::string_view source) {
    LexResult result = lex(source);
    std::unordered_map<std::string, Token> labels;
    labels.reserve(result.tokens.size());

    for (const auto& token : result.tokens) {
        if (token.kind != TokenKind::label || token.lexeme.size() < 2) continue;
        const auto key = normalizeLabel(token.lexeme.substr(1));
        const auto [it, inserted] = labels.emplace(key, token);
        if (!inserted) {
            result.diagnostics.push_back(Diagnostic{
                Severity::error,
                "SEM001",
                "Duplicate label '" + token.lexeme + "'; first declared on line "
                    + std::to_string(it->second.line) + ".",
                token.line,
                token.column
            });
        }
    }

    for (std::size_t i = 0; i + 1 < result.tokens.size(); ++i) {
        const auto& at = result.tokens[i];
        const auto& name = result.tokens[i + 1];
        if (at.kind != TokenKind::punctuation || at.lexeme != "@"
            || name.kind != TokenKind::identifier || name.line != at.line) {
            continue;
        }
        const auto key = normalizeLabel(name.lexeme);
        if (labels.find(key) == labels.end()) {
            result.diagnostics.push_back(Diagnostic{
                Severity::warning,
                "SEM002",
                "Unresolved label reference '@" + name.lexeme
                    + "'; check whether it is defined in this script or provided externally.",
                at.line,
                at.column
            });
        }
    }

    std::stable_sort(result.diagnostics.begin(), result.diagnostics.end(),
        [](const Diagnostic& left, const Diagnostic& right) {
            if (left.line != right.line) return left.line < right.line;
            return left.column < right.column;
        });
    return result;
}
} // namespace cleo_studio
