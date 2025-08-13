cmake_minimum_required(VERSION 3.16)

set(PROJECT_NAME sakura)
project(${PROJECT_NAME} LANGUAGES CXX)

# Check if VCPKG_TARGET_TRIPLET is defined
if(NOT DEFINED VCPKG_TARGET_TRIPLET)
  message(FATAL_ERROR "VCPKG_TARGET_TRIPLET is not defined. Please set VCPKG_TARGET_TRIPLET (e.g., x64-windows, x86-windows, x64-mingw-static, etc.)")
endif()

# Extract CPU architecture from VCPKG_TARGET_TRIPLET
string(REGEX MATCH "^([^-]+)" VCPKG_ARCH "${VCPKG_TARGET_TRIPLET}")
if(NOT VCPKG_ARCH)
  message(FATAL_ERROR "Could not extract architecture from VCPKG_TARGET_TRIPLET: ${VCPKG_TARGET_TRIPLET}")
endif()
message(STATUS "Using architecture: ${VCPKG_ARCH}")

# Set DLL output directory if CMAKE_RUNTIME_OUTPUT_DIRECTORY is set
if(DEFINED CMAKE_RUNTIME_OUTPUT_DIRECTORY)
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
endif()

# Include sakura.cmake for funccode definitions
include(${CMAKE_SOURCE_DIR}/src/main/cmake/sakura.cmake)

# Create a custom command for version.h generation
add_custom_command(
  OUTPUT ${CMAKE_BINARY_DIR}/version.h
  COMMAND ${CMAKE_COMMAND} 
    -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
    -DBINARY_DIR=${CMAKE_BINARY_DIR}
    -P ${CMAKE_SOURCE_DIR}/src/main/cmake/version.cmake
  COMMENT "Generating version.h"
)

# Create a custom target that depends on the generated file
add_custom_target(generate_version_header ALL
  DEPENDS ${CMAKE_BINARY_DIR}/version.h
)

# Find HeaderMake
find_program(HEADER_MAKE_EXECUTABLE HeaderMake)

# Debug: Print HeaderMake executable path
if(HEADER_MAKE_EXECUTABLE)
  message(STATUS "Found HeaderMake: ${HEADER_MAKE_EXECUTABLE}")
else()
  message(STATUS "HeaderMake not found")
endif()

# Create a custom command for funccode_define generation
add_custom_command(
  OUTPUT ${CMAKE_BINARY_DIR}/Funccode_define.h
  COMMAND ${HEADER_MAKE_EXECUTABLE}
    -in=${CMAKE_SOURCE_DIR}/sakura_core/Funccode_x.hsrc
    -out=${CMAKE_BINARY_DIR}/Funccode_define.h
    -mode=define
  COMMENT "Generating Funccode_define.h"
)

# Create a custom target that depends on the generated file
add_custom_target(generate_funccode_define ALL
  DEPENDS ${CMAKE_BINARY_DIR}/Funccode_define.h
)

# Create a custom command for funccode_enum generation
add_custom_command(
  OUTPUT ${CMAKE_BINARY_DIR}/Funccode_enum.h
  COMMAND ${HEADER_MAKE_EXECUTABLE}
    -in=${CMAKE_SOURCE_DIR}/sakura_core/Funccode_x.hsrc
    -out=${CMAKE_BINARY_DIR}/Funccode_enum.h
    -mode=enum
    -enum=EFunctionCode
  COMMENT "Generating Funccode_enum.h"
)

# Create a custom target that depends on the generated file
add_custom_target(generate_funccode_enum ALL
  DEPENDS ${CMAKE_BINARY_DIR}/Funccode_enum.h
)

# add global definitions
add_compile_definitions(
  UNICODE
  _UNICODE
  $<$<CONFIG:Debug>:_DEBUG>
  $<$<CONFIG:Release>:NDEBUG>
)

# add include directories
include_directories(
  ${CMAKE_BINARY_DIR} 
  ${CMAKE_SOURCE_DIR}/sakura_core
)

if(MSVC)
  add_compile_definitions(
    NOMINMAX
  )
  add_compile_options(
    /source-charset:utf-8
    /execution-charset:shift_jis
  )
