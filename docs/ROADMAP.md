# Development roadmap

## Milestone 0 — editor foundation
- [x] CMake project and source tree
- [x] Qt desktop editor: tabs, line numbers and basic highlighting
- [x] Open/save/save-as, dirty-file prompts, atomic writes
- [x] Shared lexical validator in GUI and CLI
- [x] Lexer regression tests and Windows CI workflow
- [ ] Run CI and address compiler/runtime issues

## Milestone 1 — language model
- [ ] Add fixtures from real Sanny Builder / CLEO scripts
- [ ] Parse statements, opcode IDs, labels, directives and parameter forms
- [ ] Detect duplicate and undefined labels
- [ ] Source spans and jump-to-diagnostic
- [ ] Cover comments, strings, variables and control flow with tests

## Milestone 2 — data and compatibility
- [ ] Import versioned opcode definitions with license/provenance
- [ ] Record game/CLEO target versions
- [ ] Establish golden binary fixtures from a trusted compiler
- [ ] Choose or implement a backend after license/compatibility review
- [ ] Verify byte-for-byte output before making compatibility claims

## Milestone 3 — productivity
- [ ] GTA SA and CLEO build profiles
- [ ] Clickable diagnostics and build output panel
- [ ] Project/workspace files and configurable include paths
- [ ] Opcode completion, signature help and navigation
- [ ] Optional formatter with round-trip tests

A feature is complete only when documented and tested. Compiler features also require known-good binary fixtures.
