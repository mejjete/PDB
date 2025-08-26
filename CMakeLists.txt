cmake_minimum_required(VERSION 3.16)

project(PDB VERSION 0.2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

option(BUILD_BACKEND "Build the backend server" ON)
option(BUILD_FRONTEND "Build the frontend client" ON)

find_package(Doxygen COMPONENTS doxygen)

if(DOXYGEN_FOUND)
    set(DOXYGEN_INPUT_DIRS
        "${CMAKE_CURRENT_SOURCE_DIR}/pdb_manager"
        "${CMAKE_CURRENT_SOURCE_DIR}/pdb_runtime"
    )

    set(DOXYGEN_OUTPUT_DIR ${CMAKE_BINARY_DIR}/docs)
    set(DOXYGEN_GENERATE_HTML YES)
    set(DOXYGEN_GENERATE_MAN YES)
    set(DOXYGEN_OUTPUT_DIRECTORY ${DOXYGEN_OUTPUT_DIR})
    set(DOXYGEN_RECURSIVE YES)
    set(DOXYGEN_EXTRACT_ALL YES)
    set(DOXYGEN_GENERATE_LATEX NO)
    set(DOXYGEN_USE_MDFILE_AS_MAINPAGE ${CMAKE_CURRENT_SOURCE_DIR}/README.md)

    doxygen_add_docs(
        docs
        ${DOXYGEN_INPUT_DIRS}
        COMMENT "Generate documentation with Doxygen"
    )

    message(STATUS "Doxygen documentation will be generated in: ${DOXYGEN_OUTPUT_DIR}")
else()
    message(WARNING "Doxygen not found, documentation will not be generated.")
endif()

if(BUILD_BACKEND)
    add_subdirectory(pdb_manager)
endif()

if(BUILD_FRONTEND)
    add_subdirectory(pdb_frontend)
endif()