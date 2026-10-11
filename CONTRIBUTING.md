# Contributing to CLEO Studio

## Engineering rules

1. Keep the core independent from Qt.
2. Add regression tests for lexer, parser, and compiler behavior changes.
3. Do not claim SCM/CLEO binary compatibility without a known-good fixture and reproducible test.
4. Never rewrite source text implicitly.
5. Do not redistribute third-party opcode data/code until its license and attribution requirements are reviewed.

## Local checks

```powershell
cmake -S . -B build -DCMAKE_PREFIX_PATH="C:/Qt/6.x.x/msvc2022_64" -DBUILD_TESTING=ON
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

Bug reports should include the smallest reproducing script, target GTA/CLEO version, expected behavior, and actual behavior.
