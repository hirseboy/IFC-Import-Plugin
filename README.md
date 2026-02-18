# SIM-VICUS IFC-IMPORT PLUGIN

[![Ubuntu 24.04](https://github.com/hirseboy/IFC-Import-Plugin/actions/workflows/IFCImportPlugin-Ubuntu2404.yml/badge.svg)](https://github.com/hirseboy/IFC-Import-Plugin/actions/workflows/IFCImportPlugin-Ubuntu2404.yml)
[![Windows VC2022](https://github.com/hirseboy/IFC-Import-Plugin/actions/workflows/IFCImportPlugin-WindowsVC2022.yml/badge.svg)](https://github.com/hirseboy/IFC-Import-Plugin/actions/workflows/IFCImportPlugin-WindowsVC2022.yml)

IFC to building energy simulation (BES) support and conversion library, written in C++.

Uses https://github.com/ifcquery/ifcplusplus to read IFC files. Then converts/extracts and provides data needed for Building Energy Performance Simulation tools in a convenient high-level interfaces.

Version 1.1, requires SIM-VICUS >= 1.5.

## Prerequisites

- CMake >= 2.8
- Qt6 (Core, Gui, Widgets, Xml, Svg, PrintSupport, Network, Concurrent)
- C++17 compiler

## Building

### 1. Clone repository and initialize submodules

```bash
git clone --recursive <repo-url>
```

Check that the SIM-VICUS submodule is on the master branch. Sometimes the head points to a wrong commit.

### 2. Create symlinks to SIM-VICUS libraries

```bash
cd externals
./_createLinuxSymlinks.sh     # Linux
# createWindowsSoftlinks.bat  # Windows (requires admin privileges)
```

This creates symlinks to needed libraries (IBK, IBKMK, TiCPP, QtExt, etc.) from the SIM-VICUS submodule. Check afterwards that all symlinks are valid. If empty folders are created, rerun the script.

### 3. Build with CMake

**Linux/macOS:**

```bash
cd build/cmake
./build.sh [cores] [debug|release|reldeb] [verbose]
# Example:
./build.sh 8 release
```

**Windows (Visual Studio 2022):**

```bat
cd build\cmake
build_VC_x64.bat
```

### Build output

| Platform | Executable | Plugin |
|----------|-----------|--------|
| Linux | `bin/release/IFC2BESTest` | `bin/release/libImportIFCPlugin.so` |
| Windows | `bin/release_x64/IFC2BESTest.exe` | `bin/release_x64/ifcplusplus.dll` |

### 4. Deploy plugin

Copy `libImportIFCPlugin.so` (or `.dll`) to the SIM-VICUS plugin folder.
