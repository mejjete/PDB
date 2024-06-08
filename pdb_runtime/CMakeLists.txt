# Cmake contatining rules for building custom runtime library
# The following CMake is default choice and made for Open MPI PDB runtime
cmake_minimum_required(VERSION 3.10)
project(PDB)

set(CMAKE_C_COMPILER mpicc)
set(CMAKE_CXX_COMPILER mpicxx)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ../build/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../build/bin)

find_package(MPI REQUIRED)

add_library(pdb_runtime SHARED 
	PDBRuntime.cpp)
    