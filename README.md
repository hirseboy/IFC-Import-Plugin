# IFC2BES++
IFC to building energy simulation (BES) support and conversion library, written in C++.

Uses https://github.com/ifcquery/ifcplusplus to read IFC files. Then converts/extracts and provides data needed for Building Energy Performance Simulation tools in a convenient high-level interfaces.


## Building plugin

1) Clone repository
2) Check that submodule is checking out master-branch
3) go to externals and run createLinuxSymlinks.sh on Linux or createWindowsSoftlinks.bat on windows. They create symlinks to the needed libraries from the SIM-VICUS repository. Please check afterwards if all symlinks are valid. Sometimes empty folders are beeing created, then just rerun the os specific script.
4) take the project file from build/Qt/IFC-Import-Plugin.pro and open it in a QtCreator-Session
5) Plugin should be compiling
