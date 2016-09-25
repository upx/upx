#! /bin/bash
## vim:set ts=4 sw=4 et:
set -e; set -o pipefail

# Support for Travis CI -- https://travis-ci.org/upx/upx/builds
# Copyright (C) Markus Franz Xaver Johannes Oberhumer

source "$TRAVIS_BUILD_DIR/.github/travis_init.sh" || exit 1

set -x

cd /; cd "$BUILD_DIR" || exit 1
if test "$ALLOW_FAIL" = "1"; then
    echo "ALLOW_FAIL=$ALLOW_FAIL"
    set +e
    if ! test -x $PWD/upx.out; then exit 0; fi
fi
if ! test -x $PWD/upx.out; then exit 1; fi

#
# very first version of the upx-testsuite
#

exit_code=0

checksum=sha256sum
if test "$TRAVIS_OS_NAME" = "osx"; then
    checksum=true # TODO: travis-osx does not have md5sum and friends?
fi
upx="$PWD/upx.out"
case $BUILD_METHOD in
    valgrind | valgrind+* | *+valgrind | *+valgrind+*)
        upx="valgrind --leak-check=full --show-reachable=yes $upx" ;;
esac
upx_391=false
if test "$TRAVIS_OS_NAME" = "linux"; then
    cp "$TRAVIS_BUILD_DIR/deps/upx-testsuite/files/packed/amd64-linux.elf/upx-3.91" upx391.out
    upx_391="$PWD/upx391.out"
fi
$upx --help
cd /; cd "$TRAVIS_BUILD_DIR/deps/upx-testsuite/files" || exit 1
ls -l            packed/*/upx-3.91*
$upx -l          packed/*/upx-3.91*
$upx --file-info packed/*/upx-3.91*
$upx -t          packed/*/upx-3.91*
for f in packed/*/upx-3.91*; do
    echo "===== $f"
    if test "$TRAVIS_OS_NAME" = "linux"; then
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

exit $exit_code
