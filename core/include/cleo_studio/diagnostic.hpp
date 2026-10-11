#pragma once
#include <cstddef>
#include <string>
namespace cleo_studio {
enum class Severity { warning, error };
struct Diagnostic {
    Severity severity{Severity::error};
    std::string code;
    std::string message;
    std::size_t line{1};
    std::size_t column{1};
};
} // namespace cleo_studio
