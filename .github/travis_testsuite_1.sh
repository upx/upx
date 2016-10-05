#! /bin/bash
## vim:set ts=4 sw=4 et:
set -e; set -o pipefail

# Support for Travis CI -- https://travis-ci.org/upx/upx/builds
# Copyright (C) Markus Franz Xaver Johannes Oberhumer

#
# very first version of the upx-testsuite
#

if [[ $TRAVIS_OS_NAME == osx ]]; then
argv0="$0"; argv0dir=$(greadlink -en -- "$0"); argv0dir=$(dirname "$argv0dir")
else
argv0="$0"; argv0dir=$(readlink -en -- "$0"); argv0dir=$(dirname "$argv0dir")
fi
source "$argv0dir/travis_init.sh" || exit 1

if [[ $BM_T =~ (^|\+)SKIP($|\+) ]]; then
    echo "UPX testsuite SKIPPED."
    exit 0
fi
[[ -f $upx_exe ]] && upx_exe=$(readlink -en -- "$upx_exe")

# create dirs
cd / || exit 1
mkbuilddirs $upx_testsuite_BUILDDIR
cd / && cd $upx_testsuite_BUILDDIR || exit 1
if [[ ! -d $upx_testsuite_SRCDIR/files/packed ]]; then exit 1; fi


# /***********************************************************************
# // support functions
# ************************************************************************/

testsuite_header() {
    local x="==========="; x="$x$x$x$x$x$x$x"
    echo -e "\n${x}\n${1}\n${x}\n"
}

testsuite_split_f() {
    fd=$(dirname "$1")
    fb=$(basename "$1")
    fsubdir=$(basename "$fd")
    # sanity checks
    if [[ ! -f $f || -z $fsubdir || -z $fb ]]; then
        fd= fb= fsubdir=
    fi
}

testsuite_check_sha() {
    (cd "$1" && sha256sum -b */* | LC_ALL=C sort -k2) > $1/.sha256sums.current
    echo
    cat $1/.sha256sums.current
    if ! cmp -s $1/.sha256sums.expected $1/.sha256sums.current; then
        echo "UPX-ERROR: checksum mismatch"
        diff -u $1/.sha256sums.expected $1/.sha256sums.current || true
        #exit 1
        exit_code=1
        let num_errors+=1 || true
    fi
    echo
}

testsuite_run_compress() {
    testsuite_header $testdir
    local f
    for f in t01_decompressed/*/*; do
        testsuite_split_f $f
        [[ -z $fb ]] && continue
        mkdir -p $testdir/$fsubdir
        $upx_run --prefer-ucl "$@" $f -o $testdir/$fsubdir/$fb
    done
    testsuite_check_sha $testdir
    $upx_run -l $testdir/*/*
    $upx_run --file-info $testdir/*/*
    $upx_run -t $testdir/*/*
}

# /***********************************************************************
# // init
# ************************************************************************/

#set -x # debug
exit_code=0
num_errors=0

if [[ $BM_T =~ (^|\+)ALLOW_FAIL($|\+) ]]; then
    echo "ALLOW_FAIL"
    set +e
fi

[[ -z $upx_exe && -f $upx_BUILDDIR/upx.out ]] && upx_exe=$upx_BUILDDIR/upx.out
[[ -z $upx_exe && -f $upx_BUILDDIR/upx.exe ]] && upx_exe=$upx_BUILDDIR/upx.exe
if [[ -z $upx_exe ]]; then exit 1; fi
upx_run=$upx_exe
if [[ $BM_T =~ (^|\+)qemu($|\+) && -n $upx_qemu ]]; then
    upx_run="$upx_qemu $upx_qemu_flags -- $upx_exe"
fi
if [[ $BM_T =~ (^|\+)wine($|\+) && -n $upx_wine ]]; then
    upx_run="$upx_wine $upx_wine_flags $upx_exe"
fi
if [[ $BM_T =~ (^|\+)valgrind($|\+) ]]; then
    if [[ -z $upx_valgrind ]]; then
        upx_valgrind="valgrind"
    fi
    if [[ -z $upx_valgrind_flags ]]; then
        upx_valgrind_flags="--leak-check=full --show-reachable=yes"
        upx_valgrind_flags="-q --leak-check=no --error-exitcode=1"
        upx_valgrind_flags="--leak-check=no --error-exitcode=1"
    fi
    upx_run="$upx_valgrind $upx_valgrind_flags $upx_exe"
fi

if [[ $BM_B =~ (^|\+)coverage($|\+) ]]; then
    (cd / && cd $upx_BUILDDIR && lcov -d . --zerocounters)
fi

export UPX=
export UPX="--no-color --no-progress"

# let's go
if ! $upx_run --version >/dev/null; then exit 1; fi
rm -rf ./testsuite_1
mkbuilddirs testsuite_1
cd testsuite_1 || exit 1


# /***********************************************************************
# // decompression tests
# ************************************************************************/

testdir=t01_decompressed
mkdir $testdir; echo -n "\
24158f78c34c4ef94bb7773a6dda7231d289be76c2f5f60e8b9ddb3f800c100e *amd64-linux.elf/upx-3.91
28d7ca8f0dfca8159e637eaf2057230b6e6719e07751aca1d19a45b5efed817c *arm-wince.pe/upx-3.91.exe
b1c1c38d50007616aaf8e942839648c80a6111023e0b411e5fa7a06c543aeb4a *armeb-linux.elf/upx-3.91
bcac77a287289301a45fde9a75e4e6c9ad7f8d57856bae6eafaae12ae4445a34 *i386-dos32.djgpp2.coff/upx-3.91.exe
730a513b72a094697f827e4ac1a4f8ef58a614fc7a7ad448fa58d60cd89af7ed *i386-linux.elf/upx-3.91
0dbc3c267ca8cd35ee3ea138b59c8b1ae35872c918be6d17df1a892b75710f9b *i386-win32.pe/upx-3.91.exe
8e5333ea040f5594d3e67d5b09e005d52b3a52ef55099a7c11d7e39ead38e66d *m68k-atari.tos/upx-3.91.ttp
c3f44b4d00a87384c03a6f9e7aec809c1addfe3e271244d38a474f296603088c *mipsel-linux.elf/upx-3.91
b8c35fa2956da17ca505956e9f5017bb5f3a746322647e24ccb8ff28059cafa4 *powerpc-linux.elf/upx-3.91
" > $testdir/.sha256sums.expected

testsuite_header $testdir
for f in $upx_testsuite_SRCDIR/files/packed/*/upx-3.91*; do
    testsuite_split_f $f
    [[ -z $fb ]] && continue
    mkdir -p $testdir/$fsubdir
    $upx_run -d $f -o $testdir/$fsubdir/$fb
done
testsuite_check_sha $testdir


# /***********************************************************************
# // compression tests
# ************************************************************************/

testdir=t02_compress_ucl_3_no_filter
mkdir $testdir; echo -n "\
86fcb3e1c6255511173ff9a86baf3407263677dfe73bff072b978da41c892bcb *amd64-linux.elf/upx-3.91
16c0493d4d89e6c581748826afd26d2b4caeeae6074b0bdcf55747a4bc9777a5 *arm-wince.pe/upx-3.91.exe
ea42d52676d2698f29587e98a9e1536b96ebf523a7d8bc926f6c241a159b4116 *armeb-linux.elf/upx-3.91
27f1afd1e1fec37d4dcd43163b9ec60a6af3abeab21a4adf539b7faea848eb20 *i386-dos32.djgpp2.coff/upx-3.91.exe
5edf3dcfd0458b0f23c19e6c44b4c54a5457bc34c3bf045b37678d4f96301e00 *i386-linux.elf/upx-3.91
a94d5ff62c061d60d64e838ea4bb7610a75cf26a0761b26f9da0efbd38e87ebe *i386-win32.pe/upx-3.91.exe
510e2cd126bb8129b9076eb61e1ad3308d9af109cd42dff7252bf43e1e3552ea *m68k-atari.tos/upx-3.91.ttp
b61aa58e493b3961646e5c0bcb7f19f74b439fe8f5c934db25b00f97f51a05d0 *mipsel-linux.elf/upx-3.91
21071b7dfa542a7e3d6c6a3586b4b844d28270a2beccdc5d03f608a2e71b787d *powerpc-linux.elf/upx-3.91
" > $testdir/.sha256sums.expected
time testsuite_run_compress -3 --no-filter

testdir=t03_compress_lzma_1_no_filter
mkdir $testdir; echo -n "\
9d5a98a9f2f17e4d7066e0b953dae94cdbb1ea28c6c792ce8f92e8b8d932b311 *amd64-linux.elf/upx-3.91
99ad77f2280a5b4791ead800c7d4d5b29aa49ce40319616f682c1b29ddf20702 *arm-wince.pe/upx-3.91.exe
07ca9736f1cd478bde2c8da95a6ecb95bd865dcf8ea48a396b13050ac40d196e *armeb-linux.elf/upx-3.91
9e852e95b38ca57319d1e911bf97226a82b60dd336e201496c30bf62943aecce *i386-dos32.djgpp2.coff/upx-3.91.exe
e408fbcbbe422c06fce0cfed5d28ea0f0ef7c1b9225bf1f5a7d789cc84fb93b8 *i386-linux.elf/upx-3.91
e249633d550abe84953a15668e3d74660fd63e9e935bec8564d9df08116b015d *i386-win32.pe/upx-3.91.exe
dd561ff9c530711be0934db352cf40f606066c280e27b26e7f8fd6dd1a087e6a *m68k-atari.tos/upx-3.91.ttp
813536a8f1b8b8852ed52c560afce8b51612876ac5a3a9e51434d8677932ef7b *mipsel-linux.elf/upx-3.91
5a3607a37534bab2a22c2e1dc3f1aa26144c2b75c261f092a3d9116d884edc1a *powerpc-linux.elf/upx-3.91
" > $testdir/.sha256sums.expected
time testsuite_run_compress -1 --lzma --no-filter

testdir=t04_compress_ucl_2_all_filters
mkdir $testdir; echo -n "\
b7f51c8da38cc65c4bdec8a07e29a7c47404fc4b057c2026c13fd2a83b24da76 *amd64-linux.elf/upx-3.91
247e385b1bf2559982cfedde0dc6dafc3fe99ad09100afc8b6ec3e59722afd4f *arm-wince.pe/upx-3.91.exe
db894f24777334abb9bf8e34661d057e25c2889b46290cf83db7310c4cdd86fa *armeb-linux.elf/upx-3.91
33bde57c3f7ed8809bec0658fe6c48d047bdec97476643f2e8e2970ab0c4eb5f *i386-dos32.djgpp2.coff/upx-3.91.exe
7f4ad752cab7db426356eb5068e37d06bade2422b6012069814dcfadae7e2cca *i386-linux.elf/upx-3.91
32c7dd51179084d7191b57016dc9e508137f3e55ba59118dfbd1c4205f730790 *i386-win32.pe/upx-3.91.exe
7b09fa5b2f8d85673f3a52c3b478a8dd129ed23681de4028625b33f5f6890942 *m68k-atari.tos/upx-3.91.ttp
707b7f32b8b0c1a4e5c26e370b93ae6eb60ab78f7d6163b4e853eb94e69f5d14 *mipsel-linux.elf/upx-3.91
3951a8dbe60f41f7ae1d71ca8d0f022675a04542786351c09281dfce41fd49e0 *powerpc-linux.elf/upx-3.91
" > $testdir/.sha256sums.expected
time testsuite_run_compress -2 --all-filters

testdir=t05_compress_all_methods_1_no_filter
mkdir $testdir; echo -n "\
9d5a98a9f2f17e4d7066e0b953dae94cdbb1ea28c6c792ce8f92e8b8d932b311 *amd64-linux.elf/upx-3.91
99ad77f2280a5b4791ead800c7d4d5b29aa49ce40319616f682c1b29ddf20702 *arm-wince.pe/upx-3.91.exe
07ca9736f1cd478bde2c8da95a6ecb95bd865dcf8ea48a396b13050ac40d196e *armeb-linux.elf/upx-3.91
9e852e95b38ca57319d1e911bf97226a82b60dd336e201496c30bf62943aecce *i386-dos32.djgpp2.coff/upx-3.91.exe
e408fbcbbe422c06fce0cfed5d28ea0f0ef7c1b9225bf1f5a7d789cc84fb93b8 *i386-linux.elf/upx-3.91
e249633d550abe84953a15668e3d74660fd63e9e935bec8564d9df08116b015d *i386-win32.pe/upx-3.91.exe
dd561ff9c530711be0934db352cf40f606066c280e27b26e7f8fd6dd1a087e6a *m68k-atari.tos/upx-3.91.ttp
813536a8f1b8b8852ed52c560afce8b51612876ac5a3a9e51434d8677932ef7b *mipsel-linux.elf/upx-3.91
5a3607a37534bab2a22c2e1dc3f1aa26144c2b75c261f092a3d9116d884edc1a *powerpc-linux.elf/upx-3.91
" > $testdir/.sha256sums.expected
time testsuite_run_compress -1 --all-methods --no-filter

testdir=t06_compress_all_methods_no_lzma_no_filter
mkdir $testdir; echo -n "\
7ff6b8f6f8ce5dfbe8dacd5f9e1ebc8c9af20059a00c52b5e3703d31718b4839 *amd64-linux.elf/upx-3.91
63be06791e4caa5ea530bd58d0fe5af7830fe6907cebb4de6eec5a73ff57bf58 *arm-wince.pe/upx-3.91.exe
b95b5a9a1ba4577df41c135b0061a7282e633f006ce6a5c729d73a708f80c0f0 *armeb-linux.elf/upx-3.91
bbf7d2290c2aff5c914f14aaa7a089895888f3e25b468ba406f9723c046b0ccb *i386-dos32.djgpp2.coff/upx-3.91.exe
b615ce3f45d7b727594c550cb2066fff154653ffd64a5329ee6723175224cc47 *i386-linux.elf/upx-3.91
a647ed1aea16f58b544228279ad7159cd3ec5c3533efef1fd2df5a5a59b5d663 *i386-win32.pe/upx-3.91.exe
4ce409cc6c1a0e26b0d5e361cd6ef7b0198830a3244235eff3edb18099d1ad22 *m68k-atari.tos/upx-3.91.ttp
8b2b9f13dd613b62d9028d2c773a2633bb756a084e5620b3496449ffe1c2dc9e *mipsel-linux.elf/upx-3.91
84e82cfffa594230ee44aa06937ccb088778c05d5d67985a314764385018f59d *powerpc-linux.elf/upx-3.91
" > $testdir/.sha256sums.expected
time testsuite_run_compress --all-methods --no-lzma --no-filter


testsuite_header "UPX testsuite summary"
echo "upx_exe='$upx_exe'"
if [[ $upx_run != $upx_exe ]]; then
    echo "upx_run='$upx_run'"
fi
if [[ -f $upx_exe ]]; then
    ls -l "$upx_exe"
    file "$upx_exe" || true
fi
echo "upx_testsuite_SRCDIR='$upx_testsuite_SRCDIR'"
echo "upx_testsuite_BUILDDIR='$upx_testsuite_BUILDDIR'"
echo
if [[ $exit_code == 0 ]]; then
    echo "UPX testsuite passed. All done."
else
    echo "UPX-ERROR: UPX testsuite FAILED with $num_errors error(s). See log file."
fi
exit $exit_code
