# Parallel Debugger for MPI Applications

<img width="5350" height="7500" alt="PDB_Concept" src="https://github.com/user-attachments/assets/fe9fa0d7-39ff-46a0-9494-1f368dbf6c2e" />

## Supported Operating Systems

PDB primarily supports Linux and MacOS. The suppot for Windows is yet to come but is not included in the latest version. 

## Dependencies 

The minimal versions of libraries, the PDB is dependent on the listed below:
- Boost 1.75.0      - Boost::leaf
- LLVM 18           - LLVM Dwarf parser
- Qt 6 (for UI)

### System-Level Installation

Linux 

- Boost 
$ sudo apt install libboost-all-dev 

- LLVM 18
$ sudo apt install llv

MacOS
- Boost

# Update your package list
$ brew update
$ brew upgrade
# Install the Boost development libraries
$ brew install boost

-LLVM 
Since LLVM is available on macOS from box, the version should be checked whether 

! Back-End could be build independently only including 

