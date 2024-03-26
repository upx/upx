#! /usr/bin/env bash
## vim:set ts=4 sw=4 et:
set -e; set -o pipefail

# Copyright (C) Markus Franz Xaver Johannes Oberhumer
# assemble cmake config flags; useful for CI jobs
# also see misc/make/Makefile-extra.mk

cmake_config_flags=()

__add_cmake_config() {
    [[ -z "${!1}" ]] || cmake_config_flags+=( -D$1="${!1}" )
}

# pass common CMake settings from environment to cmake
for v in CMAKE_VERBOSE_MAKEFILE; do
    __add_cmake_config $v
done
# pass common CMake toolchain settings from environment to cmake
for v in CMAKE_ADDR2LINE CMAKE_AR CMAKE_DLLTOOL CMAKE_LINKER CMAKE_NM CMAKE_OBJCOPY CMAKE_OBJDUMP CMAKE_RANLIB CMAKE_READELF CMAKE_STRIP CMAKE_TAPI; do
    __add_cmake_config $v
done
# pass common CMake LTO toolchain settings from environment to cmake (for use with "-flto")
for v in CMAKE_C_COMPILER_AR CMAKE_C_COMPILER_RANLIB CMAKE_CXX_COMPILER_AR CMAKE_CXX_COMPILER_RANLIB; do
    __add_cmake_config $v
done
# pass common CMake cross compilation settings from environment to cmake
for v in CMAKE_SYSTEM_NAME CMAKE_SYSTEM_PROCESSOR CMAKE_CROSSCOMPILING_EMULATOR; do
    __add_cmake_config $v
done
# pass UPX config options from environment to cmake; see CMakeLists.txt
for v in UPX_CONFIG_DISABLE_GITREV UPX_CONFIG_DISABLE_SANITIZE UPX_CONFIG_DISABLE_WSTRICT UPX_CONFIG_DISABLE_WERROR UPX_CONFIG_DISABLE_SELF_PACK_TEST; do
    __add_cmake_config $v
done
# pass UPX extra compile options from environment to cmake; see CMakeLists.txt
for v in UPX_CONFIG_EXTRA_COMPILE_OPTIONS_BZIP2 UPX_CONFIG_EXTRA_COMPILE_OPTIONS_UCL UPX_CONFIG_EXTRA_COMPILE_OPTIONS_UPX UPX_CONFIG_EXTRA_COMPILE_OPTIONS_ZLIB UPX_CONFIG_EXTRA_COMPILE_OPTIONS_ZSTD; do
    __add_cmake_config $v
done

exec "${CMAKE:-cmake}" $UPX_CMAKE_CONFIG_FLAGS "${cmake_config_flags[@]}" "$@"
exit 99