endif(MSVC)

if(MINGW)
  add_compile_options(
    -MMD
    -finput-charset=utf-8
    -fexec-charset=cp932
  )
endif(MINGW)

# ここからオブジェクトライブラリとexe

# define source files of sakura_core
file(GLOB_RECURSE SOURCES
  ${CMAKE_SOURCE_DIR}/src/main/cpp/*.cpp
  ${CMAKE_SOURCE_DIR}/sakura_core/*.cpp)

# define header files of sakura-editor
file(GLOB_RECURSE HEADERS
  ${CMAKE_SOURCE_DIR}/src/main/cpp/*.hpp
  ${CMAKE_SOURCE_DIR}/src/main/cpp/*.h
  ${CMAKE_SOURCE_DIR}/sakura_core/*.hpp
  ${CMAKE_SOURCE_DIR}/sakura_core/*.h)

# define precompiled headers
set(PCH_HEADER ${CMAKE_SOURCE_DIR}/sakura_core/StdAfx.h)

set(RESOURCE_SCRIPTS
  ${CMAKE_SOURCE_DIR}/src/main/resources/sakura_rc.rc
  ${CMAKE_SOURCE_DIR}/src/main/resources/sakura_rc.rc2)

if(MINGW)
  # Convert RC files to UTF-8 for MinGW
  convert_rc_files_to_utf8(RESOURCE_SCRIPTS ${CMAKE_BINARY_DIR})
endif(MINGW)

# Create sakura_core object library
add_library(sakura_core OBJECT ${PCH_HEADER} ${SOURCES} ${RESOURCE_SCRIPTS} ${HEADERS})

# Enable precompiled headers for sakura_core
target_precompile_headers(sakura_core PRIVATE ${PCH_HEADER})

# Set C++17 standard for sakura_core
target_compile_features(sakura_core PUBLIC cxx_std_20)

# Add dependencies for sakura_core
add_dependencies(sakura_core
  generate_version_header
  generate_funccode_define
  generate_funccode_enum
)

# Add include directories for sakura_core
target_include_directories(sakura_core
  PUBLIC
    ${CMAKE_SOURCE_DIR}/src/main/cpp
    ${CMAKE_SOURCE_DIR}/src/main/resources
)

# link libraries
target_link_libraries(sakura_core
  PUBLIC
    comctl32
    dwmapi
    imm32
    mpr
    msimg32
    ole32
    oleaut32
    shlwapi
    uuid
    uxtheme
    windowscodecs
    winmm
    winspool
)

if(MSVC)
  target_sources(sakura_core
    PUBLIC
      ${CMAKE_SOURCE_DIR}/src/main/resources/sakura.natvis
  )
  target_compile_options(sakura_core
    PRIVATE
      /FAsu
      /Fa"${CMAKE_BINARY_DIR}"
  )
  target_link_options(sakura_core
    PUBLIC
      "/MANIFESTDEPENDENCY:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='${VCPKG_ARCH}' publicKeyToken='6595b64144ccf1df' language='*'"
  )
endif(MSVC)

if(MINGW)
  target_link_options(sakura_core
    PUBLIC
      -municode
      -static
  )
endif(MINGW)

# define source files of main module
set(MAIN_SOURCES ${CMAKE_SOURCE_DIR}/sakura_core/StdAfx.cpp)

# Create main executable
add_executable(${PROJECT_NAME} WIN32 ${MAIN_SOURCES})

# link libraries
target_link_libraries(${PROJECT_NAME}
  PRIVATE
    sakura_core
)

# Find 7zip for archive extraction
find_program(7ZIP_EXECUTABLE 7z
  PATHS
    "$ENV{ChocolateyInstall}"
    "$ENV{VCPKG_ROOT}/downloads/tools"
  PATH_SUFFIXES
    bin
    "7zip-*-windows"
)

# Debug: Print 7z executable path
if(7ZIP_EXECUTABLE)
  message(STATUS "Found 7z: ${7ZIP_EXECUTABLE}")
else()
  message(STATUS "7z not found")
endif()

if(CMAKE_CONFIGURATION_TYPES)
  set(OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>")
else()
  set(OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
endif()

# Extract bregonig.dll from zip file
if(VCPKG_ARCH STREQUAL "x64" OR VCPKG_ARCH STREQUAL "x86")
  set(BREGONIG_ZIP_FILE "${CMAKE_SOURCE_DIR}/installer/externals/bregonig/bron420.zip")
  if(VCPKG_ARCH STREQUAL "x64")
    set(BREGONIG_EXTRACT_TARGET "x64/bregonig.dll")
  else()
    set(BREGONIG_EXTRACT_TARGET "bregonig.dll")
  endif()
  set(BREGONIG_OUTPUT_FILE "${OUTPUT_DIRECTORY}/bregonig.dll")

  add_custom_command(
    OUTPUT "${BREGONIG_OUTPUT_FILE}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${OUTPUT_DIRECTORY}"
    COMMAND ${7ZIP_EXECUTABLE} e "${BREGONIG_ZIP_FILE}" -o"${OUTPUT_DIRECTORY}" -y "${BREGONIG_EXTRACT_TARGET}" > NUL
    COMMAND ${CMAKE_COMMAND} -E make_directory "${OUTPUT_DIRECTORY}/license"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${OUTPUT_DIRECTORY}/license/bregonig"
    COMMAND ${7ZIP_EXECUTABLE} e "${BREGONIG_ZIP_FILE}" -o"${OUTPUT_DIRECTORY}/license/bregonig" -y "*.txt" > NUL
    DEPENDS ${BREGONIG_ZIP_FILE}
    COMMENT "Extracting bregonig.dll from ${BREGONIG_ZIP_FILE}"
  )

  add_custom_target(extract_bregonig ALL
    DEPENDS "${BREGONIG_OUTPUT_FILE}"
  )

  # Add dependencies
  add_dependencies(${PROJECT_NAME}
    extract_bregonig
  )
endif()

# Extract ctags.exe from zip file (only for x64 and x86 architectures)
if(VCPKG_ARCH STREQUAL "x64" OR VCPKG_ARCH STREQUAL "x86")
  set(CTAGS_VERSION "v6.1.0")
  set(CTAGS_ZIP_FILE "${CMAKE_SOURCE_DIR}/installer/externals/universal-ctags/ctags-${CTAGS_VERSION}-${VCPKG_ARCH}.zip")
  set(CTAGS_OUTPUT_FILE "${OUTPUT_DIRECTORY}/ctags.exe")

  add_custom_command(
    OUTPUT "${CTAGS_OUTPUT_FILE}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${OUTPUT_DIRECTORY}"
    COMMAND ${7ZIP_EXECUTABLE} e "${CTAGS_ZIP_FILE}" -o"${OUTPUT_DIRECTORY}" -y ctags.exe > NUL
    COMMAND ${CMAKE_COMMAND} -E make_directory "${OUTPUT_DIRECTORY}/license"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${OUTPUT_DIRECTORY}/license/ctags"
    COMMAND ${7ZIP_EXECUTABLE} e "${CTAGS_ZIP_FILE}" -o"${OUTPUT_DIRECTORY}/license/ctags" -y "license/*" > NUL
    DEPENDS ${CTAGS_ZIP_FILE}
    COMMENT "Extracting ctags.exe from ${CTAGS_ZIP_FILE}"
  )

  add_custom_target(extract_ctags ALL
    DEPENDS "${CTAGS_OUTPUT_FILE}"
  )

  # Add dependencies
  add_dependencies(${PROJECT_NAME}
    extract_ctags
  )
endif()

# Create language DLL projects
# en-US: 1033 (0x0409) - English (United States)
# zh-CN: 2052 (0x0804) - Chinese (Simplified, PRC)
create_language_dll(en-US 1033)
create_language_dll(zh-CN 2052)

# Include tests1 definition
include(${CMAKE_SOURCE_DIR}/tests/unittests/CMakeLists.txt)

enable_testing()

add_test(NAME UnitTest COMMAND $<TARGET_FILE:tests1> --gtest_output=xml:${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/tests1-googletest.xml)
