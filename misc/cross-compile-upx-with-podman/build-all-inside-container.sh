#! /usr/bin/env bash
## vim:set ts=4 sw=4 et:
set -e; set -o pipefail
argv0=$0; argv0abs="$(readlink -fn "$argv0")"; argv0dir="$(dirname "$argv0abs")"

# build UPX for 21 targets; must be run inside the container!
# using a rootless Podman container

# simple sanity check that we are indeed running inside the container
if [[ $UPX_CONTAINER_IMAGE_NAME != upx-cross-compile-* ]]; then
    echo "$argv0: NOTE: please run this script *inside* the container"
    exit 1
fi

# go to upx top-level directory
cd "$argv0dir/../.." || exit 1
pwd
[[ -f src/version.h ]] || exit 1 # sanity check

function run_config_and_build {
    # requires: AR CC CXX RANLIB
    local toolchain=$1
    local cmake_config_flags build_type bdir
    # cmake flags
    cmake_config_flags=
    case $toolchain in
        # these old architectures do not support sanitize
        alpha-linux-gnu) cmake_config_flags=-DUPX_CONFIG_DISABLE_SANITIZE=ON ;;
        hppa-linux-gnu) cmake_config_flags=-DUPX_CONFIG_DISABLE_SANITIZE=ON ;;
        # avoid link errors with current MinGW-w64 versions
        i686-w64-mingw32) cmake_config_flags=-DUPX_CONFIG_DISABLE_SANITIZE=ON ;;
        x86_64-w64-mingw32) cmake_config_flags=-DUPX_CONFIG_DISABLE_SANITIZE=ON ;;
        # avoid warnings about arm libstdc++ ABI change in gcc-7
        arm-linux-*) CXX="$CXX -Wno-psabi" ;;
    esac
    # for all build types
    for build_type in Debug Release; do
        bdir=build/cross-compile-upx-with-podman/$toolchain/${build_type,,}
        mkdir -p $bdir
        # run_config
        if [[ ! -f $bdir/CMakeCache.txt ]]; then
            cmake -S . -B $bdir -DCMAKE_BUILD_TYPE=$build_type -DCMAKE_AR=$AR -DCMAKE_RANLIB=$RANLIB $cmake_config_flags
        fi
        # run_build
        cmake --build $bdir --config $build_type --parallel
    done
}

# see Dockerfile for packages
for package in \
        g++-aarch64-linux-gnu \
        g++-alpha-linux-gnu \
        g++-arm-linux-gnueabi \
        g++-arm-linux-gnueabihf \
        g++-hppa-linux-gnu \
        g++-i686-linux-gnu \
        g++-m68k-linux-gnu \
        g++-mips-linux-gnu \
        g++-mipsel-linux-gnu \
        g++-mips64-linux-gnuabi64 \
        g++-mips64el-linux-gnuabi64 \
        g++-powerpc-linux-gnu \
        g++-powerpc64-linux-gnu \
        g++-powerpc64le-linux-gnu \
        g++-riscv64-linux-gnu \
        g++-s390x-linux-gnu \
        g++-sh4-linux-gnu \
        g++-sparc64-linux-gnu \
        g++-x86-64-linux-gnux32 \
        g++-mingw-w64-i686 \
        g++-mingw-w64-x86-64
do
    # setup toolchain and environment variables
    toolchain=$package
    toolchain=${toolchain/-x86-64/-x86_64}
    toolchain=${toolchain/g++-/}
    case $toolchain in
        mingw-w64-i686) toolchain=i686-w64-mingw32 ;;
        mingw-w64-x86_64) toolchain=x86_64-w64-mingw32 ;;
    esac
    echo "========== $toolchain"
    export AR=/usr/bin/${toolchain}-ar
    export CC=/usr/bin/${toolchain}-gcc
    export CXX=/usr/bin/${toolchain}-g++
    export RANLIB=/usr/bin/${toolchain}-ranlib
    ls -l $AR $CC $CXX $RANLIB
    run_config_and_build $toolchain
    unset AR CC CXX RANLIB
done
