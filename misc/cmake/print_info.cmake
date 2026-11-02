#
# UPX "CMake" build file; see https://cmake.org/
# Copyright (C) Markus Franz Xaver Johannes Oberhumer
#

#***********************************************************************
# summary section
# print some info about the build configuration
#***********************************************************************

function(upx_print_info)
    get_property(PROPERTY_GENERATOR_IS_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
    get_property(PROPERTY_TARGET_SUPPORTS_SHARED_LIBS GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS)

    # generator
    upx_print_var(CMAKE_GENERATOR_TOOLSET CMAKE_GENERATOR_PLATFORM)
    upx_print_var(PROPERTY_GENERATOR_IS_MULTI_CONFIG)

    # directories
    if(NOT ",${CMAKE_BINARY_DIR}," STREQUAL ",${CMAKE_CURRENT_BINARY_DIR}," OR NOT ",${CMAKE_SOURCE_DIR}," STREQUAL ",${CMAKE_CURRENT_SOURCE_DIR},")
        upx_print_var(CMAKE_BINARY_DIR CMAKE_SOURCE_DIR CMAKE_CURRENT_BINARY_DIR CMAKE_CURRENT_SOURCE_DIR)
    endif()

    # system
    upx_print_var(CMAKE_HOST_SYSTEM_NAME CMAKE_HOST_SYSTEM_VERSION CMAKE_HOST_SYSTEM_PROCESSOR)
    upx_print_var(CMAKE_SYSTEM_NAME CMAKE_SYSTEM_VERSION CMAKE_SYSTEM_PROCESSOR)
    upx_print_var(CMAKE_ANDROID_NDK CMAKE_ANDROID_NDK_VERSION CMAKE_ANDROID_STANDALONE_TOOLCHAIN)
    upx_print_var(CMAKE_APPLE_SILICON_PROCESSOR CMAKE_OSX_DEPLOYMENT_TARGET CMAKE_OSX_SYSROOT)
    upx_print_var(CMAKE_VS_PLATFORM_TOOLSET CMAKE_VS_PLATFORM_TOOLSET_HOST_ARCHITECTURE CMAKE_VS_PLATFORM_TOOLSET_VERSION)
    upx_print_var(CMAKE_CROSSCOMPILING CMAKE_CROSSCOMPILING_EMULATOR)
    upx_print_var(CMAKE_SIZEOF_VOID_P)

    # binutils
    upx_print_var(CMAKE_EXECUTABLE_FORMAT CMAKE_EXECUTABLE_SUFFIX UPX_CONFIG_CMAKE_EXECUTABLE_SUFFIX)
    if(0)
        upx_print_var(CMAKE_AR CMAKE_RANLIB)
    else()
        upx_print_var(CMAKE_ADDR2LINE CMAKE_AR CMAKE_DLLTOOL CMAKE_LINKER CMAKE_NM CMAKE_OBJCOPY CMAKE_OBJDUMP CMAKE_RANLIB CMAKE_READELF CMAKE_SIZE CMAKE_STRIP CMAKE_TAPI CMAKE_WINDRES)
        upx_print_var(CMAKE_C_COMPILER_AR CMAKE_C_COMPILER_RANLIB CMAKE_CXX_COMPILER_AR CMAKE_CXX_COMPILER_RANLIB)
    endif()

    # compilers
    foreach(lang IN ITEMS ASM C CXX)
        upx_print_var(CMAKE_${lang}_COMPILER_LAUNCHER)
        upx_print_var(CMAKE_${lang}_COMPILER)
        upx_print_var(CMAKE_${lang}_COMPILER_ARG1)
        upx_print_var(CMAKE_${lang}_COMPILER_ID)
        upx_print_var(CMAKE_${lang}_SIMULATE_ID)
        upx_print_var(CMAKE_${lang}_COMPILER_VERSION)
        upx_print_var(CMAKE_${lang}_COMPILER_FRONTEND_VARIANT)
        upx_print_var(CMAKE_${lang}_COMPILER_ARCHITECTURE_ID)
        upx_print_var(CMAKE_${lang}_PLATFORM_ID)
        upx_print_var(CMAKE_${lang}_COMPILER_ABI)
        upx_print_var(CMAKE_${lang}_COMPILER_TARGET)
    endforeach()

    # install
    upx_print_var(CMAKE_INSTALL_PREFIX)
    upx_print_var(CMAKE_INSTALL_BINDIR)
    upx_print_var(CMAKE_INSTALL_LIBDIR)
    upx_print_var(CMAKE_INSTALL_DOCDIR)
    upx_print_var(CMAKE_INSTALL_MANDIR)

    # misc
    upx_print_var(CMAKE_BUILD_WITH_INSTALL_RPATH CMAKE_SKIP_RPATH CMAKE_SKIP_BUILD_RPATH CMAKE_SKIP_INSTALL_RPATH)
    upx_print_var(CMAKE_INTERPROCEDURAL_OPTIMIZATION CMAKE_POSITION_INDEPENDENT_CODE)
    upx_print_var(PROPERTY_TARGET_SUPPORTS_SHARED_LIBS)
    upx_print_var(UPX_CONFIG_SANITIZE_FLAGS_DEBUG UPX_CONFIG_SANITIZE_FLAGS_RELEASE)

    # shortcuts
    upx_print_var(AIX APPLE CLANG CYGWIN GNU_FRONTEND GNUC MINGW MSVC MSVC_FRONTEND MSVC_IDE MSVC_SIMULATE MSVC_TOOLSET_VERSION MSVC_VERSION MSYS UNIX WIN32 WIN64)
endfunction()

# vim:set ft=cmake ts=4 sw=4 tw=0 et:
