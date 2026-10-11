# CLEO Studio

**An open-source script development environment for GTA San Andreas and CLEO.**

The goal is a modern, testable alternative built around a modular language engine, actionable diagnostics, automated tests, and reproducible builds.

> **Development status: early foundation (0.1.0-dev).** This milestone provides a desktop source editor, lexical analysis, and preliminary label diagnostics. It does **not** compile source files into GTA SCM/CLEO bytecode and is not yet a replacement compiler.

## Current foundation

- Windows desktop application built with C++20 and Qt 6 Widgets.
- Multiple source tabs, open/save/save-as, and dirty-file tracking.
- Line numbers and syntax highlighting for common Sanny/CLEO source constructs.
- Shared language core used by the GUI and CLI.
- Diagnostics for unterminated strings/directives, duplicate labels, and possibly unresolved @label references.
- Unit tests and a Windows GitHub Actions build/test workflow.

## Build

Requirements: CMake 3.24+, Visual Studio 2022 (Windows), and Qt 6.5+ with Widgets.

```powershell
cmake -S . -B build -DCMAKE_PREFIX_PATH="C:/Qt/6.x.x/msvc2022_64" -DBUILD_TESTING=ON
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

Command-line analysis:

```powershell
cleo-studio-cli validate path/to/script.cs
```

Opcode import reads metadata and descriptions without bundling third-party tables. Analysis does not prove that commands, parameter types, control flow, or binary encoding are valid. Unresolved labels are warnings because some target formats may reference external labels.

## Architecture

- `core/` — standalone C++20 lexer and lightweight analyzer; no Qt dependency.
- `app/` — Qt editor, line-number gutter, highlighting, and diagnostics output.
- `tests/` — core regression tests.
- `data/` — reserved for versioned opcode metadata and import tooling.

## Roadmap

1. Editor foundation and reliable diagnostics.
2. Parser, symbol table, opcode/parameter model, source navigation.
3. Versioned opcode metadata with documented provenance.
4. SCM/CLEO compiler backend verified against golden bytecode fixtures.
5. Build profiles, completion, signature help, and richer GTA integration.

## Engineering rules

- Never silently rewrite or discard user source.
- Keep parser/compiler code independent from the UI.
- Every language feature must have tests and known-good fixtures.
- Do not claim binary compatibility until reproducible tests prove it.
- Preserve licenses and attribution when integrating an existing project.

## License

MIT. See [LICENSE](LICENSE).

---

## Русский

CLEO Studio — открытая среда разработки скриптов GTA San Andreas/CLEO. Сейчас в проекте есть базовый редактор, подсветка, лексический анализ и предварительные предупреждения о ссылках на метки. **Компиляция в SCM/CLEO-байткод ещё не реализована.** Сначала обеспечим правильность языка и диагностики, затем подключим или реализуем компилятор и подтвердим совместимость тестами на реальных скриптах.
