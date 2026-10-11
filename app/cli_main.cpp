#include "cleo_studio/lexer.hpp"
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
namespace {
const char* severityName(cleo_studio::Severity s) { return s == cleo_studio::Severity::error ? "error" : "warning"; }
void printUsage() {
    std::cerr << "CLEO Studio CLI 0.1.0-dev\nUsage:\n  cleo-studio-cli validate <script-file>\n  cleo-studio-cli --version\n\nNote: validate checks lexical structure only; it does not compile bytecode.\n";
}
}
int main(int argc, char** argv) {
    if (argc == 2 && (std::string(argv[1]) == "--version" || std::string(argv[1]) == "-v")) {
        std::cout << "CLEO Studio CLI 0.1.0-dev\n"; return 0;
    }
    if (argc != 3 || std::string(argv[1]) != "validate") { printUsage(); return 64; }
    const std::string path = argv[2];
    std::ifstream input(path, std::ios::binary);
    if (!input) { std::cerr << "error: cannot open file: " << path << '\n'; return 2; }
    const std::string source((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
    const auto result = cleo_studio::lex(source);
    for (const auto& d : result.diagnostics) {
        std::cerr << path << ':' << d.line << ':' << d.column << ": " << severityName(d.severity)
                  << " [" << d.code << "]: " << d.message << '\n';
    }
    if (result.hasErrors()) {
        std::cerr << "Lexical validation failed with " << result.diagnostics.size() << " diagnostic(s).\n";
        return 1;
    }
    std::cout << "Lexical validation passed: " << result.tokens.size() << " token(s).\nNo SCM/CLEO bytecode was generated.\n";
    return 0;
}
