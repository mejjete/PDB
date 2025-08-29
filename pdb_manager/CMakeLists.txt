find_package(LLVM REQUIRED CONFIG)
find_package(Boost 1.75.0 REQUIRED system filesystem coroutine thread)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

link_directories(${Boost_LIBRARY_DIRS})

add_library(pdbmanager
    PDBProcess.cpp
    GDBDebugger.cpp
    PDB.hpp)

add_library(dwarf_handlers
    PDB_DWARF_Handlers.cpp
)

add_executable(pdb_launch
        PDBLaunch.cpp)

add_executable(pdb_man
        PDB.cpp)

llvm_map_components_to_libnames(llvm_libs
    Object
    DebugInfoDWARF
    Support
)

target_compile_options(pdbmanager PRIVATE -Wall -Wextra -Wunused-parameter)
target_compile_definitions(pdbmanager PUBLIC -DBOOST_LEAF_NO_EXCEPTIONS)
target_compile_options(dwarf_handlers PRIVATE -Wall -Wextra -fno-exceptions)

target_include_directories(pdbmanager PRIVATE pdb_runtime ${CMAKE_CURRENT_SOURCE_DIR} ${Boost_INCLUDE_DIRS})
target_include_directories(pdb_launch PRIVATE pdb_runtime ${Boost_INCLUDE_DIRS})
target_include_directories(pdb_man PRIVATE pdb_runtime ${CMAKE_CURRENT_SOURCE_DIR} ${Boost_INCLUDE_DIRS})
target_include_directories(dwarf_handlers PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${LLVM_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})

target_link_libraries(pdbmanager PRIVATE Boost::system Boost::filesystem Boost::coroutine Boost::thread dwarf_handlers)
target_link_libraries(pdb_man PRIVATE pdbmanager)
target_link_libraries(dwarf_handlers PRIVATE ${llvm_libs})