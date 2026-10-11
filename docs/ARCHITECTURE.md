# Architecture

## Boundaries

`cleo_core` is standard C++20 and must not depend on Qt. The GUI and CLI use the same library so core behavior can be tested without launching a window.

- **Editor UI:** tabs, safe file saving, highlighting, line/column status and diagnostics.
- **Lexer:** tokenization and source diagnostics. Offsets and columns currently count UTF-8 bytes.
- **Parser (planned):** opcode statements, arguments, labels, directives and source ranges.
- **Opcode catalog (planned):** versioned definitions with source attribution.
- **Compiler backend (planned):** isolated behind an interface and verified against real bytecode fixtures.

## Validation contract

Current `validate` checks lexical structure only. A successful check does not prove that a script can compile or run in GTA. The UI and CLI state this explicitly.

## Source safety

- Save through `QSaveFile` to avoid leaving a partially written file on write failure.
- Never format or rewrite source implicitly.
- Preserve exact input in regression tests.
