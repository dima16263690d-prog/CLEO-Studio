#include "cleo_studio/analyzer.hpp"
#include "cleo_studio/lexer.hpp"
#include <cstdlib>
#include <iostream>
#include <string_view>
namespace {
int failures = 0;
void check(bool value, std::string_view message) {
    if (!value) { ++failures; std::cerr << "FAIL: " << message << '\n'; }
}
std::size_t countKind(const cleo_studio::LexResult& r, cleo_studio::TokenKind k) {
    std::size_t count = 0;
    for (const auto& t : r.tokens) if (t.kind == k) ++count;
    return count;
}
}
int main() {
    using namespace cleo_studio;
    const auto valid = lex("{$CLEO}\n:MAIN\nwait 1000\n// note\n");
    check(!valid.hasErrors(), "directive, label, number and comment should lex cleanly");
    check(countKind(valid, TokenKind::directive) == 1, "recognizes directives");
    check(countKind(valid, TokenKind::label) == 1, "recognizes labels");
    check(countKind(valid, TokenKind::identifier) == 1, "recognizes identifiers");
    check(countKind(valid, TokenKind::number) == 1, "recognizes decimal numbers");
    check(countKind(valid, TokenKind::comment) == 1, "recognizes comments");
    const auto str = lex("show_text \"hello \\\"world\\\"\"");
    check(!str.hasErrors(), "escaped quotes in string should work");
    check(countKind(str, TokenKind::string) == 1, "recognizes string literal");
    const auto gxt = lex("show_text_highpriority 'SUSPECT'");
    check(!gxt.hasErrors(), "single-quoted GXT strings should be accepted");
    check(countKind(gxt, TokenKind::string) == 1, "recognizes single-quoted string literals");
    const auto hex = lex("0A8C: read_memory 0x1234");
    check(!hex.hasErrors(), "opcode-like hex and hex number should work");
    check(countKind(hex, TokenKind::number) == 2, "recognizes opcode id and hex number");
    const auto badString = lex("show_text \"not closed\nwait 0");
    check(badString.hasErrors() && !badString.diagnostics.empty()
          && badString.diagnostics.front().code == "LEX001", "reports unterminated string");
    const auto badDirective = lex("{$CLEO\nwait 0");
    check(badDirective.hasErrors() && !badDirective.diagnostics.empty()
          && badDirective.diagnostics.front().code == "LEX002", "reports unterminated directive");
    check(lex("0x wait 0").hasErrors(), "reports a hex prefix without digits");

    const auto resolved = analyze("jump @START\\n:START\\nwait 0");
    check(!resolved.hasErrors(), "a forward label reference should resolve");
    check(resolved.diagnostics.empty(), "a declared target label should not produce diagnostics");
    const auto duplicate = analyze(":MAIN\\n:main");
    check(duplicate.hasErrors(), "duplicate labels are errors, case-insensitively");
    check(!duplicate.diagnostics.empty() && duplicate.diagnostics.front().code == "SEM001",
          "duplicate label diagnostic uses SEM001");
    const auto unresolved = analyze("jump @MISSING");
    check(!unresolved.hasErrors() && !unresolved.diagnostics.empty()
          && unresolved.diagnostics.front().code == "SEM002",
          "unresolved label references are warnings, not errors");
    if (failures != 0) { std::cerr << failures << " test assertion(s) failed.\n"; return EXIT_FAILURE; }
    std::cout << "All CLEO Studio lexer tests passed.\n";
    return EXIT_SUCCESS;
}
