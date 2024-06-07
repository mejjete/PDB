# Cmake contatining rules for building custom runtime library
# The following CMake is default choice and made for Open MPI PDB runtime
cmake_minimum_required(VERSION 3.10)
project(PDB)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ../build/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../build/bin)

# Find MPI package
find_package(MPI REQUIRED)

# Check if MPI is found
if(MPI_FOUND)
    message(STATUS "MPI found: ${MPI_CXX_COMPILER}")
    message(STATUS "MPI include path: ${MPI_INCLUDE_PATH}")
    message(STATUS "MPI libraries: ${MPI_LIBRARIES}")
else()
    message(FATAL_ERROR "MPI not found. Please install Open MPI. \n
        At the moment, PDB only supports Open MPI. \n
        Install OpenMPI: \n
        1. sudo apt update & sudo apt upgrade \n
        2. sudo apt install openmpi-bin openmpi-common libopenmpi-dev\n")
endif()

add_library(pdb_runtime SHARED 
	PDBRuntime.cpp)

add_executable(pdb_launch
    PDBLaunch.cpp)