#! /bin/sh
## vim:set ts=4 sw=4 et:
set -e

#
# build UPX "by hand", using a POSIX-compliant shell and
# a minimal number of compilation flags
# Copyright (C) Markus Franz Xaver Johannes Oberhumer
#

# uses optional environment variables: AR, CC, CXX, OPTIMIZE, VERBOSE
#  and optional settings for top_srcdir, obj_suffix and XXX_extra_flags

# shell init
### set -x # enable logging
DUALCASE=1; export DUALCASE # for MKS sh
test -n "${ZSH_VERSION+set}" && emulate sh # for zsh
my_argv0="$0"

# toolchain settings and compilation flags
AR="${AR:-ar}"
CC="${CC:-cc}"
CXX="${CXX:-c++ -std=gnu++17}"
# HINT: use "export AR=false" if "$AR rcs" does not work on your system; see below
if test "x$AR" = "x0" || test "x$AR" = "xfalse" || test "x$AR" = "x/bin/false"; then
    AR="" # do not use $AR
fi
if test -z "${mandatory_flags+set}"; then
    # protect against security threats caused by misguided compiler "optimizations"
    mandatory_flags="-fno-strict-aliasing -fno-strict-overflow -funsigned-char"
fi
if test -z "${sensible_flags+set}"; then
    # not mandatory but good practice when using <windows.h>:
    sensible_flags="-DWIN32_LEAN_AND_MEAN"
    if test "x$OPTIMIZE" != "x" && test "x$OPTIMIZE" != "x0"; then
        # not mandatory and not minimal, but usually a good idea:
        sensible_flags="-Wall -O2 $sensible_flags"
    fi
fi
CC="$CC $sensible_flags $mandatory_flags"
CXX="$CXX $sensible_flags $mandatory_flags"

# go to upx top-level directory
# HINT: set "top_srcdir" manually if your system does not have "readlink -f"
if test "x$top_srcdir" = "x"; then
    my_argv0abs="$(readlink -fn "$my_argv0")"
    test "x$my_argv0abs" = "x" && exit 1
    my_argv0dir="$(dirname "$my_argv0abs")"
    test "x$my_argv0dir" = "x" && exit 1
    cd "$my_argv0dir/../.." || exit 1
else
    cd "$top_srcdir" || exit 1
fi
top_srcdir="$(pwd)"             # absolute
rel_top_srcdir=.                # relative top_srcdir
echo "# current directory: '$(pwd)'"
test -f doc/upx.pod   || exit 1 # sanity check
test -f src/version.h || exit 1 # sanity check
rm -rf ./build/by-hand          # WARNING: existing build-directory gets deleted!

# helper function
run() {
    if test 0 = 1; then
        # DEBUG dry-run: print command, but don't actually run unless $1 is "+"
        if test "x$1" = "x+"; then
            shift; echo "$@"; "$@"; return
        fi
        shift; echo "$@"; return
    fi
    # print short info and run command
    test "x$1" != "x" && test "x$1" != "x+" && echo "$1"
    shift
    if test "x$VERBOSE" != "x" && test "x$VERBOSE" != "x0"; then
        # print full command
        echo "  $@"
    fi
    "$@"
}

# helper function
check_submodule() {
    #local ff # "local" seems unsupported by some versions of ksh
    for ff in COPYING LICENSE LICENSE.txt; do
        if test -f "$rel_top_srcdir/vendor/$1/$ff"; then
            # create and enter build directory; updates global $rel_top_srcdir
            run "+" cd "$rel_top_srcdir" || exit 1
            rel_top_srcdir=.
            echo "#==== build $1 ====="
            run "+" mkdir "build/by-hand/$1"
            run "+" cd "build/by-hand/$1" || exit 1
            rel_top_srcdir=../../..
            return 0
        fi
    done
    return 1
}

# build
upx_submodule_defs=
run "+" mkdir -p "build/by-hand"
if check_submodule bzip2; then
    upx_submodule_defs="$upx_submodule_defs -DWITH_BZIP2"
    test -z "${bzip2_extra_flags+set}" && bzip2_extra_flags="-DBZ_NO_STDIO"
    for f in "$rel_top_srcdir"/vendor/bzip2/*.c; do
        run "CC  $f" $CC $bzip2_extra_flags -c "$f"
    done
fi
if check_submodule ucl; then
    #upx_submodule_defs="$upx_submodule_defs -DWITH_UCL"
    test -z "${ucl_extra_flags+set}" && ucl_extra_flags=
    for f in "$rel_top_srcdir"/vendor/ucl/src/*.c; do
        run "CC  $f" $CC -I"$rel_top_srcdir"/vendor/ucl/include -I"$rel_top_srcdir"/vendor/ucl $ucl_extra_flags -c "$f"
    done
fi
if check_submodule zlib; then
    #upx_submodule_defs="$upx_submodule_defs -DWITH_ZLIB"
    test -z "${zlib_extra_flags+set}" && zlib_extra_flags="-DHAVE_UNISTD_H -DHAVE_VSNPRINTF"
    for f in "$rel_top_srcdir"/vendor/zlib/*.c; do
        run "CC  $f" $CC $zlib_extra_flags -c "$f"
    done
fi
if check_submodule zstd; then
    upx_submodule_defs="$upx_submodule_defs -DWITH_ZSTD"
    test -z "${zstd_extra_flags+set}" && zstd_extra_flags="-DDYNAMIC_BMI2=0 -DZSTD_DISABLE_ASM"
    for f in "$rel_top_srcdir"/vendor/zstd/lib/*/*.c; do
        run "CC  $f" $CC $zstd_extra_flags -c "$f"
    done
fi
run "+" cd "$rel_top_srcdir" || exit 1
rel_top_srcdir=.
echo "#==== build UPX ====="
run "+" cd "build/by-hand" || exit 1
rel_top_srcdir=../..
for f in "$rel_top_srcdir"/src/*.cpp "$rel_top_srcdir"/src/*/*.cpp; do
    run "CXX $f"     $CXX -I"$rel_top_srcdir"/vendor $upx_submodule_defs -c "$f"
done
# echo "#==== link UPX ====="
test "x$obj_suffix" = "x" && obj_suffix=.o
if test "x$AR" = "x"; then
    # link without using $AR
    run "CXX upx"    $CXX -o upx *${obj_suffix} */*${obj_suffix}
else
    run "AR  libupx" $AR rcs ${AR_LIBFILE:-libupx_submodules.a} */*${obj_suffix}
    run "CXX upx"    $CXX -o upx *${obj_suffix} -L. -lupx_submodules
fi
echo "# current directory: '$(pwd)'"
ls -l upx*

echo "# All done."
