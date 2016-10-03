#! /bin/bash
## vim:set ts=4 sw=4 et:
set -e; set -o pipefail

# Support for Travis CI -- https://travis-ci.org/upx/upx/builds
# Copyright (C) Markus Franz Xaver Johannes Oberhumer

if [[ $TRAVIS_OS_NAME == osx ]]; then
argv0="$0"; argv0dir=$(greadlink -en -- "$0"); argv0dir=$(dirname "$argv0dir")
else
argv0="$0"; argv0dir=$(readlink -en -- "$0"); argv0dir=$(dirname "$argv0dir")
fi
source "$argv0dir/travis_init.sh" || exit 1

# create dirs
cd / || exit 1
mkbuilddirs $upx_BUILDDIR $ucl_BUILDDIR $upx_testsuite_BUILDDIR $zlib_BUILDDIR
cd / && cd $upx_SRCDIR || exit 1

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
bash ./src/stub/scripts/check_whitespace.sh || exit 1
echo "  Passed."
fi # linux

set -x

#
# build UCL
#

cd / && cd $ucl_BUILDDIR || exit 1
if [[ -n $BM_CROSS || $TRAVIS_OS_NAME == windows ]]; then
    # ucl-1.03/configure is too old - build manually
    rm -f ./*.o libucl.a
    $CC -O2 -I$ucl_SRCDIR/include -I$ucl_SRCDIR -c $ucl_SRCDIR/src/*.c
    $AR rcs libucl.a *.o
    mkdir -p src/.libs
    cp libucl.a src/.libs
else
    if [[ ! -f config.status ]]; then
        $ucl_SRCDIR/configure --enable-static --disable-shared --disable-asm
    fi
    make
fi

#
# build zlib
#

if [[ $BUILD_LOCAL_ZLIB == 1 ]]; then
    cd / && cd $zlib_BUILDDIR || exit 1
    # build manually
    rm -f ./*.o libz.a
    $CC -O2 -c $zlib_SRCDIR/*.c
    $AR rcs libz.a *.o
fi

#
# build UPX
#

export UPX_UCLDIR="$ucl_SRCDIR"
cd / && cd $upx_BUILDDIR || exit 1
make="make -f $upx_SRCDIR/src/Makefile"
EXTRA_CPPFLAGS="$EXTRA_CPPFLAGS -DUCL_NO_ASM"
EXTRA_LDFLAGS="$EXTRA_LDFLAGS -L$ucl_BUILDDIR/src/.libs"
if [[ $BUILD_LOCAL_ZLIB == 1 ]]; then
    EXTRA_CPPFLAGS="$EXTRA_CPPFLAGS -I$zlib_SRCDIR"
    EXTRA_LDFLAGS="$EXTRA_LDFLAGS -L$zlib_BUILDDIR"
fi
if [[ $BM_B =~ (^|\+)coverage($|\+) ]]; then
    EXTRA_CXXFLAGS="$EXTRA_CXXFLAGS -fprofile-arcs -ftest-coverage"
    EXTRA_LDFLAGS="$EXTRA_LDFLAGS -fprofile-arcs -ftest-coverage"
fi
if [[ $BM_B =~ (^|\+)debug($|\+) ]]; then
    make="$make BUILD_TYPE_DEBUG=1"
fi
if [[ $BM_B =~ (^|\+)sanitize($|\+) ]]; then
    case $TRAVIS_OS_NAME-$CC in linux-gcc*) EXTRA_LDFLAGS="$EXTRA_LDFLAGS -fuse-ld=gold" ;; esac
    make="$make BUILD_TYPE_SANITIZE=1"
fi
if [[ $BM_B =~ (^|\+)scan-build($|\+) ]]; then
    make="$SCAN_BUILD $make"
fi
if [[ $BM_B =~ (^|\+)valgrind($|\+) ]]; then
    EXTRA_CPPFLAGS="$EXTRA_CPPFLAGS -DWITH_VALGRIND"
fi
if [[ $BM_B =~ (^|\+)ALLOW_FAIL($|\+) ]]; then
    echo "ALLOW_FAIL"
    set +e
fi
export EXTRA_CPPFLAGS EXTRA_CXXFLAGS EXTRA_LDFLAGS

[[ -z $upx_exeext ]] && upx_exeext=.out
$make exeext=$upx_exeext

ls -l upx${upx_exeext}
$SIZE upx${upx_exeext} || true
file upx${upx_exeext}

exit 0
