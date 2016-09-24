#! /bin/bash
## vim:set ts=4 sw=4 et:
set -e; set -o pipefail

cd /; cd "$TRAVIS_BUILD_DIR" || exit 1

if test "X$B" = "X"; then B=release; fi
BUILD_METHOD="$B"

case $C in
    clang-m32) CC="clang -m32"; CXX="clang++ -m32" ;;
    clang-m64) CC="clang -m64"; CXX="clang++ -m64" ;;
    gcc-m32) CC="gcc -m32"; CXX="g++ -m32" ;;
    gcc-m64) CC="gcc -m64"; CXX="g++ -m64" ;;
    gcc-5-m32) CC="gcc-5 -m32"; CXX="g++-5 -m32" ;;
    gcc-5-m64) CC="gcc-5 -m64"; CXX="g++-5 -m64" ;;
    gcc-6-m32) CC="gcc-6 -m32 -std=gnu90"; CXX="g++-6 -m32" ;;
    gcc-6-m64) CC="gcc-6 -m64 -std=gnu90"; CXX="g++-6 -m64" ;;
esac
export CC CXX

export UPX_UCLDIR="$TRAVIS_BUILD_DIR/deps/ucl-1.03"

echo "BUILD_METHOD='$BUILD_METHOD'"
echo "CC='$CC'"
echo "CXX='$CXX'"
echo "CPPFLAGS='$CPPFLAGS'"
echo "CFLAGS='$CFLAGS'"
echo "CXXFLAGS='$CXXFLAGS'"
echo "LDFLAGS='$LDFLAGS'"
echo "LIBS='$LIBS'"
echo "BUILD_DIR='$BUILD_DIR'"
echo "UPX_UCLDIR='$UPX_UCLDIR'"
#env | LC_ALL=C sort

echo "$CC --version"; $CC --version
echo "$CXX --version"; $CXX --version

# whitespace
if test "X$TRAVIS_OS_NAME" = "Xlinux"; then
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
BUILD_DIR="$TRAVIS_BUILD_DIR/build"
mkdir -p "$BUILD_DIR"

# build UCL
cd /; cd "$UPX_UCLDIR" || exit 1
./configure --enable-static --disable-shared
make

# build UPX
cd /; cd "$BUILD_DIR" || exit 1
f="EXTRA_CPPFLAGS=-DUCL_NO_ASM"
make="make -f $TRAVIS_BUILD_DIR/src/Makefile $f"
if test "X$ALLOW_FAIL" = "X1"; then
    echo "ALLOW_FAIL=$ALLOW_FAIL"
    set +e
fi
case $BUILD_METHOD in
debug)
    $make USE_DEBUG=1 ;;
debug+sanitize)
    $make USE_DEBUG=1 USE_SANITIZE=1 ;;
release)
    $make ;;
sanitize)
    $make USE_SANITIZE=1 ;;
scan-build)
    scan-build $make ;;
*)
    echo "ERROR: invalid BUILD_METHOD '$BUILD_METHOD'"
    exit 1
    ;;
esac
ls -l upx.out
size upx.out
file upx.out

exit_code=0

# very first version of the upx-testsuite
if test -x $PWD/upx.out; then
checksum=sha256sum
if test "X$TRAVIS_OS_NAME" = "Xosx"; then
    checksum=true # TODO: travis-osx does not have md5sum and friends?
fi
upx="$PWD/upx.out"
upx_391=false
if test "X$TRAVIS_OS_NAME" = "Xlinux"; then
    cp "$TRAVIS_BUILD_DIR/deps/upx-testsuite/files/packed/amd64-linux.elf/upx-3.91" upx391.out
    upx_391="$PWD/upx391.out"
fi
$upx --help
cd /; cd "$TRAVIS_BUILD_DIR/deps/upx-testsuite/files" || exit 1
$upx -l packed/*/upx-3.91*
$upx --file-info packed/*/upx-3.91*
for f in packed/*/upx-3.91*; do
    echo "===== $f"
    if test "X$TRAVIS_OS_NAME" = "Xlinux"; then
        $upx_391 -d $f -o v391.tmp
        $upx     -d $f -o v392.tmp
        $checksum v391.tmp v392.tmp
        cmp -s v391.tmp v392.tmp
        $upx_391 --lzma --fake-stub-version=3.92 --fake-stub-year=2016 v391.tmp -o v391_packed.tmp
        $upx     --lzma                                                v392.tmp -o v392_packed.tmp
        $checksum v391_packed.tmp v392_packed.tmp
    else
        $upx     -d $f -o v392.tmp
        $checksum v392.tmp
        $upx     --lzma                                                v392.tmp -o v392_packed.tmp
        $checksum v392_packed.tmp
    fi
    $upx -d v392_packed.tmp -o v392_decompressed.tmp
    # after the first compression+decompression step the exe should be
    # canonicalized so that further compression+decompression runs
    # should yield identical results
    if ! cmp -s v392.tmp v392_decompressed.tmp; then
        # UPX 3.91 and 3.92 differ; run one more compression+decompression
        ls -l v392.tmp v392_decompressed.tmp
        echo "UPX-WARNING: $f"
        $upx v392_decompressed.tmp -o v392_packed_2.tmp
        $upx -d v392_packed_2.tmp -o v392_decompressed_2.tmp
        if ! cmp -s v392_decompressed.tmp v392_decompressed_2.tmp; then
            ls -l v392_decompressed.tmp v392_decompressed_2.tmp
            echo "UPX-ERROR: $f"
            exit_code=1
        fi
    fi
    rm *.tmp
done
fi

exit $exit_code

# vim:set ts=4 sw=4 et:
