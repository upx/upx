cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

# compilation config options
option(UPX_CONFIG_DISABLE_GITREV   "Do not compile with default Git version info.")
option(UPX_CONFIG_DISABLE_SANITIZE "Do not compile with default sanitize options.")
option(UPX_CONFIG_DISABLE_WERROR   "Do not compile with default -Werror option.")
# test config options
option(UPX_CONFIG_DISABLE_SELF_PACK_TEST "Do not test packing UPX with itself") # see below

#***********************************************************************
# init
#***********************************************************************

# Disallow in-source builds. Note that you will still have to manually
# clean up a few files if you accidentally try an in-source build.
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
set(CMAKE_DISABLE_SOURCE_CHANGES  ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
if(",${CMAKE_SOURCE_DIR}," STREQUAL ",${CMAKE_BINARY_DIR},")
    message(FATAL_ERROR "ERROR: In-source builds are not allowed, please use an extra build dir.")
endif()

# determine git revision
set(GITREV_SHORT "")
set(GITREV_PLUS "")
set(GIT_DESCRIBE "")
find_package(Git)
if(GIT_FOUND AND NOT UPX_CONFIG_DISABLE_GITREV)
    execute_process(
        RESULT_VARIABLE result
        COMMAND "${GIT_EXECUTABLE}" rev-parse --short=12 HEAD
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
        OUTPUT_VARIABLE GITREV_SHORT
    )
    string(LENGTH "${GITREV_SHORT}" l)
    if(${result} EQUAL 0 AND ${l} EQUAL 12)
        execute_process(RESULT_VARIABLE result COMMAND "${GIT_EXECUTABLE}" diff --quiet)
        if(NOT ${result} EQUAL 0)
            set(GITREV_PLUS "+")
        endif()
    else()
        set(GITREV_SHORT "")
    endif()
    execute_process(
        RESULT_VARIABLE result
        COMMAND "${GIT_EXECUTABLE}" describe --match "v*.*.*" --tags --dirty
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
        OUTPUT_VARIABLE GIT_DESCRIBE
    )
    if(GIT_DESCRIBE MATCHES "^v?([0-9]+\\.[0-9]+\\.[0-9]+)-([0-9]+)-g(.+)$")
        set(GIT_DESCRIBE "${CMAKE_MATCH_1}-devel.${CMAKE_MATCH_2}+git-${CMAKE_MATCH_3}")
    endif()
endif()
if(NOT IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/.git") # extra check
    set(GITREV_SHORT "")
endif()
if(GITREV_SHORT)
    message(STATUS "UPX_VERSION_GITREV = \"${GITREV_SHORT}${GITREV_PLUS}\"")
    if(GIT_DESCRIBE)
        message(STATUS "UPX_VERSION_GIT_DESCRIBE = \"${GIT_DESCRIBE}\"")
    endif()
else()
    message(STATUS "UPX_VERSION_GITREV: not set")
endif()

project(upx VERSION 4.0.1 LANGUAGES C CXX)

# set default build type to "Release"
get_property(is_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(is_multi_config)
    set(c "${CMAKE_CONFIGURATION_TYPES}")
    list(INSERT c 0 "Release")
    list(INSERT c 1 "Debug")
    if (CMAKE_BUILD_TYPE)
        list(INSERT c 0 "${CMAKE_BUILD_TYPE}")
    endif()
    list(REMOVE_DUPLICATES c)
    set(CMAKE_CONFIGURATION_TYPES "${c}" CACHE STRING "List of supported configuration types." FORCE)
elseif(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
endif()

#***********************************************************************
# targets and compilation flags
#***********************************************************************

file(GLOB ucl_SOURCES "vendor/ucl/src/*.c")
list(SORT ucl_SOURCES)
add_library(upx_vendor_ucl STATIC ${ucl_SOURCES})
set_property(TARGET upx_vendor_ucl PROPERTY C_STANDARD 11)

file(GLOB zlib_SOURCES "vendor/zlib/*.c")
list(SORT zlib_SOURCES)
add_library(upx_vendor_zlib STATIC ${zlib_SOURCES})
set_property(TARGET upx_vendor_zlib PROPERTY C_STANDARD 11)

file(GLOB upx_SOURCES "src/*.cpp" "src/util/*.cpp")
list(SORT upx_SOURCES)
add_executable(upx ${upx_SOURCES})
set_property(TARGET upx PROPERTY CXX_STANDARD 14)
target_link_libraries(upx upx_vendor_ucl upx_vendor_zlib)

if(UPX_CONFIG_DISABLE_WERROR)
    set(warn_Werror "")
    set(warn_WX "")
else()
    set(warn_Werror -Werror)
    set(warn_WX -WX)
endif()

if(NOT MSVC)
    # use -O2 instead of -O3 to reduce code size
    string(REGEX REPLACE "(^| )-O3( |$$)" "\\1-O2\\2" a "${CMAKE_C_FLAGS_RELEASE}")
    string(REGEX REPLACE "(^| )-O3( |$$)" "\\1-O2\\2" b "${CMAKE_CXX_FLAGS_RELEASE}")
    set(CMAKE_C_FLAGS_RELEASE "${a}" CACHE STRING "Flags used by the C compiler during RELEASE builds." FORCE)
    set(CMAKE_CXX_FLAGS_RELEASE "${b}" CACHE STRING "Flags used by the CXX compiler during RELEASE builds." FORCE)
endif()

if(MSVC)
    # disable silly warnings about using "deprecated" POSIX functions like fopen()
    add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
    add_definitions(-D_CRT_NONSTDC_NO_WARNINGS)
    add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
else()
    # protect against security threats caused by misguided compiler "optimizations"
    if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
        add_definitions(-fno-delete-null-pointer-checks)
    endif()
    add_definitions(-fno-strict-aliasing -fno-strict-overflow -funsigned-char)
    # disable overambitious auto-vectorization until this actually gains something
    add_definitions(-fno-tree-vectorize)
endif()

set(t upx_vendor_ucl)
target_include_directories(${t} PRIVATE vendor/ucl/include vendor/ucl)
if(MSVC)
    target_compile_options(${t} PRIVATE -J -W4 ${warn_WX})
else()
    target_compile_options(${t} PRIVATE -Wall -Wextra -Wvla ${warn_Werror})
endif()

set(t upx_vendor_zlib)
if(MSVC)
    target_compile_options(${t} PRIVATE -DHAVE_STDARG_H -DHAVE_VSNPRINTF -J -W3 ${warn_WX})
else()
    target_compile_options(${t} PRIVATE -DHAVE_STDARG_H -DHAVE_UNISTD_H -DHAVE_VSNPRINTF)
    # clang-15: -Wno-strict-prototypes is needed to silence the new -Wdeprecated-non-prototype warning
    target_compile_options(${t} PRIVATE -Wall -Wextra -Wvla -Wno-strict-prototypes ${warn_Werror})
endif()

set(t upx)
target_include_directories(${t} PRIVATE vendor)
target_compile_definitions(${t} PRIVATE $<$<CONFIG:Debug>:DEBUG=1>)
if(GITREV_SHORT)
    target_compile_definitions(${t} PRIVATE UPX_VERSION_GITREV="${GITREV_SHORT}${GITREV_PLUS}")
    if(GIT_DESCRIBE)
        target_compile_definitions(${t} PRIVATE UPX_VERSION_GIT_DESCRIBE="${GIT_DESCRIBE}")
    endif()
endif()
if(NOT UPX_CONFIG_DISABLE_SANITIZE AND NOT MSVC)
    # default sanitizer for Debug builds
    target_compile_options(${t} PRIVATE $<$<CONFIG:Debug>:-fsanitize=undefined -fsanitize-undefined-trap-on-error -fstack-protector-all>)
    # default sanitizer for Release builds
    target_compile_options(${t} PRIVATE $<$<CONFIG:Release>:-fstack-protector>)
endif()
if(MSVC)
    target_compile_options(${t} PRIVATE -EHsc -J -W4 ${warn_WX})
else()
    target_compile_options(${t} PRIVATE
        -Wall -Wextra -Wcast-align -Wcast-qual -Wmissing-declarations -Wpointer-arith
        -Wshadow -Wvla -Wwrite-strings ${warn_Werror}
    )
endif()

#***********************************************************************
# "make test"
#***********************************************************************

include(CTest)
if(NOT CMAKE_CROSSCOMPILING)
    add_test(NAME upx-version COMMAND upx --version)
    add_test(NAME upx-help    COMMAND upx --help)
endif()
if(NOT CMAKE_CROSSCOMPILING AND NOT UPX_CONFIG_DISABLE_SELF_PACK_TEST)
    # NOTE: these tests can only work if the host executable format is supported by UPX
    set(exe ${CMAKE_EXECUTABLE_SUFFIX})
    set(upx_self_exe "$<TARGET_FILE:upx>")
    add_test(NAME upx-test-cleanup COMMAND rm -f ./upx.packed${exe} ./upx.unpacked${exe})
    add_test(NAME upx-self-pack    COMMAND upx -3 ${upx_self_exe} -o upx.packed${exe})
    add_test(NAME upx-list         COMMAND upx -l upx.packed${exe})
    add_test(NAME upx-fileinfo     COMMAND upx --fileinfo upx.packed${exe})
    add_test(NAME upx-test         COMMAND upx -t upx.packed${exe})
    add_test(NAME upx-unpack       COMMAND upx -d upx.packed${exe} -o upx.unpacked${exe})
    add_test(NAME upx-run-unpacked COMMAND ./upx.unpacked${exe} --version-short)
    add_test(NAME upx-run-packed   COMMAND ./upx.packed${exe} --version-short)
    # run these tests sequentially
    set_tests_properties(
        upx-test-cleanup upx-self-pack upx-list upx-fileinfo upx-test upx-unpack
        upx-run-unpacked upx-run-packed PROPERTIES RUN_SERIAL TRUE
    )
endif()

# TODO: could add some more explicit test programs; but then, the integrated
# "doctest" is quite convenient and already covers a number of basic
# checks - and more complex cases better should be handled by an
# external test suite.

#***********************************************************************
# "make install"
#***********************************************************************

# installation prefix and directories
if(NOT CMAKE_INSTALL_PREFIX)
    message(FATAL_ERROR "ERROR: CMAKE_INSTALL_PREFIX is not defined.")
endif()
include(GNUInstallDirs)

if(DEFINED CMAKE_INSTALL_FULL_BINDIR)
    install(TARGETS upx DESTINATION "${CMAKE_INSTALL_FULL_BINDIR}")
    install(FILES
        COPYING LICENSE NEWS README THANKS doc/upx-doc.html doc/upx-doc.txt
        DESTINATION "${CMAKE_INSTALL_FULL_DOCDIR}"
    )
    install(FILES doc/upx.1 DESTINATION "${CMAKE_INSTALL_FULL_MANDIR}/man1")
endif()

#***********************************************************************
# finally print some info about the build configuration
#***********************************************************************

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/maint/src/CMakeLists.extra.txt")
include("${CMAKE_CURRENT_SOURCE_DIR}/maint/src/CMakeLists.extra.txt")
endif()

function(print_var v)
    if(${v})
        message(STATUS "${v} = ${${v}}")
    endif()
endfunction()
print_var(CMAKE_BUILD_TYPE)
print_var(CMAKE_CONFIGURATION_TYPES)
print_var(CMAKE_INSTALL_PREFIX)
print_var(CMAKE_CROSSCOMPILING)
print_var(CMAKE_C_COMPILER_ID)
print_var(CMAKE_C_COMPILER_VERSION)
print_var(CMAKE_C_COMPILER_ARCHITECTURE_ID)
print_var(CMAKE_C_PLATFORM_ID)
print_var(CMAKE_C_COMPILER_ABI)
print_var(CMAKE_CXX_COMPILER_ID)
print_var(CMAKE_CXX_COMPILER_VERSION)
print_var(CMAKE_CXX_COMPILER_ARCHITECTURE_ID)
print_var(CMAKE_CXX_PLATFORM_ID)
print_var(CMAKE_CXX_COMPILER_ABI)
if (CMAKE_BUILD_TYPE AND NOT CMAKE_BUILD_TYPE MATCHES "^(Debug|Release)$")
    message(WARNING "WARNING: unsupported CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}; please use \"Debug\" or \"Release\"")
endif()

# vim:set ft=cmake ts=4 sw=4 tw=0 et:
