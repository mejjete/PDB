
find_package(LLVM REQUIRED CONFIG)
find_package(Boost 1.75.0 REQUIRED)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

add_library(pdbmanager
    PDBProcess.cpp
    GDBDebugger.cpp
    PDB.hpp)

add_executable(pdb_launch
        PDBLaunch.cpp)

add_executable(pdb_man
        PDB.cpp)

add_custom_command(
    TARGET pdbmanager
    PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "=== Building back-end server ==="
)

target_compile_options(pdbmanager PRIVATE -Wall -Wextra -Wunused-parameter -fno-exceptions)
target_compile_definitions(pdbmanager PUBLIC -DBOOST_LEAF_NO_EXCEPTIONS)
target_include_directories(pdbmanager PRIVATE pdb_runtime ${CMAKE_CURRENT_SOURCE_DIR} ${LLVM_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})
target_include_directories(pdb_launch PRIVATE pdb_runtime ${Boost_INCLUDE_DIRS})
target_include_directories(pdb_man PRIVATE pdb_runtime ${CMAKE_CURRENT_SOURCE_DIR} ${Boost_INCLUDE_DIRS})

llvm_map_components_to_libnames(llvm_libs
    Object
    DebugInfoDWARF
    Support
)

target_link_libraries(pdbmanager PRIVATE ${llvm_libs})
target_link_libraries(pdb_man PRIVATE pdbmanager)
