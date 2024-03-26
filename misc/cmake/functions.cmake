#
# UPX "CMake" build file; see https://cmake.org/
# Copyright (C) Markus Franz Xaver Johannes Oberhumer
#

#***********************************************************************
# macros
#***********************************************************************

# support config hooks; developer convenience
macro(upx_cmake_include_hook section)
    include("${CMAKE_CURRENT_SOURCE_DIR}/misc/cmake/hooks/CMakeLists.${section}.txt" OPTIONAL)
    include("${CMAKE_CURRENT_SOURCE_DIR}/maint/make/CMakeLists.${section}.txt" OPTIONAL)
endmacro()

# Disallow in-source build. Note that you will still have to manually
# clean up a few files if you accidentally try an in-source build.
macro(upx_disallow_in_source_build)
    set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
    set(CMAKE_DISABLE_SOURCE_CHANGES  ON)
    if(",${CMAKE_CURRENT_SOURCE_DIR}," STREQUAL ",${CMAKE_CURRENT_BINARY_DIR},")
        # try to clean up (does not work)
        #file(REMOVE "${CMAKE_CURRENT_BINARY_DIR}/CMakeCache.txt")
        #file(REMOVE "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/cmake.check_cache")
        message(FATAL_ERROR "ERROR: In-source builds are not allowed, please use an extra build dir.")
    endif()
endmacro()

