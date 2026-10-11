#include "cleo_studio/analyzer.hpp"
#include "cleo_studio/opcode_catalog.hpp"
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
namespace {
const char* severityName(cleo_studio::Severity s) { return s == cleo_studio::Severity::error ? "error" : "warning"; }
void printUsage() {
    std::cerr << "CLEO Studio CLI 0.1.0-dev\nUsage:\n  cleo-studio-cli validate <script-file>\n  cleo-studio-cli opcodes <SASCM.INI>\n  cleo-studio-cli --version\n\nNote: validate does not compile SCM/CLEO bytecode.\n";
}
}
int main(int argc, char** argv) {
    if (argc == 2 && (std::string(argv[1]) == "--version" || std::string(argv[1]) == "-v")) {
        std::cout << "CLEO Studio CLI 0.1.0-dev\n"; return 0;
    }
    if (argc != 3 || (std::string(argv[1]) != "validate" && std::string(argv[1]) != "opcodes")) {
        printUsage(); return 64;
    }
    const std::string path = argv[2];
    std::ifstream input(path, std::ios::binary);
    if (!input) { std::cerr << "error: cannot open file: " << path << '\n'; return 2; }
    const std::string source((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
    if (std::string(argv[1]) == "opcodes") {
        const auto catalog = cleo_studio::parseOpcodeCatalog(source);
        std::cout << "Opcode definitions loaded: " << catalog.opcodes.size() << "\n";
        if (!catalog.version.empty()) std::cout << "Version: " << catalog.version << "\n";
        if (!catalog.publisher.empty()) std::cout << "Publisher: " << catalog.publisher << "\n";
        if (!catalog.date.empty()) std::cout << "Date: " << catalog.date << "\n";
        if (catalog.duplicateDefinitions != 0) {
            std::cout << "Duplicate IDs replaced by later entries: " << catalog.duplicateDefinitions << "\n";
        }
        for (const auto& diagnostic : catalog.diagnostics) {
            const char* severity = diagnostic.severity == cleo_studio::Severity::error ? "error" : "warning";
            std::cerr << path << ':' << diagnostic.line << ':' << diagnostic.column
                      << ": " << severity << " [" << diagnostic.code << "]: "
                      << diagnostic.message << "\n";
        }
        return catalog.hasErrors() ? 1 : 0;
    }
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
