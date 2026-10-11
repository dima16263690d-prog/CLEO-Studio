# Experimental gta3sc backend

CLEO Studio can optionally invoke an external `gta3sc.exe` build for GTA3script `.sc` source files. The backend is not bundled with CLEO Studio; configure its executable path from the **Сборка** menu.

The current invocation targets GTA San Andreas and requests a CLEO script output. The compiler flags are adapted from the upstream `gta3sc` CLI. A successful process exit plus an existing output file is reported as a successful external build.

## Limitations

- This is an adapter to an external compiler, not a native CLEO Studio compiler.
- It is restricted to `.sc` source files for this command and does not promise full compatibility with every Sanny Builder syntax extension.
- The user must install a compatible gta3sc build with its configuration files and command definitions.
- CLEO Studio does not bundle or redistribute gta3sc binaries or data.
- Always test the generated script in the intended GTA/CLEO version. A zero exit code does not guarantee game-level behavior.

Upstream project and releases: https://github.com/thelink2012/gta3sc
