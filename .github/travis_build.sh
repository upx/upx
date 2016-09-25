#! /bin/bash
## vim:set ts=4 sw=4 et:
set -e; set -o pipefail

# Support for Travis CI -- https://travis-ci.org/upx/upx/builds
# Copyright (C) Markus Franz Xaver Johannes Oberhumer

source "$TRAVIS_BUILD_DIR/.github/travis_init.sh" || exit 1

echo "BUILD_METHOD='$BUILD_METHOD'"
echo "BUILD_DIR='$BUILD_DIR'"
echo "UPX_UCLDIR='$UPX_UCLDIR'"
echo "CC='$CC'"
echo "CXX='$CXX'"
echo "CPPFLAGS='$CPPFLAGS'"
echo "CFLAGS='$CFLAGS'"
echo "CXXFLAGS='$CXXFLAGS'"
echo "LDFLAGS='$LDFLAGS'"
echo "LIBS='$LIBS'"
echo "SCAN_BUILD='$SCAN_BUILD'"
#env | LC_ALL=C sort

echo "$CC --version"; $CC --version
echo "$CXX --version"; $CXX --version

# whitespace
if test "$TRAVIS_OS_NAME" = "linux"; then
echo "Checking source code for whitespace violations..."
find . \
    -type d -name '.git' -prune -o \
    -type d -name 'deps' -prune -o \
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
       elsif ($ARGV =~ m,(^|/)make(file|vars),i) { }
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

cd /; cd "$UPX_UCLDIR" || exit 1
./configure --enable-static --disable-shared --disable-asm
make

#
# build UPX
#

cd /; cd "$BUILD_DIR" || exit 1
make="make -f $TRAVIS_BUILD_DIR/src/Makefile"
export EXTRA_CPPFLAGS="-DUCL_NO_ASM"
case $BUILD_METHOD in
    debug | debug+* | *+debug | *+debug+*)
       make="$make USE_DEBUG=1" ;;
esac
case $BUILD_METHOD in
    sanitize | sanitize+* | *+sanitize | *+sanitize+*)
        case $TRAVIS_OS_NAME-$CC in linux-gcc*) export EXTRA_LDFLAGS="-fuse-ld=gold" ;; esac
        make="$make USE_SANITIZE=1" ;;
esac
case $BUILD_METHOD in
    scan-build | scan-build+* | *+scan-build | *+scan-build+*)
        make="$SCAN_BUILD $make" ;;
esac
case $BUILD_METHOD in
    valgrind | valgrind+* | *+valgrind | *+valgrind+*)
        export EXTRA_CPPFLAGS="$EXTRA_CPPFLAGS -DWITH_VALGRIND" ;;
esac
if test "$ALLOW_FAIL" = "1"; then
    echo "ALLOW_FAIL=$ALLOW_FAIL"
    set +e
fi

$make

ls -l upx.out
size upx.out
file upx.out

exit 0
