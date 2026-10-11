# GTA San Andreas CLEO build workflow

## Compatible build using Sanny Builder CLI

Use this path for ordinary Sanny Builder source intended to produce a CLEO script for GTA San Andreas.

1. Install Sanny Builder separately and locate `sanny.exe`.
2. In CLEO Studio, open or save the source as `.txt` or `.sc`.
3. Ensure the source includes `{$CLEO}`. This marks it as a CLEO script rather than the main SCM.
4. In **Сборка**, choose **Указать путь к sanny.exe...**.
5. Choose **Собрать CLEO для GTA San Andreas (Sanny CLI)** and select the output `.cs` file.
6. Test the generated file with the intended GTA SA and CLEO versions.

CLEO Studio invokes the CLI with the GTA SA game target and writes the result to a temporary output first. The destination is updated only after the external compiler exits successfully and produces a file, avoiding replacement of an existing result after a failed build.

Sanny Builder CLI reference: https://docs.sannybuilder.com/editor/cli

## Open-source experimental backend

The separate `gta3sc.exe` integration uses `--config=gtasa` and `--cs`. It is limited to GTA3script syntax and is not guaranteed to support all Sanny Builder constructs.

Upstream: https://github.com/thelink2012/gta3sc

## Important limits

- This project does not bundle either compiler.
- Invoking an external compiler is a compatibility bridge, not a native CLEO Studio compiler.
- Runtime correctness must be checked in the game; successful compilation cannot prove that a script behaves correctly.
