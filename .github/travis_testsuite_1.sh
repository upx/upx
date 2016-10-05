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
# // info: we use fast compression levels because we want to
# //   test UPX and not the compression libraries
# ************************************************************************/

testdir=t02_compress_ucl_nrv2b_3_no_filter
mkdir $testdir; echo -n "\
aefe34311318ea210df73cc3c8fa8c7258673f2e27f8a511a3a8f2af135a918d *amd64-linux.elf/upx-3.91
a22a5bf72d564887d4b77f2d4e578ffece6e71b2a60cc1050862e13a65bff7c8 *arm-wince.pe/upx-3.91.exe
9d0ccc7e39ef23845b858869bf9718c43de5fd0eee7c61edbd63565a55110032 *armeb-linux.elf/upx-3.91
03988f039a5372125bf90b6658516656a8582dd1c46423cb12faeb82870348bb *i386-dos32.djgpp2.coff/upx-3.91.exe
91a9d980788c2974448c83a84af245cf22dbbe01d33850849c8b0ebcec79aab4 *i386-linux.elf/upx-3.91
1f4fcebacc6898338445c921ccec7ae51f4e21009c82cac9f66b70f556ee102b *i386-win32.pe/upx-3.91.exe
08baacfa5d7e9339cd67263f41983e7c6f773fe9fce862f5daad58a2ac57ce28 *m68k-atari.tos/upx-3.91.ttp
6b33a055b2c85dddd5fa6d846939d114ee24a342205f5a23aad31b8873041592 *mipsel-linux.elf/upx-3.91
611bde9c83fe28dd8f7119414724318c23ef532577bb404d618d100e3a801fe8 *powerpc-linux.elf/upx-3.91
" > $testdir/.sha256sums.expected
time testsuite_run_compress --nrv2b -3 --no-filter

testdir=t02_compress_ucl_nrv2d_3_no_filter
mkdir $testdir; echo -n "\
6e9fd36871cf4afebf6609a39690565c933668ec6791e547916e93bbb32bbbd8 *amd64-linux.elf/upx-3.91
45e86901dde8ef9ac9e9727110f629b57ca96d078e685f167b848e26a19a9693 *arm-wince.pe/upx-3.91.exe
5f1584a75584ef3421ba4aec8167422e78d33e2451ef9a925560d1112eacf88e *armeb-linux.elf/upx-3.91
8d2938aa4c2b6ee3c50a2fce1041592a766d48c26605b50129b8a14697cc9694 *i386-dos32.djgpp2.coff/upx-3.91.exe
40ecb345f3094446b5035e058b3d2351c42cc9f108333e3e4d51a64142d8950c *i386-linux.elf/upx-3.91
d96f0e8f42aff4c1b7b50d410498fd422852d9866338a12748f91488319a193f *i386-win32.pe/upx-3.91.exe
5abbe27c76c6f12fb560e68dbf56dd0f39bb089428374c42be7585d09f7df1fd *m68k-atari.tos/upx-3.91.ttp
ac552f23195dfebd82558e0523eaa6ece695c711f298ed9a3cdbb571f1df4f1d *mipsel-linux.elf/upx-3.91
abd1d89deb62839e957089e636a1e53bfc7436f6fd2a210ac92907346c239562 *powerpc-linux.elf/upx-3.91
" > $testdir/.sha256sums.expected
time testsuite_run_compress --nrv2d -3 --no-filter

testdir=t02_compress_ucl_nrv2e_3_no_filter
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
time testsuite_run_compress --nrv2e -3 --no-filter

testdir=t03_compress_lzma_2_no_filter
mkdir $testdir; echo -n "\
2aee8c5a4d1fb9f7a84700b9880da1098544c110a188750a937e0c231cce664f *amd64-linux.elf/upx-3.91
e6123d50ad24a10412ee97d9b440f5d5f9830f44a74aadd6ca20a7cee9a09d47 *arm-wince.pe/upx-3.91.exe
3e7446d8ac6d9b442d6c8e6a3c45aec97f54aa8e90bdc554dd39976495bfa36a *armeb-linux.elf/upx-3.91
1ecd559c89d3995fa0e7ecfa8e60b7746f804fc74ddce87d2c6cbeac8e372a39 *i386-dos32.djgpp2.coff/upx-3.91.exe
8a9bde7a9b4a8f6c4efc4a28afe2b3cabe6c0b820dbc38c74c6abae73093b784 *i386-linux.elf/upx-3.91
37f4f82c3de1ea3e444740b1112f42c8011129825bee011889beb4a592f5d9df *i386-win32.pe/upx-3.91.exe
35333e53bc2f8a4a8e346f4fba0d366f024bd63d0b8bb083967193ae6466b6ff *m68k-atari.tos/upx-3.91.ttp
20526efe085aee06bfde1952f90e9d0e1524e0cb56d582caa720934f55562b78 *mipsel-linux.elf/upx-3.91
49c88eb39416858ac1a485dfd3bcee3825ef76ed5a16cb951f68f212a057c014 *powerpc-linux.elf/upx-3.91
" > $testdir/.sha256sums.expected
time testsuite_run_compress --lzma -2 --no-filter

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
time testsuite_run_compress --all-methods -1 --no-filter

testdir=t06_compress_all_methods_no_lzma_5_no_filter
mkdir $testdir; echo -n "\
d2607801730ae28244a4ca310676f912131383d6126801df05ff54903c9ef18a *amd64-linux.elf/upx-3.91
575df11d8450ce11fabc007b913a95977f6e131d0472c92e22ef3af12d325904 *arm-wince.pe/upx-3.91.exe
7ee87918e7473d44c3a5af014cad48298f0a99c4028880d0a87a5a1776b977c7 *armeb-linux.elf/upx-3.91
dcdcad70f1b79290022041938957dccd4daa2f2fad166864cfb55f3400d5b03b *i386-dos32.djgpp2.coff/upx-3.91.exe
fece6171853a531cffdf264ae0ca9a47ee3b9b9a1416adc1ba2632e5da8a5d93 *i386-linux.elf/upx-3.91
771416e8a06041ab15f5f45702328eae8db53fcf29aee90c93815e1fc5e819de *i386-win32.pe/upx-3.91.exe
73dc44603f0fbf3a2a9e470c1291671b2670fa7f2f0cb54a8a7cdb883d1783c9 *m68k-atari.tos/upx-3.91.ttp
48877f2e614d4697e0ebc6d3f69e59328e0942e48bb5e54b4d9b9fe3cb917d9f *mipsel-linux.elf/upx-3.91
9c7f990bb0d322e5ac5131cb4180541b3d8918e1c81a730311e359e456ee92d0 *powerpc-linux.elf/upx-3.91
" > $testdir/.sha256sums.expected
time testsuite_run_compress --all-methods --no-lzma -5 --no-filter


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
echo ".sha256sums.{expected,current}:"
cat */.sha256sums.expected | LC_ALL=C sort -u | wc
cat */.sha256sums.current  | LC_ALL=C sort -u | wc
echo
if [[ $exit_code == 0 ]]; then
    echo "UPX testsuite passed. All done."
else
    echo "UPX-ERROR: UPX testsuite FAILED with $num_errors error(s). See log file."
fi
exit $exit_code
