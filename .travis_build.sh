#! /bin/bash
## vim:set ts=4 sw=4 et:
set -e; set -o pipefail

echo "BUILD_METHOD_AND_BUILD_TYPE='$BUILD_METHOD_AND_BUILD_TYPE'"
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

# build UCL
cd /
set -x
cd "$UPX_UCLDIR"
./configure --enable-static --disable-shared
make

cd /
set -x
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

case $BUILD_METHOD_AND_BUILD_TYPE in
make/debug)
    make -f "$TRAVIS_BUILD_DIR/src/Makefile" USE_DEBUG=1
    ;;
make/release)
    make -f "$TRAVIS_BUILD_DIR/src/Makefile"
    ;;
make/scan-build)
    if test "$CC" = "clang"; then
        scan-build make -f "$TRAVIS_BUILD_DIR/src/Makefile"
    else
        make -f "$TRAVIS_BUILD_DIR/src/Makefile" USE_SANITIZE=1
    fi
    ;;
*)
    echo "ERROR: invalid build '$BUILD_METHOD_AND_BUILD_TYPE'"
    exit 1
    ;;
esac