# set the default build type; must be called before project() cmake init
macro(upx_set_default_build_type type)
    set(upx_global_default_build_type "${type}")
    get_property(upx_global_is_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
    if(NOT upx_global_is_multi_config AND NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE "${upx_global_default_build_type}" CACHE STRING "Choose the type of build." FORCE)
    endif()
endmacro()

# set the default multi-config build type; must be called after project() cmake init
macro(upx_apply_build_type)
    if(upx_global_is_multi_config)
        set(c "${CMAKE_CONFIGURATION_TYPES}")
        list(INSERT c 0 "${upx_global_default_build_type}")
        if(CMAKE_BUILD_TYPE)
            list(INSERT c 0 "${CMAKE_BUILD_TYPE}")
        endif()
        list(REMOVE_DUPLICATES c)
        set(CMAKE_CONFIGURATION_TYPES "${c}" CACHE STRING "List of supported configuration types." FORCE)
    endif()
    # now also set the build type for use in try_compile()
    if(NOT CMAKE_TRY_COMPILE_CONFIGURATION)
        if(CMAKE_BUILD_TYPE)
            set(CMAKE_TRY_COMPILE_CONFIGURATION "${CMAKE_BUILD_TYPE}")
        else()
            set(CMAKE_TRY_COMPILE_CONFIGURATION "${upx_global_default_build_type}")
        endif()
    endif()
endmacro()

#***********************************************************************
# util
#***********************************************************************

function(upx_print_var) # ARGV
    foreach(var_name ${ARGV})
        if(DEFINED ${var_name})
            if(NOT ",${${var_name}}," STREQUAL ",,")
                if(${var_name})
                    message(STATUS "${var_name} = ${${var_name}}")
                endif()
            endif()
        endif()
    endforeach()
endfunction()

function(upx_print_env_var) # ARGV
    foreach(var_name ${ARGV})
        if(DEFINED ENV{${var_name}})
            if(NOT ",$ENV{${var_name}}," STREQUAL ",,")
                message(STATUS "ENV{${var_name}} = $ENV{${var_name}}")
            endif()
        endif()
    endforeach()
endfunction()

function(upx_print_have_symbol) # ARGV; needs include(CheckSymbolExists)
    foreach(symbol ${ARGV})
        set(cache_var_name "HAVE_symbol_${symbol}")
        check_symbol_exists(${symbol} "limits.h;stddef.h;stdint.h" ${cache_var_name})
        if(${cache_var_name})
            message(STATUS "HAVE ${symbol}")
        endif()
    endforeach()
endfunction()

# examine MinGW/Cygwin compiler configuration
function(upx_print_mingw_symbols)
    if(WIN32 OR MINGW OR CYGWIN)
        if(CMAKE_C_COMPILER_ID MATCHES "(Clang|GNU)")
            # runtime library: msvcrt vs ucrt vs cygwin
            upx_print_have_symbol(__CRTDLL__ __CYGWIN__ __CYGWIN32__ __CYGWIN64__ __MINGW32__ __MINGW64__ __MINGW64_VERSION_MAJOR __MSVCRT__ __MSVCRT_VERSION__ _UCRT _WIN32 _WIN64)
            # exception handing: SJLJ (setjmp/longjmp) vs DWARF vs SEH
            upx_print_have_symbol(__GCC_HAVE_DWARF2_CFI_ASM __SEH__ __USING_SJLJ_EXCEPTIONS__)
            # threads: win32 vs posix/pthread/winpthreads vs mcfgthread
            upx_print_have_symbol(_REENTRANT __USING_MCFGTHREAD__)
        endif()
    endif()
endfunction()

# add wildcard expansions to a variable
function(upx_add_glob_files) # ARGV
    set(var_name ${ARGV0})
    list(REMOVE_AT ARGV 0)
    file(GLOB files ${ARGV})
    set(result "")
    if(DEFINED ${var_name})
        set(result "${${var_name}}")
    endif()
    list(APPEND result "${files}")
    list(SORT result)
    list(REMOVE_DUPLICATES result)
    #message(STATUS "upx_add_glob_files: ${var_name} = ${result}")
    set(${var_name} "${result}" PARENT_SCOPE) # return value
endfunction()

# remove wildcard expansions from a variable
function(upx_remove_glob_files) # ARGV
    set(var_name ${ARGV0})
    list(REMOVE_AT ARGV 0)
    file(GLOB files ${ARGV})
    set(result "")
    if(DEFINED ${var_name})
        set(result "${${var_name}}")
        foreach(file ${files})
            list(REMOVE_ITEM result "${file}")
        endforeach()
        list(SORT result)
        list(REMOVE_DUPLICATES result)
    endif()
    #message(STATUS "upx_remove_glob_files: ${var_name} = ${result}")
    set(${var_name} "${result}" PARENT_SCOPE) # return value
endfunction()

# useful for CI jobs: allow settings via environment and cache result
function(upx_cache_bool_vars) # ARGV
    set(default_value "${ARGV0}")
    list(REMOVE_AT ARGV 0)
    foreach(var_name ${ARGV})
        set(value ${default_value})
        if(DEFINED UPX_CACHE_VALUE_${var_name})     # check cache
            set(value "${UPX_CACHE_VALUE_${var_name}}")
        elseif(DEFINED ${var_name})                 # defined via "cmake -DXXX=YYY"
            set(value "${${var_name}}")
        elseif(DEFINED ENV{${var_name}})            # check environment
            if("$ENV{${var_name}}" MATCHES "^(0|1|OFF|ON|FALSE|TRUE)$")
                set(value "$ENV{${var_name}}")
                set(UPX_CACHE_ORIGIN_FROM_ENV_${var_name} TRUE CACHE INTERNAL "" FORCE) # for status message below
            endif()
        endif()
        # convert to bool
        if(value)
            set(value ON)
        else()
            set(value OFF)
        endif()
        # store result
        if(UPX_CACHE_ORIGIN_FROM_ENV_${var_name})
            message(STATUS "setting from environment: ${var_name} = ${value}")
        endif()
        set(${var_name} "${value}" PARENT_SCOPE) # store result
        set(UPX_CACHE_VALUE_${var_name} "${value}" CACHE INTERNAL "" FORCE) # and store in cache
    endforeach()
endfunction()

function(upx_platform_check_mismatch var_name_1 var_name_2)
    if(DEFINED ${var_name_1} OR DEFINED ${var_name_2})
        if(NOT ",${${var_name_1}}," STREQUAL ",${${var_name_2}},")
            if(UPX_CONFIG_DISABLE_WERROR)
                message(WARNING "WARNING: '${var_name_1}' '${var_name_2}' mismatch")
            else()
                message(FATAL_ERROR "FATAL ERROR: '${var_name_1}' '${var_name_2}' mismatch")
            endif()
        endif()
    endif()
endfunction()

#***********************************************************************
# compilation flags
#***********************************************************************

function(upx_internal_add_definitions_with_prefix) # ARGV; needs include(CheckCCompilerFlag)
    set(flag_prefix "${ARGV0}")
    if(flag_prefix MATCHES "^empty$") # need "empty" to work around bug in old CMake versions
        set(flag_prefix "")
    endif()
    list(REMOVE_AT ARGV 0)
    set(failed "")
    foreach(f ${ARGV})
        set(flag "${flag_prefix}${f}")
        string(REGEX REPLACE "[^0-9a-zA-Z_]" "_" cache_var_name "HAVE_CFLAG_${flag}")
        check_c_compiler_flag("${flag}" ${cache_var_name})
        if(${cache_var_name})
            #message(STATUS "add_definitions: ${flag}")
            add_definitions("${flag}")
        else()
            list(APPEND failed "${f}")
        endif()
    endforeach()
    set(failed_flags "${failed}" PARENT_SCOPE) # return value
endfunction()

function(upx_add_definitions) # ARGV; needs include(CheckCCompilerFlag)
    set(failed_flags "")
    if(MSVC_FRONTEND AND CMAKE_C_COMPILER_ID MATCHES "Clang")
        # for clang-cl try "-clang:" flag prefix first
        upx_internal_add_definitions_with_prefix("-clang:" ${ARGV})
        upx_internal_add_definitions_with_prefix("empty" ${failed_flags})
    else()
        upx_internal_add_definitions_with_prefix("empty" ${ARGV})
    endif()
endfunction()

# useful for CI jobs: allow target extra compile options
function(upx_add_target_extra_compile_options) # ARGV
    set(t "${ARGV0}")
    list(REMOVE_AT ARGV 0)
    foreach(var_name ${ARGV})
        if(NOT DEFINED ${var_name})
        elseif(",${${var_name}}," STREQUAL ",,")
        else()
            set(flags "${${var_name}}")
            if(NOT flags MATCHES ";") # NOTE: split into list from string only if not already a list
                if(${CMAKE_VERSION} VERSION_GREATER "3.8.99")
                    separate_arguments(flags NATIVE_COMMAND "${flags}")
                else()
                    separate_arguments(flags)
                endif()
            endif()
            target_compile_options(${t} PRIVATE "${flags}")
        endif()
    endforeach()
endfunction()

# compile a target with -O2 optimization even in Debug build
function(upx_compile_target_debug_with_O2) # ARGV
    foreach(t ${ARGV})
        if(MSVC_FRONTEND)
            # MSVC uses some Debug compilation options like -RTC1 that are incompatible with -O2
        else()
            target_compile_options(${t} PRIVATE $<$<CONFIG:Debug>:-O2>)
        endif()
    endforeach()
endfunction()

# compile a source file with -O2 optimization even in Debug build; messy because of CMake limitations
function(upx_compile_source_debug_with_O2) # ARGV
    set(flags "$<$<CONFIG:Debug>:-O2>")
    if(${CMAKE_VERSION} VERSION_LESS "3.8")
        # 3.8: The COMPILE_FLAGS source file property learned to support generator expressions
        if(upx_global_is_multi_config OR NOT CMAKE_BUILD_TYPE MATCHES "^Debug$")
            return()
        endif()
        set(flags "-O2")
    endif()
    if(CMAKE_GENERATOR MATCHES "Xcode") # multi-config
        # NOTE: Xcode does not support per-config per-source COMPILE_FLAGS (as of CMake 3.27.7)
        return()
    endif()
    foreach(source ${ARGV})
        if(MSVC_FRONTEND)
            # MSVC uses some Debug compilation options like -RTC1 that are incompatible with -O2
        else()
            get_source_file_property(prop "${source}" COMPILE_FLAGS)
            if(prop MATCHES "^(NOTFOUND)?$")
                set_source_files_properties("${source}" PROPERTIES COMPILE_FLAGS "${flags}")
            else()
                set_source_files_properties("${source}" PROPERTIES COMPILE_FLAGS "${prop} ${flags}")
            endif()
        endif()
    endforeach()
endfunction()

# sanitize a target; note that this may require special run-time support libs from your toolchain
function(upx_sanitize_target) # ARGV
    foreach(t ${ARGV})
        if(UPX_CONFIG_DISABLE_SANITIZE)
            # no-op
        elseif(MSVC_FRONTEND)
            # MSVC uses -GS (similar to -fstack-protector) by default
        elseif(MINGW OR CYGWIN)
            # avoid link errors with current MinGW-w64 versions
            # see https://www.mingw-w64.org/contribute/#sanitizers-asan-tsan-usan
        elseif(CMAKE_C_COMPILER_ID MATCHES "^GNU" AND CMAKE_C_COMPILER_VERSION VERSION_LESS "8.0")
            # unsupported compiler; unreliable/broken sanitize implementation
        else()
            # default sanitizer for Debug builds
            target_compile_options(${t} PRIVATE $<$<CONFIG:Debug>:-fsanitize=undefined -fsanitize-undefined-trap-on-error -fstack-protector-all>)
            # default sanitizer for Release builds
            target_compile_options(${t} PRIVATE $<$<CONFIG:MinSizeRel>:-fstack-protector>)
            target_compile_options(${t} PRIVATE $<$<CONFIG:Release>:-fstack-protector>)
            target_compile_options(${t} PRIVATE $<$<CONFIG:RelWithDebInfo>:-fstack-protector>)
        endif()
    endforeach()
endfunction()

#***********************************************************************
# test
#***********************************************************************

function(upx_add_serial_test) # ARGV
    set(name "${ARGV0}")
    list(REMOVE_AT ARGV 0)
    add_test(NAME "${name}" COMMAND ${ARGV})
    set_tests_properties("${name}" PROPERTIES RUN_SERIAL TRUE) # run these tests sequentially
endfunction()

# vim:set ft=cmake ts=4 sw=4 tw=0 et:
