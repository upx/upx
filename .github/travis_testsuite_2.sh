#! /bin/bash
## vim:set ts=4 sw=4 et:
set -e; set -o pipefail

# Support for Travis CI -- https://travis-ci.org/upx/upx/builds
# Copyright (C) Markus Franz Xaver Johannes Oberhumer

source ./.github/travis_init.sh || exit 1

set -x

cd $upx_BUILDDIR || exit 1
if [[ $ALLOW_FAIL == 1 ]]; then
    echo "ALLOW_FAIL=$ALLOW_FAIL"
    set +e
    if [[ ! -x $PWD/upx.out ]]; then exit 0; fi
fi
if [[ ! -x $PWD/upx.out ]]; then exit 1; fi

#
# very first version of the upx-testsuite
#

exit_code=0

sha256sum=sha256sum
if [[ $TRAVIS_OS_NAME == osx ]]; then
    sha256sum=gsha256sum # brew install coreutils
fi
upx=$PWD/upx.out
if [[ $B =~ (^|\+)valgrind($|\+) ]]; then
    upx="valgrind --leak-check=full --show-reachable=yes $upx"
fi
upx_391=
if [[ $TRAVIS_OS_NAME == linux ]]; then
    rm -f upx391.out
    cp $upx_testsuite_SRCDIR/files/packed/amd64-linux.elf/upx-3.91 upx391.out
    upx_391="$PWD/upx391.out --fake-stub-version=3.92 --fake-stub-year=2016"
fi

if [[ $B =~ (^|\+)coverage($|\+) ]]; then
    lcov -d $upx_BUILDDIR --zerocounters
fi

$upx --version
$upx --help

cd $upx_testsuite_SRCDIR/files || exit 1
ls -l            packed/*/upx-3.91*
$upx -l          packed/*/upx-3.91*
$upx --file-info packed/*/upx-3.91*
$upx -t          packed/*/upx-3.91*
cd $upx_testsuite_BUILDDIR || exit 1
rm -rf ./t
for f in $upx_testsuite_SRCDIR/files/packed/*/upx-3.91*; do
    [[ -d $f ]] && continue
    echo "===== $f"
    mkdir -p ./t
    if [[ -n $upx_391 ]]; then
        $upx_391 -d $f -o t/v391
        $upx     -d $f -o t/v392
        $sha256sum t/v391 t/v392
        cmp -s t/v391 t/v392
        $upx_391 --lzma t/v391 -o t/v391_packed
        $upx     --lzma t/v392 -o t/v392_packed
        $sha256sum t/v391_packed t/v392_packed
    else
        $upx     -d $f -o t/v392
        $sha256sum t/v392
        $upx     --lzma t/v392 -o t/v392_packed
        $sha256sum t/v392_packed
    fi
    $upx -d t/v392_packed -o t/v392_decompressed
    # after the first compression+decompression step the exe should be
    # canonicalized so that further compression+decompression runs
    # should yield identical results
    if ! cmp -s t/v392 t/v392_decompressed; then
        # UPX 3.91 and 3.92 differ; run one more compression+decompression
        ls -l t/v392 t/v392_decompressed
        echo "UPX-WARNING: $f"
        $upx t/v392_decompressed -o t/v392_packed_2
        $upx -d t/v392_packed_2 -o t/v392_decompressed_2
        if ! cmp -s t/v392_decompressed t/v392_decompressed_2; then
            ls -l t/v392_decompressed t/v392_decompressed_2
            echo "UPX-ERROR: $f"
            exit_code=1
        fi
    fi
    #
    u391=$upx_391
    [[ -z $u391 ]] && u391=true
    x=t/v392
    $u391 --lzma -2 $x -o t/x_v391_lzma2
    $upx  --lzma -2 $x -o t/x_v392_lzma2
    $u391 --lzma -2 --small $x -o t/x_v391_lzma2_small
    $upx  --lzma -2 --small $x -o t/x_v392_lzma2_small
    $u391 --lzma -2 --all-filters $x -o t/x_v391_lzma2_small_all_filters
    $upx  --lzma -2 --all-filters $x -o t/x_v392_lzma2_small_all_filters
    ls -l      t/x_v*
    $upx -t    t/x_v*
    $sha256sum t/x_v*
    #
    rm -rf ./t
done

if [[ $B =~ (^|\+)coverage($|\+) ]]; then
    if [[ -n $TRAVIS_JOB_ID ]]; then
        cd $upx_SRCDIR || exit 1
        coveralls -b $upx_BUILDDIR --gcov-options '\-lp' || true
    else
        cd $upx_BUILDDIR || exit 1
        mkdir -p $lcov_OUTPUTDIR
        lcov -d . --capture -o $lcov_OUTPUTDIR/upx.info
        cd $lcov_OUTPUTDIR || exit 1
        genhtml upx.info
    fi
fi

exit $exit_code
