# CLEO Studio

**An open-source script development environment for GTA San Andreas and CLEO.**

CLEO Studio is being built as a modern, testable alternative for editing, validating, and eventually compiling GTA SA / CLEO scripts. The goal is not to copy the old editor line-for-line: the project is designed around a modular language engine, actionable diagnostics, automated tests, and a reproducible build.

> **Development status: early foundation (0.1.0-dev).** The first milestone provides a desktop source editor and lexical validation. It does **not** yet compile source files into GTA SCM/CLEO bytecode. Do not use it as a replacement compiler yet.

## Initial capabilities

- Windows desktop application built with C++20 and Qt 6 Widgets.
- Multiple source tabs, open/save/save-as, and dirty-file tracking.
- Line numbers and syntax highlighting for common Sanny/CLEO source constructs.
- Lexical validation with line/column diagnostics.
- A small command-line validator for scripts and automation.
- Unit tests for the language lexer.
- CMake-based build and Windows CI.

## Build

Requirements:
- CMake 3.24+
- A C++20 compiler (Visual Studio 2022 on Windows)
- Qt 6.5+ with the **Widgets** component

```powershell
cmake -S . -B build -DCMAKE_PREFIX_PATH="C:/Qt/6.x.x/msvc2022_64" -DBUILD_TESTING=ON
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

Launch the GUI executable from the build output directory. The command-line validator accepts:

```powershell
cleo-studio-cli validate path/to/script.cs
```

The validator checks lexical structure only; it does not prove that opcodes, parameter types, labels, or binary encoding are valid.

## Architecture

- `core/` — standalone language processing and diagnostic types; no Qt dependency.
- `app/` — Qt desktop editor, line-number gutter, highlighting, and output panel.
- `tests/` — repeatable core tests.
- `data/` — format documentation and future opcode metadata imports.
- `.github/workflows/` — automated Windows build and test.

## Roadmap

1. **Editor foundation:** tabs, file handling, highlighting, lexical diagnostics, tests.
2. **Language model:** parser, labels, directives, command/parameter structure, source ranges.
3. **Opcode data:** importer for versioned opcode definitions; source/version provenance and validation.
4. **Compiler backend:** select or implement a compatible SCM/CLEO backend only after test fixtures prove binary compatibility.
5. **Debug workflow:** structured diagnostics, jump-to-error, build profiles, generated artifacts and reproducible tests.
6. **GTA/CLEO integration:** target-version profiles and compatibility suites using known-good scripts.

## Engineering rules

- Never silently rewrite or discard user source.
- Keep parser/compiler code independent from the UI.
- Every language feature must have tests and known-good fixtures.
- Keep binary compatibility claims tied to reproducible input/output tests.
- Preserve original compiler/backend attribution and licenses if an existing project is integrated.

## License

MIT. See [LICENSE](LICENSE).

---

## Русский

CLEO Studio — открытая среда разработки скриптов для GTA San Andreas и CLEO. Проект рассчитан на современный редактор, понятные ошибки, автоматические тесты и в перспективе — совместимый компилятор.

**Сейчас готовится только базовая версия редактора и лексической проверки.** Компиляция в SCM/CLEO-байткод ещё не реализована. До появления проверенного компилятора проект не заменяет Sanny Builder.

Приоритеты: надёжность, отсутствие потери исходников, тесты на каждом этапе и подтверждение бинарной совместимости на реальных скриптах.
