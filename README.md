# Parallel Debugger for MPI Applications

  

<img width="5350" height="7500" alt="PDB_Concept" src="https://github.com/user-attachments/assets/fe9fa0d7-39ff-46a0-9494-1f368dbf6c2e" />

  

## Supported Operating Systems

PDB primarily supports Linux and MacOS. The support for Windows is yet to come but is not included in the latest version.

## Dependencies

The PDB could be build with clang 18 and gcc 13, potentially with any other compilers that support C++ 17. The minimal versions of libraries, the PDB is dependent on, listed below:

- Boost 1.75.0 
- LLVM 18
- Qt 6 (for UI)

## System-Level Installation

Installation of Boost library is taken from the official documentation. Please check carefully which library you are installing.

**Linux**
```none
# Update your package list
$ sudo apt update

# Install the Boost development libraries
$ sudo apt install libboost-all-dev
# Install the LLVM 18 toolchain
$ sudo apt install llvm-18-dev
```

**MacOS**
```none
# Update your package list
$ brew update
$ brew upgrade

# Install the Boost development libraries
$ brew install boost
# Install the LLVM 18 toolchain
$ brew install llvm-18-dev
```

> [!Note]
> Qt is not a strict requirements for building the entire project. The Back-End can be build independently, but in case you want default UI, go and get Open Source Qt 6. 

## Additional Dependencies

If you are going to use PDB for its primary goal, therefore debugging MPI applications you would need MPI installed. We recommend installing OpenMPI on a desktop machine, as it's open and freely available library. For any other MPI not mentioned here, you are using PDB on your own risk. To install OpenMPI:

**Linux**
```
$ sudo apt install libopenmpi-dev
``` 

MacOS
```
$ brew install openmpi
```

> [!Note]
> The latest MPI version supported by PDB is MPI 4.0. Check which library is installed on your system before running a debugger. To install or re-install the desired version of MPI please contact the official website: https://docs.open-mpi.org/en/v5.0.x/index.html
> 


## Building PDB

To clone and build PDB run:

```
# Clone PDB
$ git clone git@github.com:mejjete/PDB.git
$ cd PDB && mkdir build && cd build

# Check requirements and configure project
$ cmake ..
$ cmake --build .
# Build docs (Doxygen required)
$ make docs
```

If the PDB is built successfully, the executables located under the `{project_root}/build/bin` directory. The target PDB executable would be `./PDB` Have fun!!!
