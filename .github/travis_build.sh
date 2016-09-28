#! /bin/bash
## vim:set ts=4 sw=4 et:
set -e; set -o pipefail

# Support for Travis CI -- https://travis-ci.org/upx/upx/builds
# Copyright (C) Markus Franz Xaver Johannes Oberhumer

source ./.github/travis_init.sh || exit 1

echo "B='$B'"
echo "UPX_UCLDIR='$UPX_UCLDIR'"
echo "CC='$CC'"
echo "CXX='$CXX'"
echo "CPPFLAGS='$CPPFLAGS'"
echo "CFLAGS='$CFLAGS'"
echo "CXXFLAGS='$CXXFLAGS'"
echo "LDFLAGS='$LDFLAGS'"
echo "LIBS='$LIBS'"
echo "SCAN_BUILD='$SCAN_BUILD'"
##env | LC_ALL=C sort

echo "$CC --version"; $CC --version
echo "$CXX --version"; $CXX --version

# whitespace
if [[ $TRAVIS_OS_NAME == linux ]]; then
cd $upx_SRCDIR || exit 1
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
       elsif ($ARGV =~ m,(^|/)m?make(file|vars),i) { }
       elsif ($ARGV =~ m,/tmp/.*\.(disasm|dump)$,) { }
       elsif ($ARGV =~ m,/src/stub/src/arch/.*\.S$,) { }
       else { print "ERROR: hard TAB detected $ARGV: $_"; exit(1); }
    }
' || exit 1
echo "  Done."
fi # linux

set -x

#
# build UCL
#

cd $ucl_BUILDDIR || exit 1
if [[ ! -f config.status ]]; then
$ucl_SRCDIR/configure --enable-static --disable-shared --disable-asm
fi
make

#
# build UPX
#

cd $upx_BUILDDIR || exit 1
make="make -f $upx_SRCDIR/src/Makefile"
EXTRA_CPPFLAGS="$EXTRA_CPPFLAGS -DUCL_NO_ASM"
EXTRA_LDFLAGS="$EXTRA_LDFLAGS -L$ucl_BUILDDIR/src/.libs"
if [[ $B =~ (^|\+)coverage($|\+) ]]; then
    EXTRA_CXXFLAGS="$EXTRA_CXXFLAGS -fprofile-arcs -ftest-coverage"
    EXTRA_LDFLAGS="$EXTRA_LDFLAGS -fprofile-arcs -ftest-coverage"
fi
if [[ $B =~ (^|\+)debug($|\+) ]]; then
    make="$make BUILD_TYPE_DEBUG=1"
fi
if [[ $B =~ (^|\+)sanitize($|\+) ]]; then
    case $TRAVIS_OS_NAME-$CC in linux-gcc*) EXTRA_LDFLAGS="$EXTRA_LDFLAGS -fuse-ld=gold" ;; esac
    make="$make BUILD_TYPE_SANITIZE=1"
fi
if [[ $B =~ (^|\+)scan-build($|\+) ]]; then
    make="$SCAN_BUILD $make"
fi
if [[ $B =~ (^|\+)valgrind($|\+) ]]; then
    EXTRA_CPPFLAGS="$EXTRA_CPPFLAGS -DWITH_VALGRIND"
fi
if [[ $ALLOW_FAIL == 1 ]]; then
    echo "ALLOW_FAIL=$ALLOW_FAIL"
    set +e
fi
export EXTRA_CPPFLAGS EXTRA_CXXFLAGS EXTRA_LDFLAGS

$make

ls -l upx.out
size upx.out
file upx.out

exit 0
