cmake_minimum_required(VERSION 3.10)
project(PDB)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_library(pdbmanager_lib
    pdb_manager/PDBDebug.cpp
    pdb_manager/PDBDebug.hpp)

target_compile_options(pdbmanager_lib PRIVATE -Wall -Wextra -Wunused-parameter)

# set(BOOST_ROOT "/usr/lib/boost*")
# find_package(Boost 1.66.0 REQUIRED COMPONENTS process)
# include_directories(${Boost_INCLUDE_DIRS})

add_executable(pdb_man
       	pdb_manager/PDBProcess.cpp	
	pdb_manager/PDBDebug.cpp
	pdb_manager/PDB.cpp)

target_compile_options(pdb_man PRIVATE -Wextra -Wunused-parameter)
