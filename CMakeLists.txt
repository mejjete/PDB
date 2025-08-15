cmake_minimum_required(VERSION 3.5)

project(PDB VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
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

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
)

add_library(pdbmanager
    pdb_manager/PDBProcess.cpp
    pdb_manager/PDBDebug.cpp
    pdb_manager/GDBDebugger.cpp
    pdb_manager/PDB.hpp)

add_executable(pdb_launch
        pdb_manager/PDBLaunch.cpp)

add_executable(pdb_man
	pdb_manager/PDB.cpp)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(PDB
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        rsc.qrc
        setorremovebreakpointdialog.h setorremovebreakpointdialog.cpp setorremovebreakpointdialog.ui
        highlighter.h highlighter.cpp
    )
endif()

target_compile_options(pdbmanager PRIVATE -Wall -Wextra -Wunused-parameter)
target_include_directories(pdbmanager PRIVATE pdb_manager pdb_runtime ${CMAKE_SOURCE_DIR})
target_include_directories(PDB PRIVATE pdb_manager pdb_runtime ${CMAKE_SOURCE_DIR})
target_link_libraries(PDB PRIVATE Qt${QT_VERSION_MAJOR}::Widgets pdbmanager)
target_link_libraries(pdb_man PRIVATE pdbmanager)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.PDB)
endif()
set_target_properties(PDB PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS PDB
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(PDB)
endif()
