project(PDB)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ../build/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../build/bin)

# Cmake contatining rules for building custom runtime library
# The following CMake is default choice and made for Open MPI PDB runtime
add_library(pdb_runtime SHARED 
	PDBRuntime.cpp)

add_executable(pdb_launch
    PDBLaunch.cpp)