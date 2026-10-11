#include "cleo_studio/analyzer.hpp"
#include <algorithm>
#include <charconv>
#include <cctype>
#include <cstdint>
#include <string>
#include <system_error>
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

bool parseExplicitOpcode(const Token& token, std::uint16_t& id) {
    if (token.kind != TokenKind::number || token.lexeme.size() != 4) return false;
    for (const char c : token.lexeme) {
        if (std::isxdigit(static_cast<unsigned char>(c)) == 0) return false;
    }
    unsigned int value = 0;
    const auto parsed = std::from_chars(token.lexeme.data(),
        token.lexeme.data() + token.lexeme.size(), value, 16);
    if (parsed.ec != std::errc{} || parsed.ptr != token.lexeme.data() + token.lexeme.size()
        || value > 0xFFFFU) return false;
    id = static_cast<std::uint16_t>(value);
    return true;
}
} // namespace

LexResult analyze(std::string_view source, const OpcodeCatalog* catalog) {
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

    // Explicit legacy opcode syntax: four hexadecimal digits followed by ':'.
    // Missing IDs are warnings because the selected catalogue may be incomplete or
    // target a different game/CLEO version. 0x8000-negated conditionals are accepted
    // when their base opcode is present.
    if (catalog != nullptr && !catalog->opcodes.empty()) {
        for (std::size_t i = 0; i + 1 < result.tokens.size(); ++i) {
            const auto& token = result.tokens[i];
            const auto& colon = result.tokens[i + 1];
            if (colon.kind != TokenKind::punctuation || colon.lexeme != ":"
                || colon.line != token.line) continue;

            std::uint16_t id = 0;
            if (!parseExplicitOpcode(token, id)) continue;

            bool found = catalog->opcodes.find(id) != catalog->opcodes.end();
            if (!found && id >= 0x8000U) {
                const auto baseId = static_cast<std::uint16_t>(id - 0x8000U);
                found = catalog->opcodes.find(baseId) != catalog->opcodes.end();
            }
            if (!found) {
                std::string printable = token.lexeme;
                for (char& c : printable) {
                    const auto byte = static_cast<unsigned char>(c);
                    if (byte < 0x80) c = static_cast<char>(std::toupper(byte));
                }
                result.diagnostics.push_back(Diagnostic{
                    Severity::warning,
                    "SEM003",
                    "Opcode " + printable
                        + " is absent from the loaded catalogue; verify the data set and target version.",
                    token.line,
                    token.column
                });
            }
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
