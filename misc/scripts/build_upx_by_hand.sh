#! /bin/sh
## vim:set ts=4 sw=4 et:
set -e

#
# build UPX "by hand", using a POSIX-compliant shell and
# a minimal number of compilation flags
# Copyright (C) Markus Franz Xaver Johannes Oberhumer
#

# uses optional environment variables: AR, CC, CXX, OPTIMIZE, top_srcdir

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
# protect against security threats caused by misguided compiler "optimizations"
mandatory_flags="-fno-strict-aliasing -fno-strict-overflow -funsigned-char"
if test "x$OPTIMIZE" != "x" && test "x$OPTIMIZE" != "x0"; then
    # not mandatory and not minimal, but usually a good idea:
    mandatory_flags="-Wall -O2 $mandatory_flags"
fi
CC="$CC $mandatory_flags"
CXX="$CXX $mandatory_flags"

# go to upx top-level directory
# HINT: set "top_srcdir" manually if your system does not have "readlink"
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
    shift; "$@"
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
run "+" mkdir -p "build/by-hand"
if check_submodule bzip2; then
    for f in "$rel_top_srcdir"/vendor/bzip2/*.c; do
        run "CC  $f" $CC -c "$f"
    done
fi
if check_submodule ucl; then
    for f in "$rel_top_srcdir"/vendor/ucl/src/*.c; do
        run "CC  $f" $CC -I"$rel_top_srcdir"/vendor/ucl/include -I"$rel_top_srcdir"/vendor/ucl -c "$f"
    done
fi
if check_submodule zlib; then
    for f in "$rel_top_srcdir"/vendor/zlib/*.c; do
        run "CC  $f" $CC -DHAVE_UNISTD_H -DHAVE_VSNPRINTF -c "$f"
    done
fi
if check_submodule zstd; then
    for f in "$rel_top_srcdir"/vendor/zstd/lib/*/*.c; do
        run "CC  $f" $CC -DDYNAMIC_BMI2=0 -DZSTD_DISABLE_ASM -c "$f"
    done
fi
run "+" cd "$rel_top_srcdir" || exit 1
rel_top_srcdir=.
echo "#==== build UPX ====="
run "+" cd "build/by-hand" || exit 1
rel_top_srcdir=../..
for f in "$rel_top_srcdir"/src/*.cpp "$rel_top_srcdir"/src/*/*.cpp; do
    run "CXX $f"     $CXX -I"$rel_top_srcdir"/vendor -c "$f"
done
# echo "#==== link UPX ====="
if test "x$AR" = "x"; then
    # link without using $AR
    run "CXX upx"    $CXX -o upx *.o */*.o
else
    run "AR  libupx" $AR rcs libupx_submodules.a */*.o
    run "CXX upx"    $CXX -o upx *.o -L. -lupx_submodules
fi
echo "# current directory: '$(pwd)'"
ls -l upx*

echo "# All done."
