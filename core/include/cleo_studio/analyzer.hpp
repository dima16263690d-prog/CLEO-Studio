#pragma once
#include "cleo_studio/lexer.hpp"
#include "cleo_studio/opcode_catalog.hpp"
#include <string_view>
namespace cleo_studio {
// Lexes source, diagnoses labels, and optionally warns on explicit opcode IDs
// that are absent from the selected catalog. Unresolved labels remain warnings.
[[nodiscard]] LexResult analyze(std::string_view source,
                                const OpcodeCatalog* catalog = nullptr);
} // namespace cleo_studio
