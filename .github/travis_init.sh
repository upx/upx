## vim:set ts=4 sw=4 et:

# Support for Travis CI -- https://travis-ci.org/upx/upx/builds
# Copyright (C) Markus Franz Xaver Johannes Oberhumer

#set -x # debug
umask 022

# just in case
UPX=

# set CC and CXX from C
[[ -z $C ]] && C=gcc
if [[ -z $CC_OVERRIDE ]]; then
CC=false CXX=false SCAN_BUILD=false
case $C in
    clang | clang-m?? | clang-3.4-m?? | clang-[678][0-9][0-9]-m??)
        CC="clang"; CXX="clang++" ;; # standard system compiler
    clang-[34].[0-9]-m??)
        v=${C:6:3}; CC="clang-$v"; CXX="clang++-$v"; SCAN_BUILD="scan-build-$v" ;;
    gcc | gcc-m??)
        CC="gcc"; CXX="g++" ;; # standard system compiler
    gcc-[34].[0-9]-m??)
        v=${C:4:3}; CC="gcc-$v"; CXX="g++-$v" ;;
    gcc-[56]-m?? | gcc-[56].[0-9]-m??)
        v=${C:4:1}; CC="gcc-$v"; CXX="g++-$v" ;;
    msvc | msvc-*)
        CC="cl"; CXX="cl" ;; # standard system compiler
esac
case $C in
    msvc*) ;;
    clang*-m32) CC="$CC -m32"; CXX="$CXX -m32" ;;
    gcc*-m64)   CC="$CC -m64"; CXX="$CXX -m64" ;;
esac
case $C in
    clang* | gcc*) CC="$CC -std=gnu89" ;;
esac
export CC CXX
fi # CC_OVERRIDE

# B is BUILD_TYPE
[[ -z $B ]] && B=release

# dirs
[[ -z $upx_SRCDIR ]] && upx_SRCDIR="$PWD"
[[ -z $ucl_SRCDIR ]] && ucl_SRCDIR="$PWD/../deps/ucl-1.03"
[[ -z $upx_testsuite_SRCDIR ]] && upx_testsuite_SRCDIR="$PWD/../deps/upx-testsuite"

[[ -z $tmake_top_builddir ]] && tmake_top_builddir="$PWD/../build"
[[ -z $tmake_top_bdir ]] && tmake_top_bdir="$tmake_top_builddir/$C/$B"
upx_BUILDDIR="$tmake_top_bdir/upx"
ucl_BUILDDIR="$tmake_top_bdir/ucl-1.03"
upx_testsuite_BUILDDIR="$tmake_top_bdir/upx-testsuite"
# tools
lcov_OUTPUTDIR="$tmake_top_bdir/.lcov-results"

mkdir -p -v $upx_BUILDDIR $ucl_BUILDDIR $upx_testsuite_BUILDDIR
[[ -d $tmake_top_bdir/.mfxnobackup ]] && echo "timestamp" > $tmake_top_bdir/.mfxnobackup

export UPX_UCLDIR="$ucl_SRCDIR"

# check dirs
cd / && cd $ucl_BUILDDIR || exit 1
cd / && cd $ucl_SRCDIR || exit 1
cd / && cd $upx_testsuite_BUILDDIR || exit 1
cd / && cd $upx_testsuite_SRCDIR || exit 1
cd / && cd $upx_BUILDDIR || exit 1
# enter srcdir
cd / && cd $upx_SRCDIR || exit 1

if [[ -n $APPVEYOR_JOB_ID ]]; then
# for some reason this is needed for bash on AppVeyor
sort() {
    /usr/bin/sort "$@"
}
fi
