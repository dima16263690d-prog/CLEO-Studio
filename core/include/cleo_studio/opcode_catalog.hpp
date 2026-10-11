#pragma once

#include "cleo_studio/diagnostic.hpp"

#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace cleo_studio {

struct OpcodeDefinition {
    std::uint16_t id{0};
    std::size_t parameterCount{0};
    std::string description;
    std::size_t sourceLine{0};
};

struct OpcodeCatalog {
    std::string version;
    std::string publisher;
    std::string date;
    std::map<std::uint16_t, OpcodeDefinition> opcodes;
    std::vector<Diagnostic> diagnostics;
    std::size_t duplicateDefinitions{0};

    [[nodiscard]] bool hasErrors() const noexcept;
};

// Reads the documented SCM.INI-style syntax (xxxx=N, description).
// Unknown non-opcode INI settings are ignored; later duplicate opcode entries win.
[[nodiscard]] OpcodeCatalog parseOpcodeCatalog(std::string_view source);

} // namespace cleo_studio
