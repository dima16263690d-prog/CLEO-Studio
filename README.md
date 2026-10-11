# CLEO Studio

**An open-source script development environment for GTA San Andreas and CLEO.**

The goal is a modern, testable alternative built around a modular language engine, actionable diagnostics, versioned command data, automated tests, and reproducible builds.

> **Development status: early foundation (0.1.0-dev).** The editor is an executable Windows application. Lexical and preliminary label diagnostics work; a user-supplied SCM.INI-style opcode catalogue can be browsed in the GUI and inspected with the CLI. This version does **not** compile source into GTA SCM/CLEO bytecode and is not yet a replacement compiler.

## Current capabilities

- Windows x64 desktop application built with C++20 and Qt 6 Widgets.
- Multiple source tabs, open/save/save-as, unsaved-change prompts, and atomic writes.
- Line numbers, current-line highlight, and syntax highlighting for common Sanny/CLEO constructs.
- Lexer diagnostics for unterminated string literals/directives and unrecognized characters.
- Preliminary label analysis: duplicate labels are errors; unresolved `@label` references are warnings because labels may be external.
- SCM.INI-style opcode catalogue importer: retains opcode IDs, parameter counts, descriptions, and version metadata; detects malformed entries and warns on duplicate definitions.
- Searchable opcode browser in the GUI. Double-clicking a row inserts its explicit opcode form (for example, `0001: `) into the editor.
- CLI for script analysis and opcode-data inspection.
- Experimental optional GTA3script backend integration using an externally installed `gta3sc.exe`; this is not a claim of full Sanny Builder dialect compatibility.
- Automated Windows build, regression tests, and a packaged Windows x64 artifact in GitHub Actions.

## Run the Windows build

Download the artifact named **CLEO-Studio-Windows-x64** from the latest successful **Windows build** run under the repository's **Actions** tab. Extract the ZIP and launch `cleo-studio.exe`. The ZIP includes the Qt runtime libraries required by the editor.

## Use the application

1. Open or create a source file.
2. Press **F7** or choose **Проверка → Проверить структуру исходника** to run the current lexical and label analysis.
3. Choose **Данные → Загрузить базу SCM.INI...** and select a compatible opcode database, such as `SASCM.INI`.
4. Search the catalogue by opcode ID or description. Double-click an entry to insert its ID into the editor.
5. For experimental `.sc` sources, configure `gta3sc.exe` under **Сборка** and use the experimental build command. This backend requires a compatible gta3sc installation and is not a general Sanny Builder compiler.

The imported data set's version matters. A successful import does not guarantee the database matches the game/CLEO version being targeted.

## Command-line tools

```powershell
cleo-studio-cli validate path/to/script.cs
cleo-studio-cli opcodes path/to/SASCM.INI
cleo-studio-cli --version
```

`validate` currently performs lexical checks and preliminary label analysis. It does not prove that commands, parameter types, control flow, or binary encoding are valid. `opcodes` checks and summarizes SCM.INI-style data; it does not generate bytecode.

## Build from source

Requirements: CMake 3.24+, Visual Studio 2022 (Windows), and Qt 6.5+ with Widgets.

```powershell
cmake -S . -B build -DCMAKE_PREFIX_PATH="C:/Qt/6.x.x/msvc2022_64" -DBUILD_TESTING=ON
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

## Architecture

- `core/` — standalone C++20 lexer, label analyzer, opcode-catalogue parser, and diagnostics; no Qt dependency.
- `app/` — Qt editor, line-number gutter, highlighting, opcode browser, and diagnostics panel.
- `tests/` — core regression tests.
- `data/` — reserved for versioned opcode metadata and import tooling.

## Roadmap

1. **Language model:** parse statements, parameters, directives, expressions, and source spans.
2. **Opcode integration:** use imported catalogues to improve navigation, command information, and safe diagnostics.
3. **Editor productivity:** completion, signature help, jump-to-diagnostic, project/workspace settings.
4. **Native compiler:** move beyond the optional gta3sc adapter only after defining the language subset and proving output compatibility with golden binary fixtures.
5. **GTA/CLEO integration:** build profiles, target-version validation, and tests with real scripts.

## Engineering rules

- Never silently rewrite or discard user source.
- Keep parser/compiler code independent from the UI.
- Every language feature must have tests and known-good fixtures.
- Do not claim binary compatibility until reproducible tests prove it.
- Preserve licenses and attribution when integrating existing projects or opcode data.

## License

MIT. See [LICENSE](LICENSE).

---

## Русский

CLEO Studio — открытая среда разработки скриптов GTA San Andreas/CLEO. Сейчас доступны Windows x64 редактор, вкладки, подсветка, анализатор меток, CLI и просмотрщик пользовательской базы `SASCM.INI`/SCM.INI. **Компиляция в SCM/CLEO-байткод пока не реализована.** Следующий этап — полноценный парсер языка и диагностика на основе опкодов, затем совместимый компилятор, проверенный на реальных скриптах.
