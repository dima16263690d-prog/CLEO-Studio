#include "cleo_studio/opcode_catalog.hpp"

#include <algorithm>
#include <charconv>
#include <cctype>
#include <string>
#include <system_error>
#include <utility>

namespace cleo_studio {
namespace {

std::string_view trim(std::string_view value) {
    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.front())) != 0) {
        value.remove_prefix(1);
    }
    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.back())) != 0) {
        value.remove_suffix(1);
    }
    return value;
}

std::string upperAscii(std::string_view value) {
    std::string result(value);
    for (char& c : result) {
        const auto byte = static_cast<unsigned char>(c);
        if (byte < 0x80) c = static_cast<char>(std::toupper(byte));
    }
    return result;
}

bool isHexId(std::string_view value) {
    return value.size() == 4 && std::all_of(value.begin(), value.end(), [](char c) {
        return std::isxdigit(static_cast<unsigned char>(c)) != 0;
    });
}

void addIssue(OpcodeCatalog& catalog, Severity severity, std::string code,
              std::string message, std::size_t line) {
    catalog.diagnostics.push_back(Diagnostic{
        severity, std::move(code), std::move(message), line, 1
    });
}

} // namespace

bool OpcodeCatalog::hasErrors() const noexcept {
    return std::any_of(diagnostics.begin(), diagnostics.end(), [](const Diagnostic& d) {
        return d.severity == Severity::error;
    });
}

OpcodeCatalog parseOpcodeCatalog(std::string_view source) {
    OpcodeCatalog catalog;
    std::size_t lineNumber = 0;
    std::size_t offset = 0;

    while (offset <= source.size()) {
        ++lineNumber;
        const std::size_t newline = source.find('\n', offset);
        const std::size_t end = newline == std::string_view::npos ? source.size() : newline;
        std::string_view line = source.substr(offset, end - offset);
        if (!line.empty() && line.back() == '\r') line.remove_suffix(1);
        line = trim(line);

        if (!line.empty() && line.front() != ';' && line.front() != '#'
            && !(line.front() == '[' && line.back() == ']')) {
            const std::size_t equals = line.find('=');
            if (equals == std::string_view::npos) {
                addIssue(catalog, Severity::error, "OPC001",
                         "Expected an INI assignment or comment.", lineNumber);
            } else {
                const std::string_view key = trim(line.substr(0, equals));
                const std::string_view value = trim(line.substr(equals + 1));
                const std::string normalizedKey = upperAscii(key);

                if (normalizedKey == "VERSION") {
                    catalog.version = std::string(value);
                } else if (normalizedKey == "PUBLISHER") {
                    catalog.publisher = std::string(value);
                } else if (normalizedKey == "DATE") {
                    catalog.date = std::string(value);
                } else if (isHexId(key)) {
                    const std::size_t comma = value.find(',');
                    if (comma == std::string_view::npos) {
                        addIssue(catalog, Severity::error, "OPC002",
                                 "Opcode definition requires 'parameter-count, description'.",
                                 lineNumber);
                    } else {
                        std::size_t parameterCount = 0;
                        const std::string_view parameterText = trim(value.substr(0, comma));
                        const auto parseResult = std::from_chars(
                            parameterText.data(), parameterText.data() + parameterText.size(),
                            parameterCount, 10);
                        if (parameterText.empty() || parseResult.ec != std::errc{}
                            || parseResult.ptr != parameterText.data() + parameterText.size()) {
                            addIssue(catalog, Severity::error, "OPC003",
                                     "Opcode parameter count must be a non-negative decimal integer.",
                                     lineNumber);
                        } else {
                            const auto description = trim(value.substr(comma + 1));
                            if (description.empty()) {
                                addIssue(catalog, Severity::warning, "OPC004",
                                         "Opcode description is empty.", lineNumber);
                            }
                            unsigned int numericId = 0;
                            const auto idResult = std::from_chars(
                                key.data(), key.data() + key.size(), numericId, 16);
                            if (idResult.ec != std::errc{} || idResult.ptr != key.data() + key.size()) {
                                addIssue(catalog, Severity::error, "OPC005",
                                         "Opcode identifier could not be parsed.", lineNumber);
                            } else {
                                const auto id = static_cast<std::uint16_t>(numericId);
                                const auto existing = catalog.opcodes.find(id);
                                if (existing != catalog.opcodes.end()) {
                                    ++catalog.duplicateDefinitions;
                                    addIssue(catalog, Severity::warning, "OPC006",
                                        "Duplicate opcode; the later definition replaces the earlier one.",
                                        lineNumber);
                                    existing->second = OpcodeDefinition{
                                        id, parameterCount, std::string(description), lineNumber
                                    };
                                } else {
                                    catalog.opcodes.emplace(id, OpcodeDefinition{
                                        id, parameterCount, std::string(description), lineNumber
                                    });
                                }
                            }
                        }
                    }
                } else if (key.size() == 4) {
                    addIssue(catalog, Severity::error, "OPC007",
                             "Opcode identifiers must contain exactly four hexadecimal digits.",
                             lineNumber);
                }
                // Other INI keys are configuration data rather than opcode entries.
            }
        }

        if (newline == std::string_view::npos) break;
        offset = newline + 1;
    }

    std::stable_sort(catalog.diagnostics.begin(), catalog.diagnostics.end(),
        [](const Diagnostic& left, const Diagnostic& right) {
            if (left.line != right.line) return left.line < right.line;
            return left.column < right.column;
        });
    return catalog;
}

} // namespace cleo_studio
