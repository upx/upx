#! /bin/sh
## vim:set ts=4 sw=4 et:
set -e

#
# build UPX "by hand", using POSIX shell and a minimal number of compilation flags
# Copyright (C) Markus Franz Xaver Johannes Oberhumer
#

# shell init
### set -x # enable logging
DUALCASE=1; export DUALCASE # for MKS sh
test -n "${ZSH_VERSION+set}" && emulate sh # for zsh
argv0="$0"; argv0abs="$(readlink -fn "$argv0")"; argv0dir="$(dirname "$argv0abs")"
# HINT: set "argv0dir" manually if your system does not have "readlink"

# toolchain settings and flags
CC="${CC:-cc}"
CXX="${CXX:-c++ -std=gnu++17}"
AR="${AR:-ar}"
# HINT: use "export AR=false" if "$AR rcs" does not work on your system; see below
if test "x$AR" = "x0" || test "x$AR" = "xfalse" || test "x$AR" = "x/bin/false"; then
    AR="" # do not use $AR
fi
# protect against security threats caused by misguided compiler "optimizations"
mandatory_flags="-fno-strict-aliasing -fno-strict-overflow -funsigned-char"
# not mandatory and not minimal, but usually a good idea:
### mandatory_flags="-Wall -O2 $mandatory_flags"
CC="$CC $mandatory_flags"
CXX="$CXX $mandatory_flags"

# go to upx top-level directory
cd "$argv0dir/../.." || exit 1
pwd
test -f src/version.h || exit 1 # sanity check
top_srcdir="$PWD"
rm -rf ./build/by-hand          # WARNING: existing build-directory gets deleted!
mkdir -p ./build/by-hand

# helper function
check_submodule() {
    local f
    for f in COPYING LICENSE LICENSE.txt; do
        if test -f "$top_srcdir/vendor/$1/$f"; then
            # create and enter build directory
            mkdir -p "$top_srcdir/build/by-hand/$1"
            cd "$top_srcdir/build/by-hand/$1" || exit 1
            echo "===== build $1 ====="
            return 0
        fi
    done
    return 1
}

# build
if check_submodule bzip2; then
    for f in "$top_srcdir"/vendor/bzip2/*.c; do
        echo "CC  $f"
        $CC -c "$f"
    done
fi
if check_submodule ucl; then
    for f in "$top_srcdir"/vendor/ucl/src/*.c; do
        echo "CC  $f"
        $CC -I"$top_srcdir"/vendor/ucl/include -I"$top_srcdir"/vendor/ucl -c "$f"
    done
fi
if check_submodule zlib; then
    for f in "$top_srcdir"/vendor/zlib/*.c; do
        echo "CC  $f"
        $CC -DHAVE_STDARG_H -DHAVE_VSNPRINTF -DHAVE_UNISTD_H -c "$f"
    done
fi
if check_submodule zstd; then
    for f in "$top_srcdir"/vendor/zstd/lib/*/*.c; do
        echo "CC  $f"
        $CC -DDYNAMIC_BMI2=0 -DZSTD_DISABLE_ASM -c "$f"
    done
fi
echo "===== build UPX ====="
cd "$top_srcdir"/build/by-hand || exit 1
for f in "$top_srcdir"/src/*.cpp "$top_srcdir"/src/*/*.cpp; do
    echo "CXX $f"
    $CXX -I"$top_srcdir"/vendor -c "$f"
done
# echo "===== link UPX ====="
if test "x$AR" = "x"; then
    # link without using $AR
    echo "CXX upx"
    $CXX -o upx *.o */*.o
else
    echo "AR  libupx"
    $AR rcs libupx_submodules.a */*.o
    echo "CXX upx"
    $CXX -o upx *.o -L. -lupx_submodules
fi
pwd
ls -l upx*

echo "All done."
