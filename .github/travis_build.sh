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

# whitespace
if [[ $TRAVIS_OS_NAME == linux ]]; then
cd / && cd $upx_SRCDIR || exit 1
echo "Checking source code for whitespace violations..."
find . \
    -type d -name '.git' -prune -o \
    -type d -name '.hg' -prune -o \
    -type d -name 'build' -prune -o \
    -type d -name 'tmp' -prune -o \
    -type f -iname '*.bat' -prune -o \
    -type f -iname '*.exe' -prune -o \
    -type f -iname '*.pdf' -prune -o \
    -type f -print0 | LC_ALL=C sort -z | \
xargs -0r perl -n -e '
    if (m,[\r\x1a],) { print "ERROR: DOS EOL detected $ARGV: $_"; exit(1); }
    if (m,([ \t]+)$,) {
        # allow exactly two trailing spaces for GitHub flavoured Markdown in .md files
        if ($1 ne "  " || $ARGV !~ m,\.md$,) {
            print "ERROR: trailing whitespace detected $ARGV: $_"; exit(1);
        }
    }
    if (m,\t,) {
       if ($ARGV =~ m,(^|/)\.gitmodules$,) { }
       elsif ($ARGV =~ m,(^|/)(gnu|m)?make(file|vars),i) { }
       elsif ($ARGV =~ m,/tmp/.*\.(disasm|dump)$,) { }
       elsif ($ARGV =~ m,/src/stub/src/arch/.*\.S$,) { }
       else { print "ERROR: hard TAB detected $ARGV: $_"; exit(1); }
    }
' || exit 1
echo "  Passed."
fi # linux

set -x

#
# build UCL
#

cd / && cd $ucl_BUILDDIR || exit 1
if [[ -n $BM_CROSS || $TRAVIS_OS_NAME == windows ]]; then
    # configure is too old
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

if [[ $BUILD_LOCAL_ZLIB ]]; then
    cd / && cd $zlib_BUILDDIR || exit 1
    rm -f ./*.o libz.a
    $CC -O2 -c $zlib_SRCDIR/*.c
    $AR rcs libz.a *.o
fi # BUILD_LOCAL_ZLIB

#
# build UPX
#

export UPX_UCLDIR="$ucl_SRCDIR"
cd / && cd $upx_BUILDDIR || exit 1
make="make -f $upx_SRCDIR/src/Makefile"
EXTRA_CPPFLAGS="$EXTRA_CPPFLAGS -DUCL_NO_ASM"
EXTRA_LDFLAGS="$EXTRA_LDFLAGS -L$ucl_BUILDDIR/src/.libs"
if [[ $BUILD_LOCAL_ZLIB ]]; then
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

$make

[[ -z $exeext ]] && exeext=.out
ls -l upx${exeext}
$SIZE upx${exeext} || true
file upx${exeext}

exit 0
