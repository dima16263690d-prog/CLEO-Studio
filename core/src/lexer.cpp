#include "cleo_studio/lexer.hpp"
#include <cctype>
#include <utility>

namespace cleo_studio {
namespace {
bool isIdentifierStart(unsigned char c) { return std::isalpha(c) != 0 || c == '_' || c >= 0x80; }
bool isIdentifierContinue(unsigned char c) { return std::isalnum(c) != 0 || c == '_' || c >= 0x80; }
bool isHexDigit(unsigned char c) { return std::isxdigit(c) != 0; }
bool isPunctuation(unsigned char c) {
    constexpr std::string_view punctuation = "()[]{},:;=+-*/<>!&|.%?$@^~\\";
    return punctuation.find(static_cast<char>(c)) != std::string_view::npos;
}
} // namespace

bool LexResult::hasErrors() const noexcept {
    for (const auto& d : diagnostics) if (d.severity == Severity::error) return true;
    return false;
}

LexResult lex(std::string_view source) {
    LexResult result;
    std::size_t offset = 0, line = 1, column = 1;
    auto advance = [&]() {
        const char c = source[offset++];
        if (c == '\n') { ++line; column = 1; } else { ++column; }
        return c;
    };
    const auto addToken = [&](TokenKind kind, std::size_t start, std::size_t startLine, std::size_t startColumn) {
        result.tokens.push_back(Token{kind, std::string(source.substr(start, offset - start)),
                                      startLine, startColumn, start, offset - start});
    };
    const auto addError = [&](std::string code, std::string message, std::size_t errorLine, std::size_t errorColumn) {
        result.diagnostics.push_back(Diagnostic{Severity::error, std::move(code), std::move(message), errorLine, errorColumn});
    };

    while (offset < source.size()) {
        const auto c = static_cast<unsigned char>(source[offset]);
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') { advance(); continue; }
        const std::size_t start = offset, startLine = line, startColumn = column;

        if (offset + 1 < source.size() && source[offset] == '/' && source[offset + 1] == '/') {
            while (offset < source.size() && source[offset] != '\n') advance();
            addToken(TokenKind::comment, start, startLine, startColumn);
            continue;
        }
        if (offset + 1 < source.size() && source[offset] == '{' && source[offset + 1] == '$') {
            advance(); advance();
            bool closed = false;
            while (offset < source.size() && source[offset] != '\n') {
                if (advance() == '}') { closed = true; break; }
            }
            addToken(TokenKind::directive, start, startLine, startColumn);
            if (!closed) addError("LEX002", "Unterminated compiler directive; expected '}'.", startLine, startColumn);
            continue;
        }
        if (source[offset] == '"' || source[offset] == '\'') {
            const char quote = source[offset];
            advance();
            bool closed = false;
            while (offset < source.size()) {
                const char current = source[offset];
                if (current == '\n' || current == '\r') break;
                if (current == '\\') {
                    advance();
                    if (offset < source.size() && source[offset] != '\n' && source[offset] != '\r') advance();
                    continue;
                }
                if (advance() == quote) { closed = true; break; }
            }
            addToken(TokenKind::string, start, startLine, startColumn);
            if (!closed) addError("LEX001", "Unterminated string literal; expected a closing quote.", startLine, startColumn);
            continue;
        }
        if (source[offset] == ':' && offset + 1 < source.size()
            && isIdentifierStart(static_cast<unsigned char>(source[offset + 1]))) {
            advance(); advance();
            while (offset < source.size() && isIdentifierContinue(static_cast<unsigned char>(source[offset]))) advance();
            addToken(TokenKind::label, start, startLine, startColumn);
            continue;
        }
        if (std::isdigit(c) != 0) {
            if (offset + 1 < source.size() && source[offset] == '0'
                && (source[offset + 1] == 'x' || source[offset + 1] == 'X')) {
                advance(); advance();
                const std::size_t digitsStart = offset;
                while (offset < source.size() && isHexDigit(static_cast<unsigned char>(source[offset]))) advance();
                addToken(TokenKind::number, start, startLine, startColumn);
                if (digitsStart == offset) addError("LEX004", "Hexadecimal number requires at least one digit.", startLine, startColumn);
                continue;
            }
            const bool mayBeOpcodeHex = source[offset] == '0';
            while (offset < source.size() && std::isdigit(static_cast<unsigned char>(source[offset])) != 0) advance();
            if (mayBeOpcodeHex && offset < source.size() && source[offset] >= 'A' && source[offset] <= 'F') {
                while (offset < source.size() && isHexDigit(static_cast<unsigned char>(source[offset]))) advance();
            }
            addToken(TokenKind::number, start, startLine, startColumn);
            continue;
        }
        if (isIdentifierStart(c)) {
            advance();
            while (offset < source.size() && isIdentifierContinue(static_cast<unsigned char>(source[offset]))) advance();
            addToken(TokenKind::identifier, start, startLine, startColumn);
            continue;
        }
        if (isPunctuation(c)) {
            advance();
            addToken(TokenKind::punctuation, start, startLine, startColumn);
            continue;
        }
        advance();
        addToken(TokenKind::unknown, start, startLine, startColumn);
        addError("LEX003", "Unrecognized character in source text.", startLine, startColumn);
    }
    return result;
}
} // namespace cleo_studio
