#! /bin/bash
## vim:set ts=4 sw=4 et:
set -e; set -o pipefail

# Support for Travis CI -- https://travis-ci.org/upx/upx/builds
# Copyright (C) Markus Franz Xaver Johannes Oberhumer

#
# very first version of the upx-testsuite
#

if [[ $TRAVIS_OS_NAME == osx ]]; then
argv0=$0; argv0abs=$(greadlink -en -- "$0"); argv0dir=$(dirname "$argv0abs")
else
argv0=$0; argv0abs=$(readlink -en -- "$0"); argv0dir=$(dirname "$argv0abs")
fi
source "$argv0dir/travis_init.sh" || exit 1

if [[ $BM_T =~ (^|\+)SKIP($|\+) ]]; then
    echo "UPX testsuite SKIPPED."
    exit 0
fi
if [[ $BM_X == rebuild-stubs ]]; then
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
        echo "UPX-ERROR: $1 FAILED: checksum mismatch"
        diff -u $1/.sha256sums.expected $1/.sha256sums.current || true
        exit_code=1
        let num_errors+=1 || true
        all_errors="${all_errors} $1"
        #exit 1
    fi
    echo
}

testsuite_check_sha_decompressed() {
    (cd "$1" && sha256sum -b */* | LC_ALL=C sort -k2) > $1/.sha256sums.current
    if ! cmp -s $1/.sha256sums.expected $1/.sha256sums.current; then
        cat $1/.sha256sums.current
        echo "UPX-ERROR: FATAL: $1 FAILED: decompressed checksum mismatch"
        diff -u $1/.sha256sums.expected $1/.sha256sums.current || true
        exit 1
    fi
}

testsuite_use_canonicalized=1
testsuite_run_compress() {
    testsuite_header $testdir
    local files f
    if [[ $testsuite_use_canonicalized == 1 ]]; then
        files=t020_canonicalized/*/*
    else
        files=t010_decompressed/*/*
    fi
    for f in $files; do
        testsuite_split_f $f
        [[ -z $fb ]] && continue
        echo "# $f"
        mkdir -p $testdir/$fsubdir $testdir/.decompressed/$fsubdir
        $upx_run -qq --prefer-ucl "$@" $f -o $testdir/$fsubdir/$fb
        $upx_run -qq -d $testdir/$fsubdir/$fb -o $testdir/.decompressed/$fsubdir/$fb
    done
    testsuite_check_sha $testdir
    $upx_run -qq -l $testdir/*/*
    $upx_run -qq --file-info $testdir/*/*
    $upx_run -q -t $testdir/*/*
    if [[ $testsuite_use_canonicalized == 1 ]]; then
        # check that after decompression the file matches the canonicalized version
        cp t020_canonicalized/.sha256sums.expected $testdir/.decompressed/
        testsuite_check_sha_decompressed $testdir/.decompressed
        rm -rf "./$testdir/.decompressed"
    fi
}


# /***********************************************************************
# // expected checksums
# //
# // To ease maintenance of this script in case of updates this section
# // can be automatically re-created from the current checksums -
# // see call of function recreate_expected_sha256sums below.
# ************************************************************************/

recreate_expected_sha256sums() {
    local o="$1"
    local files f d
    echo "########## begin .sha256sums.recreate" > "$o"
    files=*/.sha256sums.current
    for f in $files; do
        d=$(dirname "$f")
        echo "expected_sha256sums__${d}="'"\' >> "$o"
        cat "$f" >> "$o"
        echo '"' >> "$o"
    done
    echo "########## end .sha256sums.recreate" >> "$o"
}

########## begin .sha256sums.recreate
expected_sha256sums__t010_decompressed="\
24158f78c34c4ef94bb7773a6dda7231d289be76c2f5f60e8b9ddb3f800c100e *amd64-linux.elf/upx-3.91
28d7ca8f0dfca8159e637eaf2057230b6e6719e07751aca1d19a45b5efed817c *arm-wince.pe/upx-3.91.exe
b1c1c38d50007616aaf8e942839648c80a6111023e0b411e5fa7a06c543aeb4a *armeb-linux.elf/upx-3.91
bcac77a287289301a45fde9a75e4e6c9ad7f8d57856bae6eafaae12ae4445a34 *i386-dos32.djgpp2.coff/upx-3.91.exe
730a513b72a094697f827e4ac1a4f8ef58a614fc7a7ad448fa58d60cd89af7ed *i386-linux.elf/upx-3.91
0dbc3c267ca8cd35ee3ea138b59c8b1ae35872c918be6d17df1a892b75710f9b *i386-win32.pe/upx-3.91.exe
8e5333ea040f5594d3e67d5b09e005d52b3a52ef55099a7c11d7e39ead38e66d *m68k-atari.tos/upx-3.91.ttp
c3f44b4d00a87384c03a6f9e7aec809c1addfe3e271244d38a474f296603088c *mipsel-linux.elf/upx-3.91
b8c35fa2956da17ca505956e9f5017bb5f3a746322647e24ccb8ff28059cafa4 *powerpc-linux.elf/upx-3.91
"
expected_sha256sums__t020_canonicalized="\
24158f78c34c4ef94bb7773a6dda7231d289be76c2f5f60e8b9ddb3f800c100e *amd64-linux.elf/upx-3.91
28d7ca8f0dfca8159e637eaf2057230b6e6719e07751aca1d19a45b5efed817c *arm-wince.pe/upx-3.91.exe
b1c1c38d50007616aaf8e942839648c80a6111023e0b411e5fa7a06c543aeb4a *armeb-linux.elf/upx-3.91
bcac77a287289301a45fde9a75e4e6c9ad7f8d57856bae6eafaae12ae4445a34 *i386-dos32.djgpp2.coff/upx-3.91.exe
730a513b72a094697f827e4ac1a4f8ef58a614fc7a7ad448fa58d60cd89af7ed *i386-linux.elf/upx-3.91
0dbc3c267ca8cd35ee3ea138b59c8b1ae35872c918be6d17df1a892b75710f9b *i386-win32.pe/upx-3.91.exe
8e5333ea040f5594d3e67d5b09e005d52b3a52ef55099a7c11d7e39ead38e66d *m68k-atari.tos/upx-3.91.ttp
c3f44b4d00a87384c03a6f9e7aec809c1addfe3e271244d38a474f296603088c *mipsel-linux.elf/upx-3.91
b8c35fa2956da17ca505956e9f5017bb5f3a746322647e24ccb8ff28059cafa4 *powerpc-linux.elf/upx-3.91
"
expected_sha256sums__t110_compress_ucl_nrv2b_3_no_filter="\
92839fcb951cfeb37a3bbbcc49fb954e8ce3fd983ca749101f7b8e7ce3c1830e *amd64-linux.elf/upx-3.91
6be5bd999387712f90fde45556c9487d02306593146c8d3cd4ced89568732729 *arm-wince.pe/upx-3.91.exe
9d0ccc7e39ef23845b858869bf9718c43de5fd0eee7c61edbd63565a55110032 *armeb-linux.elf/upx-3.91
03988f039a5372125bf90b6658516656a8582dd1c46423cb12faeb82870348bb *i386-dos32.djgpp2.coff/upx-3.91.exe
91a9d980788c2974448c83a84af245cf22dbbe01d33850849c8b0ebcec79aab4 *i386-linux.elf/upx-3.91
b9145c02bac9ce7f10bd65587ce35001d3e15a7a102d8f7cc51930b92f477ebf *i386-win32.pe/upx-3.91.exe
08baacfa5d7e9339cd67263f41983e7c6f773fe9fce862f5daad58a2ac57ce28 *m68k-atari.tos/upx-3.91.ttp
6b33a055b2c85dddd5fa6d846939d114ee24a342205f5a23aad31b8873041592 *mipsel-linux.elf/upx-3.91
3e97639d9b13132edd3cf5e17f18a5d3f158124b920cabe8c415b8df3e4a8d1d *powerpc-linux.elf/upx-3.91
"
expected_sha256sums__t120_compress_ucl_nrv2d_3_no_filter="\
21aaf94748d315fe38af79da78d96ed100c382027af089cb3188d71d6a910b84 *amd64-linux.elf/upx-3.91
23ef6083a6e678975305ceb96f78ce186973b4400d357ca643573b6dd6487512 *arm-wince.pe/upx-3.91.exe
5f1584a75584ef3421ba4aec8167422e78d33e2451ef9a925560d1112eacf88e *armeb-linux.elf/upx-3.91
8d2938aa4c2b6ee3c50a2fce1041592a766d48c26605b50129b8a14697cc9694 *i386-dos32.djgpp2.coff/upx-3.91.exe
40ecb345f3094446b5035e058b3d2351c42cc9f108333e3e4d51a64142d8950c *i386-linux.elf/upx-3.91
7f4931501baa222ce54b241417f143162865de790be48ecc75846115baa2ce2f *i386-win32.pe/upx-3.91.exe
5abbe27c76c6f12fb560e68dbf56dd0f39bb089428374c42be7585d09f7df1fd *m68k-atari.tos/upx-3.91.ttp
ac552f23195dfebd82558e0523eaa6ece695c711f298ed9a3cdbb571f1df4f1d *mipsel-linux.elf/upx-3.91
d30f7e75bcfab63aa620e0c677f738395ab7c8b26ba0871e94e052849660dccb *powerpc-linux.elf/upx-3.91
"
expected_sha256sums__t130_compress_ucl_nrv2e_3_no_filter="\
ec993e4115bc2d4b88b640bd2c4beb0dabdbb2af3df6d03e3f0ba946bd57b157 *amd64-linux.elf/upx-3.91
8844ace5189f90a9bf696e5fa2323be9e715d90df4ba60805a2079cb94c218f1 *arm-wince.pe/upx-3.91.exe
ea42d52676d2698f29587e98a9e1536b96ebf523a7d8bc926f6c241a159b4116 *armeb-linux.elf/upx-3.91
27f1afd1e1fec37d4dcd43163b9ec60a6af3abeab21a4adf539b7faea848eb20 *i386-dos32.djgpp2.coff/upx-3.91.exe
5edf3dcfd0458b0f23c19e6c44b4c54a5457bc34c3bf045b37678d4f96301e00 *i386-linux.elf/upx-3.91
e440de3826a193a81700dd6238eab5f903826e08af120a25bc963ffb6d365087 *i386-win32.pe/upx-3.91.exe
510e2cd126bb8129b9076eb61e1ad3308d9af109cd42dff7252bf43e1e3552ea *m68k-atari.tos/upx-3.91.ttp
b61aa58e493b3961646e5c0bcb7f19f74b439fe8f5c934db25b00f97f51a05d0 *mipsel-linux.elf/upx-3.91
18c6b0ec20590dfbe4851ad837e2bb9bc99c6e4e1ac97b49dff2fb632e0e9d4c *powerpc-linux.elf/upx-3.91
"
expected_sha256sums__t140_compress_lzma_2_no_filter="\
73e37cb63dac97dac1276d8ed627b54500ddd32378c74d83df161be967643202 *amd64-linux.elf/upx-3.91
0eceefcc32ed25b2f1296cfa853935124aa1b7785b8a177d6e8c5804c24ca319 *arm-wince.pe/upx-3.91.exe
3e7446d8ac6d9b442d6c8e6a3c45aec97f54aa8e90bdc554dd39976495bfa36a *armeb-linux.elf/upx-3.91
1ecd559c89d3995fa0e7ecfa8e60b7746f804fc74ddce87d2c6cbeac8e372a39 *i386-dos32.djgpp2.coff/upx-3.91.exe
8a9bde7a9b4a8f6c4efc4a28afe2b3cabe6c0b820dbc38c74c6abae73093b784 *i386-linux.elf/upx-3.91
02d3b09168728cace52f29c5ee5a455c71d2777ccf12f6767cf92a709ceb6640 *i386-win32.pe/upx-3.91.exe
35333e53bc2f8a4a8e346f4fba0d366f024bd63d0b8bb083967193ae6466b6ff *m68k-atari.tos/upx-3.91.ttp
20526efe085aee06bfde1952f90e9d0e1524e0cb56d582caa720934f55562b78 *mipsel-linux.elf/upx-3.91
65990ec2ab6bceb2d5444918af797be0bdb0931abce8e7db0d2e774614faa64b *powerpc-linux.elf/upx-3.91
"
expected_sha256sums__t150_compress_ucl_2_all_filters="\
6ada3f67b70c39c0c2cc2c0ee946859ac38339a4276e94539e484a0755f60f51 *amd64-linux.elf/upx-3.91
0bf874c45ebb90b062cd986fe10cfbd01cdc6a50e3e0b5dda035eb55ded492ff *arm-wince.pe/upx-3.91.exe
db894f24777334abb9bf8e34661d057e25c2889b46290cf83db7310c4cdd86fa *armeb-linux.elf/upx-3.91
33bde57c3f7ed8809bec0658fe6c48d047bdec97476643f2e8e2970ab0c4eb5f *i386-dos32.djgpp2.coff/upx-3.91.exe
7f4ad752cab7db426356eb5068e37d06bade2422b6012069814dcfadae7e2cca *i386-linux.elf/upx-3.91
b00756ba6b73b937ac3b2c1222267dc44953ac69337a7ac582ba2e7a471e83a6 *i386-win32.pe/upx-3.91.exe
7b09fa5b2f8d85673f3a52c3b478a8dd129ed23681de4028625b33f5f6890942 *m68k-atari.tos/upx-3.91.ttp
707b7f32b8b0c1a4e5c26e370b93ae6eb60ab78f7d6163b4e853eb94e69f5d14 *mipsel-linux.elf/upx-3.91
4403a20c1951c9d42b51a175b55a4a9ee6901d626b8e3ed1e7e55a51ae098272 *powerpc-linux.elf/upx-3.91
"
expected_sha256sums__t160_compress_all_methods_1_no_filter="\
cd9d2ded81609941406fd6d64bfddcd5b674a8da67fd936643293494c404c724 *amd64-linux.elf/upx-3.91
df81b2376af28989faa0e48f9cb5d5f339d94166b7e11232c8d1ff952113a669 *arm-wince.pe/upx-3.91.exe
07ca9736f1cd478bde2c8da95a6ecb95bd865dcf8ea48a396b13050ac40d196e *armeb-linux.elf/upx-3.91
9e852e95b38ca57319d1e911bf97226a82b60dd336e201496c30bf62943aecce *i386-dos32.djgpp2.coff/upx-3.91.exe
e408fbcbbe422c06fce0cfed5d28ea0f0ef7c1b9225bf1f5a7d789cc84fb93b8 *i386-linux.elf/upx-3.91
30dbd980e85c88df62f4c13d6d0e44708f4ebfac63a4c12fa6a91b75563aead6 *i386-win32.pe/upx-3.91.exe
dd561ff9c530711be0934db352cf40f606066c280e27b26e7f8fd6dd1a087e6a *m68k-atari.tos/upx-3.91.ttp
813536a8f1b8b8852ed52c560afce8b51612876ac5a3a9e51434d8677932ef7b *mipsel-linux.elf/upx-3.91
c6d010b0cdee8adc381efc424a44c14a1d110c52d7b12925265fddc4ba781a6c *powerpc-linux.elf/upx-3.91
"
expected_sha256sums__t170_compress_all_methods_no_lzma_5_no_filter="\
1ee8b3195a96b7677b296b852931fa5eb86fde8e68f428f7081022fecea1c8a4 *amd64-linux.elf/upx-3.91
debd7f6b30bfcb1fef0fac558d8ae721c6a9d193c9b41126b3491c66870435ab *arm-wince.pe/upx-3.91.exe
7ee87918e7473d44c3a5af014cad48298f0a99c4028880d0a87a5a1776b977c7 *armeb-linux.elf/upx-3.91
dcdcad70f1b79290022041938957dccd4daa2f2fad166864cfb55f3400d5b03b *i386-dos32.djgpp2.coff/upx-3.91.exe
fece6171853a531cffdf264ae0ca9a47ee3b9b9a1416adc1ba2632e5da8a5d93 *i386-linux.elf/upx-3.91
0a32f50fe00e6f24ab5ab0bbfdfd037819f01071e849fb486a07ccc0794105f9 *i386-win32.pe/upx-3.91.exe
73dc44603f0fbf3a2a9e470c1291671b2670fa7f2f0cb54a8a7cdb883d1783c9 *m68k-atari.tos/upx-3.91.ttp
48877f2e614d4697e0ebc6d3f69e59328e0942e48bb5e54b4d9b9fe3cb917d9f *mipsel-linux.elf/upx-3.91
9df3e5bb2a641d03da582f6e5dc01e2497e616d585c8e2c07c78910ca7ac6db3 *powerpc-linux.elf/upx-3.91
"
########## end .sha256sums.recreate



# /***********************************************************************
# // init
# ************************************************************************/

#set -x # debug
exit_code=0
num_errors=0
all_errors=

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

export UPX="--prefer-ucl --no-color --no-progress"

# let's go
if ! $upx_run --version;          then echo "UPX-ERROR: FATAL: upx --version FAILED"; exit 1; fi
if ! $upx_run -L >/dev/null 2>&1; then echo "UPX-ERROR: FATAL: upx -L FAILED"; exit 1; fi
if ! $upx_run --help >/dev/null;  then echo "UPX-ERROR: FATAL: upx --help FAILED"; exit 1; fi
rm -rf ./testsuite_1
mkbuilddirs testsuite_1
cd testsuite_1 || exit 1


# /***********************************************************************
# // decompression tests
# ************************************************************************/

testdir=t010_decompressed
mkdir $testdir; v=expected_sha256sums__$testdir; echo -n "${!v}" >$testdir/.sha256sums.expected

testsuite_header $testdir
for f in $upx_testsuite_SRCDIR/files/packed/*/upx-3.91*; do
    testsuite_split_f $f
    [[ -z $fb ]] && continue
    echo "# $f"
    mkdir -p $testdir/$fsubdir
    $upx_run -qq -d $f -o $testdir/$fsubdir/$fb
done
testsuite_check_sha $testdir


# run one pack+unpack step to canonicalize the files
testdir=t020_canonicalized
mkdir $testdir; v=expected_sha256sums__$testdir; echo -n "${!v}" >$testdir/.sha256sums.expected

testsuite_header $testdir
for f in t010_decompressed/*/*; do
    testsuite_split_f $f
    [[ -z $fb ]] && continue
    echo "# $f"
    mkdir -p $testdir/$fsubdir/.packed
    $upx_run -qq --prefer-ucl -1 $f -o $testdir/$fsubdir/.packed/$fb
    $upx_run -qq -d $testdir/$fsubdir/.packed/$fb -o $testdir/$fsubdir/$fb
done
testsuite_check_sha $testdir


# /***********************************************************************
# // compression tests
# // info: we use fast compression levels because we want to
# //   test UPX and not the compression libraries
# ************************************************************************/

testdir=t110_compress_ucl_nrv2b_3_no_filter
mkdir $testdir; v=expected_sha256sums__$testdir; echo -n "${!v}" >$testdir/.sha256sums.expected
time testsuite_run_compress --nrv2b -3 --no-filter

testdir=t120_compress_ucl_nrv2d_3_no_filter
mkdir $testdir; v=expected_sha256sums__$testdir; echo -n "${!v}" >$testdir/.sha256sums.expected
time testsuite_run_compress --nrv2d -3 --no-filter

testdir=t130_compress_ucl_nrv2e_3_no_filter
mkdir $testdir; v=expected_sha256sums__$testdir; echo -n "${!v}" >$testdir/.sha256sums.expected
time testsuite_run_compress --nrv2e -3 --no-filter

testdir=t140_compress_lzma_2_no_filter
mkdir $testdir; v=expected_sha256sums__$testdir; echo -n "${!v}" >$testdir/.sha256sums.expected
time testsuite_run_compress --lzma -2 --no-filter

testdir=t150_compress_ucl_2_all_filters
mkdir $testdir; v=expected_sha256sums__$testdir; echo -n "${!v}" >$testdir/.sha256sums.expected
time testsuite_run_compress -2 --all-filters

testdir=t160_compress_all_methods_1_no_filter
mkdir $testdir; v=expected_sha256sums__$testdir; echo -n "${!v}" >$testdir/.sha256sums.expected
time testsuite_run_compress --all-methods -1 --no-filter

testdir=t170_compress_all_methods_no_lzma_5_no_filter
mkdir $testdir; v=expected_sha256sums__$testdir; echo -n "${!v}" >$testdir/.sha256sums.expected
time testsuite_run_compress --all-methods --no-lzma -5 --no-filter


# /***********************************************************************
# // summary
# ************************************************************************/

# recreate checkums from current version for an easy update in case of changes
recreate_expected_sha256sums .sha256sums.recreate

testsuite_header "UPX testsuite summary"
$upx_run --version || echo "UPX-ERROR: upx --version FAILED"
echo
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
cat */.sha256sums.expected | LC_ALL=C sort | wc
cat */.sha256sums.current  | LC_ALL=C sort | wc
echo
if [[ $exit_code == 0 ]]; then
    echo "UPX testsuite passed. All done."
else
    echo "UPX-ERROR: UPX testsuite FAILED:${all_errors}"
    echo "UPX-ERROR: UPX testsuite FAILED with $num_errors error(s). See log file."
fi
exit $exit_code
