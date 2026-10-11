# Development roadmap

## Milestone 0 — editor foundation
- [x] CMake project, Qt editor tabs, line numbers and syntax highlighting
- [x] Open/save/save-as, modified-file prompts and atomic writes
- [x] Shared lexer, CLI, preliminary label analysis and regression tests
- [x] Windows CI build/test workflow
- [ ] Verify the first CI run and fix any build/runtime issues

## Milestone 1 — language model
- [ ] Establish fixtures from real Sanny Builder / CLEO scripts
- [ ] Parse statements, explicit opcode IDs, labels, directives and parameter forms
- [x] Detect duplicate labels and warn on possible unresolved @label references
- [ ] Build accurate symbol rules for each target edit mode
- [ ] Track source spans for jump-to-diagnostic
- [ ] Test comments, strings, variables, arrays and control flow

## Milestone 2 — data and compatibility
- [ ] Import versioned opcode definitions with license and provenance
- [ ] Record game/CLEO target versions and avoid mixing data sets
- [ ] Establish golden binary fixtures from a trusted existing compiler
- [ ] Select or implement a backend after license/compatibility review
- [ ] Verify byte-for-byte output before claiming compatibility

## Milestone 3 — productive workflow
- [ ] GTA SA and CLEO build profiles
- [ ] Clickable diagnostics and build output panel
- [ ] Project/workspace files and configurable include paths
- [ ] Opcode completion, signature help and navigation
- [ ] Optional formatter with round-trip tests

A feature is complete only when documented and tested. Compiler features additionally require known-good binary fixtures.
