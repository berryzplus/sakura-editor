# sakura.cmake - utility functions for sakura editor project

# Function to convert RC files from UTF-16LE to UTF-8 for MinGW
# Parameters:
#   RC_FILES_VAR - Variable name containing list of RC file paths
#   BINARY_DIR   - Directory where converted files will be placed
function(convert_rc_files_to_utf8 RC_FILES_VAR BINARY_DIR)
    # Find iconv
    find_program(ICONV_PATH iconv REQUIRED)
    
    set(RC_FILES_UTF8)
    foreach(RC_FILE ${${RC_FILES_VAR}})
        get_filename_component(RC_NAME ${RC_FILE} NAME_WE)
        get_filename_component(RC_EXT ${RC_FILE} EXT)
        set(UTF8_RC_FILE ${BINARY_DIR}/${RC_NAME}${RC_EXT})
        
        add_custom_command(
            OUTPUT ${UTF8_RC_FILE}
            COMMAND ${ICONV_PATH} -f UTF-16LE -t UTF-8 "${RC_FILE}" > "${UTF8_RC_FILE}"
            DEPENDS ${RC_FILE}
            COMMENT "Converting ${RC_NAME}${RC_EXT} from UTF-16LE to UTF-8 using iconv"
        )
        
        list(APPEND RC_FILES_UTF8 ${UTF8_RC_FILE})
    endforeach()
    
    # Replace the original variable with UTF-8 converted files
    set(${RC_FILES_VAR} ${RC_FILES_UTF8} PARENT_SCOPE)
endfunction()

# Function to create a language DLL project
# Parameters:
#   LOCALE_NAME  - Name of the locale (e.g., en-US, zh-CN)
#   LOCALE_ID    - Locale identifier in decimal (e.g., 1033 for en-US, 2052 for zh-CN)
function(create_language_dll LOCALE_NAME LOCALE_ID)
    string(REPLACE "-" "_" LOCALE_NAME_UNDERSCORE "${LOCALE_NAME}")
    set(PROJECT_NAME sakura_lang_${LOCALE_NAME_UNDERSCORE})
    project(${PROJECT_NAME} LANGUAGES CXX)
    
    set(RC_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/src/main/resources)

    set(SCRIPT_SURFIX "")
    if(NOT "${LOCALE_NAME}" STREQUAL "ja-JP")
        set(SCRIPT_SURFIX "_${LOCALE_NAME}")
    endif()

    set(RESOURCE_SCRIPTS
        ${RC_FOLDER}/sakura_rc${SCRIPT_SURFIX}.rc
        ${RC_FOLDER}/sakura_rc${SCRIPT_SURFIX}.rc2)
    
    if(MINGW)
        # Convert RC files to UTF-8 for MinGW
        convert_rc_files_to_utf8(RESOURCE_SCRIPTS ${CMAKE_CURRENT_BINARY_DIR})
    endif(MINGW)
    
    # Create the library
    add_library(${PROJECT_NAME} MODULE ${RESOURCE_SCRIPTS})
    
    # Set include directories
    target_include_directories(${PROJECT_NAME}
      PRIVATE 
        ${CMAKE_CURRENT_BINARY_DIR} 
        ${CMAKE_CURRENT_SOURCE_DIR}/src/main/resources
    )
    
    # Add dependencies
    add_dependencies(${PROJECT_NAME} generate_version_header generate_funccode_define)
    
    # Set target properties
    set_target_properties(${PROJECT_NAME} PROPERTIES
        LINKER_LANGUAGE "CXX"
    )
    
    # MSVC specific settings
    if(MSVC)
        # Convert decimal LOCALE_ID to hexadecimal for MSVC RC
        math(EXPR LOCALE_ID_HEX "${LOCALE_ID}" OUTPUT_FORMAT HEXADECIMAL)

        # Set RC flags for MSVC
        target_compile_options(${PROJECT_NAME} PRIVATE
            $<$<COMPILE_LANGUAGE:RC>:/c utf-8 /l ${LOCALE_ID_HEX}>
        )

        # avoid error LNK2001 for "__DllMainCRTStartup@12"
        set_target_properties(${PROJECT_NAME} PROPERTIES
            LINK_FLAGS "/NOENTRY"
        )
    endif(MSVC)
    
    # MinGW specific settings
    if(MINGW)
        # Set RC flags for MinGW (windres uses decimal)
        target_compile_options(${PROJECT_NAME} PRIVATE
            $<$<COMPILE_LANGUAGE:RC>:-c 65001 -l ${LOCALE_ID}>
        )

        # avoid prefixing of DLL name, set PREFIX to blank.
        # https://cmake.org/cmake/help/v3.12/prop_tgt/PREFIX.html?highlight=prefix
        set_target_properties(${PROJECT_NAME} PROPERTIES
            PREFIX ""
        )
    endif(MINGW)
endfunction()
