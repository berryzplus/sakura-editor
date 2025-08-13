cmake_minimum_required(VERSION 3.16)

# Find GTest
find_package(GTest CONFIG REQUIRED)

# Create a custom target for test_resource_zip generation
add_custom_target(test_resource_zip ALL
  COMMAND ${7ZIP_EXECUTABLE}
    u -tzip -r -mcu=on
    ${CMAKE_BINARY_DIR}/resources.ja-JP.zip
    ${CMAKE_SOURCE_DIR}/src/test/resources/test-plugin
    > NUL
  BYPRODUCTS ${CMAKE_BINARY_DIR}/resources.ja-JP.zip
  COMMENT "Generating resources.ja-JP.zip"
)

# define a variable of project name
set(PROJECT_NAME tests1)

# define source files of tests1
file(GLOB_RECURSE TESTS1_SOURCES
  ${CMAKE_SOURCE_DIR}/src/test/cpp/*.cpp
  ${CMAKE_SOURCE_DIR}/src/test/resources/*.cpp
  ${CMAKE_CURRENT_LIST_DIR}/test*.cpp)

if(MINGW)
  # coverage.cppをリストから削除
  list(REMOVE_ITEM TESTS1_SOURCES ${CMAKE_SOURCE_DIR}/src/test/resources/coverage.cpp)
endif(MINGW)

# define header files of tests1
file(GLOB_RECURSE TESTS1_HEADERS
  ${CMAKE_SOURCE_DIR}/src/test/cpp/*.hpp
  ${CMAKE_SOURCE_DIR}/src/test/cpp/*.h
  ${CMAKE_SOURCE_DIR}/src/test/resources/*.hpp
  ${CMAKE_SOURCE_DIR}/src/test/resources/*.h
  ${CMAKE_CURRENT_LIST_DIR}/*.hpp
  ${CMAKE_CURRENT_LIST_DIR}/*.h)

# define precompiled headers
set(TESTS1_PCH_HEADER ${CMAKE_CURRENT_LIST_DIR}/../../src/test/cpp/pch.h)

set(TESTS1_RESOURCE_SCRIPTS
  ${CMAKE_CURRENT_LIST_DIR}/../../src/test/resources/tests1_rc.rc)

if(MINGW)
  # Convert RC files to UTF-8 for MinGW
  convert_rc_files_to_utf8(TESTS1_RESOURCE_SCRIPTS ${CMAKE_BINARY_DIR})
endif(MINGW)

# define sources files of an executable
add_executable(${PROJECT_NAME} ${TESTS1_PCH_HEADER} ${TESTS1_SOURCES} ${TESTS1_RESOURCE_SCRIPTS} ${TESTS1_HEADERS})

# Enable precompiled headers
target_precompile_headers(${PROJECT_NAME} PRIVATE ${TESTS1_PCH_HEADER})

# add definitions for project
target_compile_definitions(${PROJECT_NAME}
  PUBLIC
    _CONSOLE
)

# add include directories for project
target_include_directories(${PROJECT_NAME}
  PRIVATE
    ${CMAKE_SOURCE_DIR}/src/test/cpp
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_SOURCE_DIR}/src/test/resources
)

# Add dependencies
add_dependencies(${PROJECT_NAME}
  test_resource_zip
)

# link libraries
target_link_libraries(${PROJECT_NAME}
  PRIVATE
    sakura_core
)

# link with GoogleTest
target_link_libraries(${PROJECT_NAME}
  PRIVATE
    GTest::gtest
    GTest::gmock)

if(MINGW)
  target_link_options(${PROJECT_NAME}
    PRIVATE
      -mconsole
  )
endif(MINGW)
