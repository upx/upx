#! /bin/bash
## vim:set ts=4 sw=4 et:
set -e; set -o pipefail

# Copyright (C) Markus Franz Xaver Johannes Oberhumer

# ...lots of outdated/unneeded stuff from the old Travis/Circle/AppVeyor CI days...

if [[ $TRAVIS_OS_NAME == osx ]]; then
    argv0=$0; argv0abs=$(greadlink -en -- "$0"); argv0dir=$(dirname "$argv0abs")
else
    argv0=$0; argv0abs=$(readlink -en -- "$0"); argv0dir=$(dirname "$argv0abs")
fi
source "$argv0dir/travis_init.sh" || exit 1

# create dirs
cd / || exit 1
mkbuilddirs $upx_BUILDDIR $ucl_BUILDDIR $upx_testsuite_BUILDDIR $zlib_BUILDDIR
cd / && cd "$upx_SRCDIR" || exit 1

echo
print_settings
echo
echo "$CC --version"; $CC --version
echo "$CXX --version"; $CXX --version
echo

# check whitespace
if [[ $TRAVIS_OS_NAME == linux ]]; then
    cd / && cd $upx_SRCDIR || exit 1
    echo "Checking source code for whitespace violations..."
    bash ./misc/scripts/check_whitespace.sh || exit 1
    echo "  Passed."
fi # linux

set -x

#
# rebuild UPX stubs (needs upx-stubtools)
#

if [[ $BM_X =~ (^|\+)rebuild-stubs($|\+) ]]; then
    if [[ -f "$HOME/local/bin/bin-upx/upx-stubtools-check-version" ]]; then
        bin_upx=$(readlink -en -- "$HOME/local/bin/bin-upx")
    elif [[ -f "$HOME/.local/bin/bin-upx/upx-stubtools-check-version" ]]; then
        bin_upx=$(readlink -en -- "$HOME/.local/bin/bin-upx")
    else
        bin_upx=$(readlink -en -- "$upx_SRCDIR/../deps/bin-upx-20210104")
    fi
    cd / && cd "$upx_SRCDIR" || exit 1
    extra_subdirs=()
    extra_subdirs+=( src/stub/src/arch/amd64 )
    extra_subdirs+=( src/stub/src/arch/arm/v4a )
    extra_subdirs+=( src/stub/src/arch/arm/v4t )
    extra_subdirs+=( src/stub/src/arch/arm64/v8 )
    extra_subdirs+=( src/stub/src/arch/i386 )
    extra_subdirs+=( src/stub/src/arch/m68k/m68000 )
    extra_subdirs+=( src/stub/src/arch/m68k/m68020 )
    extra_subdirs+=( src/stub/src/arch/mips/r3000 )
    extra_subdirs+=( src/stub/src/arch/powerpc/32 )
    extra_subdirs+=( src/stub/src/arch/powerpc/64 )
    extra_subdirs+=( src/stub/src/arch/powerpc/64le )
    make -C src/stub maintainer-clean
    for d in ${extra_subdirs[@]}; do
        make -C $d -f Makefile.extra maintainer-clean
        git status $d || true
    done
    git status src/stub/src || true
    git status || true
    failed=0
    for d in ${extra_subdirs[@]}; do
        PATH="$bin_upx:$PATH" make -C $d -f Makefile.extra || failed=1
    done
    PATH="$bin_upx:$PATH" make -C src/stub all || failed=1
    if [[ $failed != 0 ]]; then
        echo "UPX-ERROR: FATAL: rebuild-stubs failed"
        exit 1
    fi
    if ! git diff --quiet; then
        git status || true
        git diff || true
        echo "UPX-ERROR: FATAL: rebuild-stubs git status mismatch. See log file."
        exit 1
    fi
    git status
    if [[ $BM_X == rebuild-stubs ]]; then
        echo "X=rebuild-stubs done. Exiting."
        exit 0
    fi
    unset bin_upx extra_subdirs d failed
fi

exit 1
