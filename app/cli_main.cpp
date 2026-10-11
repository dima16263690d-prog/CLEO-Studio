#include "cleo_studio/analyzer.hpp"
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
namespace {
const char* severityName(cleo_studio::Severity s) { return s == cleo_studio::Severity::error ? "error" : "warning"; }
void printUsage() {
    std::cerr << "CLEO Studio CLI 0.1.0-dev\nUsage:\n  cleo-studio-cli validate <script-file>\n  cleo-studio-cli --version\n\nNote: validate does not compile SCM/CLEO bytecode.\n";
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
    const auto result = cleo_studio::analyze(source);
    std::size_t errors = 0, warnings = 0;
    for (const auto& d : result.diagnostics) {
        if (d.severity == cleo_studio::Severity::error) ++errors; else ++warnings;
        std::cerr << path << ':' << d.line << ':' << d.column << ": " << severityName(d.severity)
                  << " [" << d.code << "]: " << d.message << '\n';
    }
    std::cout << "Analysis completed: " << result.tokens.size() << " token(s), "
              << errors << " error(s), " << warnings << " warning(s).\n"
              << "No SCM/CLEO bytecode was generated.\n";
    return result.hasErrors() ? 1 : 0;
}
