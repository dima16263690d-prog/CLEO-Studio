# Importing opcode definitions

CLEO Studio can read a user-supplied SCM.INI-style opcode file. For San Andreas this is commonly named `SASCM.INI`.

Each opcode definition follows the established format:

```ini
VERSION=example-version
PUBLISHER=example-source
DATE=2026-01-01
0001=1,wait %1d% ms
00BF=2, $hours $minutes
```

The opcode ID is four hexadecimal digits. The value before the first comma is the parameter count; the remainder is a description string. Semicolons start comments. Other INI settings are ignored.

The importer records source line numbers, preserves descriptions, and reports malformed definitions. If an ID appears more than once, the later definition replaces the earlier entry and a warning is emitted, matching SCM.INI behavior described by Sanny Builder documentation.

## Safety and compatibility

The parser does not bundle or redistribute a third-party opcode database. Use a data file that you are allowed to use and keep its version/publisher information with your project. Import success confirms only that the file can be read; it does not certify that the definitions match the installed GTA/CLEO runtime.
