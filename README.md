# CLEO Studio

**An open-source script development environment for GTA San Andreas and CLEO.**

The goal is a modern, testable alternative built around a modular language engine, actionable diagnostics, versioned command data, automated tests, and reproducible builds.

> **Development status: early foundation (0.1.0-dev).** The editor is an executable Windows application. Lexical and preliminary label diagnostics work; a user-supplied SCM.INI-style opcode catalogue can be browsed in the GUI and inspected with the CLI. This version does **not** compile source into GTA SCM/CLEO bytecode and is not yet a replacement compiler.

## Current capabilities

- Windows x64 desktop application built with C++20 and Qt 6 Widgets.
- Multiple source tabs, open/save/save-as, unsaved-change prompts, and atomic writes.
- Encoding-aware source I/O for UTF-8/UTF-16 BOM files, explicit legacy-code-page selection for invalid UTF-8, preservation of the detected newline style, and a deliberate conversion-to-UTF-8 action.
- Line numbers, current-line highlight, and syntax highlighting for common Sanny/CLEO constructs.
- Lexer diagnostics for unterminated string literals/directives and unrecognized characters.
- Preliminary label analysis: duplicate labels are errors; unresolved `@label` references are warnings because labels may be external.
- SCM.INI-style opcode catalogue importer: retains opcode IDs, parameter counts, descriptions, and version metadata; detects malformed entries and warns on duplicate definitions.
- Searchable opcode browser in the GUI. Double-clicking a row inserts its explicit opcode form (for example, `0001: `) into the editor.
- CLI for script analysis and opcode-data inspection.
- GTA San Andreas CLEO compilation through an installed Sanny Builder CLI (`sanny.exe --game sa --compile ...`), as a compatibility backend. Sanny Builder must be installed separately.
- An additional experimental open-source `gta3sc.exe` backend for GTA3script `.sc` source files; it is not a full replacement for the Sanny Builder language.
- Automated Windows build, regression tests, and a packaged Windows x64 artifact in GitHub Actions.

## Run the Windows build

Download the artifact named **CLEO-Studio-Windows-x64** from the latest successful **Windows build** run under the repository's **Actions** tab. Extract the ZIP and launch `cleo-studio.exe`. The ZIP includes the Qt runtime libraries required by the editor.

## Use the application

1. Open or create a source file.
2. Press **F7** or choose **Проверка → Проверить структуру исходника** to run the current lexical and label analysis.
3. Choose **Данные → Загрузить базу SCM.INI...** and select a compatible opcode database, such as `SASCM.INI`.
4. Search the catalogue by opcode ID or description. Double-click an entry to insert its ID into the editor.
5. For normal CLEO scripts, save the source as `.txt` or `.sc`, include the `{$CLEO}` directive, configure `sanny.exe` under **Сборка**, then choose **Собрать CLEO для GTA San Andreas (Sanny CLI)**. Sanny Builder is an external compatibility compiler and is not bundled.
6. The separate `gta3sc` command is experimental and is intended for GTA3script `.sc` sources.

The imported data set's version matters. A successful import does not guarantee the database matches the game/CLEO version being targeted.

## Command-line tools

```powershell
cleo-studio-cli validate path/to/script.cs
cleo-studio-cli opcodes path/to/SASCM.INI
cleo-studio-cli --version
```

`validate` currently performs lexical checks and preliminary label analysis. It does not prove that commands, parameter types, control flow, or binary encoding are valid. `opcodes` checks and summarizes SCM.INI-style data; it does not generate bytecode.

## Build and open in Visual Studio 2022

This repository uses CMake as the single source of truth for target and source-file definitions. The setup script generates a native Visual Studio 2022 solution (CLEOStudio.sln) for the current machine; generated build files stay outside the source tree.

Requirements: Visual Studio 2022 with **Desktop development with C++** and CMake components, CMake 3.24+, and Qt 6.5+ Widgets for **MSVC 2022 x64**.

From PowerShell in the repository root, run the setup script (adjust the Qt path to your installation):

    .\scripts\configure-vs2022.ps1 -QtRoot "C:\Qt\6.8.3\msvc2022_64"

Then open **out\build\vs2022-x64\CLEOStudio.sln** in Visual Studio 2022, select **Release | x64**, and build the solution. It contains the GUI app, CLI, core library, and tests. To run tests from PowerShell:

    ctest --test-dir out/build/vs2022-x64 -C Release --output-on-failure

The .vsconfig file at the repository root records the Visual Studio components needed for this workflow. See [Visual Studio 2022 setup](docs/VISUAL-STUDIO-2022.md) for troubleshooting and manual commands.

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

CLEO Studio — открытая среда разработки скриптов GTA San Andreas/CLEO. Сейчас доступны Windows x64 редактор, вкладки, подсветка, анализатор меток, CLI и просмотрщик пользовательской базы SASCM.INI/SCM.INI. **Для совместимой компиляции CLEO под GTA San Andreas пока используется установленный sanny.exe как внешний backend.** Собственный компилятор ещё не реализован; gta3sc доступен отдельно как экспериментальный открытый backend.

### Сборка в Visual Studio 2022

Проект использует CMake как единый источник описания исходников и целей сборки. Скрипт подготовки создаёт обычное решение Visual Studio 2022 CLEOStudio.sln для текущего компьютера — вручную поддерживать отдельные копии vcxproj не требуется.

1. Установите Visual Studio 2022 с компонентами **Разработка классических приложений на C++** и CMake.
2. Установите Qt 6.5+ для **MSVC 2022 64-bit** с компонентом Widgets.
3. Откройте PowerShell в корне репозитория и выполните команду, указав свой путь к Qt:

       .\scripts\configure-vs2022.ps1 -QtRoot "C:\Qt\6.8.3\msvc2022_64"

4. Откройте out\build\vs2022-x64\CLEOStudio.sln в Visual Studio 2022.
5. Выберите конфигурацию **Release | x64** и соберите решение. В него входят графическое приложение, CLI, библиотека ядра и тесты.

Подробная инструкция и команды ручной настройки: [docs/VISUAL-STUDIO-2022.md](docs/VISUAL-STUDIO-2022.md). Файл .vsconfig в корне содержит список компонентов Visual Studio для этого проекта.
