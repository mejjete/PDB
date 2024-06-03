cmake_minimum_required(VERSION 3.10)
project(PDB)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_library(pdbmanager
    pdb_manager/PDBProcess.cpp
    pdb_manager/PDBDebug.cpp
    pdb_manager/PDBRuntime.cpp
    pdb_manager/PDB.hpp)

target_compile_options(pdbmanager PRIVATE -Wall -Wextra -Wunused-parameter)

add_executable(pdb_man
	pdb_manager/PDB.cpp)

target_compile_options(pdb_man PRIVATE -Wextra -Wunused-parameter)
target_link_libraries(pdb_man pdbmanager)
