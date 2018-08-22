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
    print_header "$1"
}

testsuite_split_f() {
    fd=$(dirname "$1")
    fb=$(basename "$1")
    fsubdir=$(basename "$fd")
    # sanity checks
    if [[ ! -f "$1" || -z "$fsubdir" || -z "$fb" ]]; then
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
7301bb93535f91bb789b44da75c2f365cc2d0fa99a4d597f49de8716d96dbcf6 *amd64-linux.elf/upx-3.91
17a9aef8087ca2b3f5fc4e3a41d7c4d721bf3ad222030ac2068ed00eb85a9deb *arm-wince.pe/upx-3.91.exe
3ddbde0d00ff6ea9ce163d889e6f418146b35238d05005b98b76adb555bcf644 *armeb-linux.elf/upx-3.91
f680566e3daae0526c5f6965c23219d7946c4d4117eeaa277bd9e68e907418c6 *i386-dos32.djgpp2.coff/upx-3.91.exe
14394ce9806dc7a3622addfb625d1731e021600e0489dc7c8ea7f30c54d1fd37 *i386-linux.elf/upx-3.91
d3cfb5347758ee54e54cfc92ae502a3e19702cd4fec115d74f84f8a5ab7a9bc2 *i386-win32.pe/upx-3.91.exe
d0e68eed8f001f3074351acbc7133bd41f67f8e3b7e5f82d5df1911481123378 *m68k-atari.tos/upx-3.91.ttp
58db1b12b5462dbb05c08d7952cacd7c7090a6b3cbfbf9b431e6390f246cbb95 *mipsel-linux.elf/upx-3.91
4fd2e2fc33563e95453f9b4a52c153f248937a548cbfe746bb0d71f19ae5ad03 *powerpc-linux.elf/upx-3.91
"
expected_sha256sums__t120_compress_ucl_nrv2d_3_no_filter="\
e537c1a28bea559cbe43cd23723443e2617316a40a6632927f6d94589f800efe *amd64-linux.elf/upx-3.91
33a13fa63379b9eae6878cd2bfb0b41963795a27bc5a026de0b50b4a43131b12 *arm-wince.pe/upx-3.91.exe
3c6f9ab67e21c72073f66c558765d8f07f192ba63d3ffb2ce40ebc9178ccd5ef *armeb-linux.elf/upx-3.91
28bc9ee28b73b11a32fb2e4d3ad99ff06d61cf5b9f104052db89f6f6f0557ca9 *i386-dos32.djgpp2.coff/upx-3.91.exe
8201f15f7ef5e54cddb18474a5ed2d48e2fc7726d6014c457ad3cd82c01e1072 *i386-linux.elf/upx-3.91
5bebadb8455b052580b1f22a949c3eb5a441c8b6ba9c6b50506cb703fc3f65ce *i386-win32.pe/upx-3.91.exe
e4d3a430f93b41a3aa67492e6c06d74a1fcd4e06b272de2c681252d91b7a953a *m68k-atari.tos/upx-3.91.ttp
1bd8be4eb09dc39b4b0d447ef4eebac29c51d56380be79b46461962dea81b8fb *mipsel-linux.elf/upx-3.91
c1928cd500450f1f09f2c0265980c329cd0e083095abb3a48f1999a7dd551309 *powerpc-linux.elf/upx-3.91
"
expected_sha256sums__t130_compress_ucl_nrv2e_3_no_filter="\
3fadda0299775195558de2ac25a673ab65467bd147ab8aaf57b7af1542f1d5de *amd64-linux.elf/upx-3.91
99f7d8d7faf57c78053158c6a6915dab8c5cfdcc73112cee9607bcec55837ae8 *arm-wince.pe/upx-3.91.exe
8f06e11a0c60fd83db607089b7ad985a48a26ee09bd533e55072f9fe2ba39f2f *armeb-linux.elf/upx-3.91
542c368392f2d584145cc51d51b9dbc5dc61754597bb1cb7673821a2a4650816 *i386-dos32.djgpp2.coff/upx-3.91.exe
edf61800e09e0ef963b27fded0e032839f4b0cb907a84e43d19ed31bd536440f *i386-linux.elf/upx-3.91
ef5e25c79d356e9ed0736f34dc5ee7a8f4c66d0c330b8d16672fac7d829b5a7c *i386-win32.pe/upx-3.91.exe
2c85e506d670aef707789e5d8a97901ddbb42f50a205a207f575f79b901c2cd7 *m68k-atari.tos/upx-3.91.ttp
93014cd6edd07fafcae133b449837f550f92eeab86ceebd8e7ed5337ab65bdc2 *mipsel-linux.elf/upx-3.91
761f0d45b61e020fd8931cd5192677d06b34fec64a534d99be9fe0346eb56d2b *powerpc-linux.elf/upx-3.91
"
expected_sha256sums__t140_compress_lzma_2_no_filter="\
4abe4fff006e53ee147573d74a0f7feba9ebc62b8233e19ac331c5350391bd66 *amd64-linux.elf/upx-3.91
df8ad2d951368ddfb64b8c405f4b4727a2cf5f1c823ecd6e1d298eb21cad0afc *arm-wince.pe/upx-3.91.exe
cd109f76ff00030f564a883da5a57784d9626e85c2c54e6de1f0f2cfda709325 *armeb-linux.elf/upx-3.91
deeae51e0a4c25e6f5afd741361eea445c198fba247b583663b010778699419a *i386-dos32.djgpp2.coff/upx-3.91.exe
591d02bbb604aab55ed653411ea5888d3e93c5e078f3fc79f73d54b12bc83b22 *i386-linux.elf/upx-3.91
171bde9f27a5571b524e9d7cdba6cefa142bb8a0b114c4d5294944ee5781e0bb *i386-win32.pe/upx-3.91.exe
54ca62d5f15ce87708867f7b349584799779711bca015a3ebac69f5cf077b905 *m68k-atari.tos/upx-3.91.ttp
bc9c9ce0ce7ce32e3f77fca39ffdd7809731e93f978fb201ea49b53a980c5d4e *mipsel-linux.elf/upx-3.91
4357697b615fb40d041ad5a2217a4722aa9066d2100ba2c73212262f581f820f *powerpc-linux.elf/upx-3.91
"
expected_sha256sums__t150_compress_ucl_2_all_filters="\
15dd4ad982953a1f7b7c74e7cb34e777fc7db11201280cd5af19ac547c8b9a0f *amd64-linux.elf/upx-3.91
a8cd8c9abb50ab45f30bd2f01bcb4d2e934806440ef206d7da44097aced7085b *arm-wince.pe/upx-3.91.exe
c7af209eb44204a6e4ccbed18e1b3f6eaac5df01c1106426d375d1cd65cd01fd *armeb-linux.elf/upx-3.91
ea265e1f78c23beb632187bfebcb4ae15ce688f10e87db69952ab54724666042 *i386-dos32.djgpp2.coff/upx-3.91.exe
530a267e8fdae0f4b746df62d3f2d406c6bcb08a58058448b867e372dd4a6cc6 *i386-linux.elf/upx-3.91
7c3d7398f63eb9e235992d2d8fd6de9e355f6f21621c45032a6ae6c9009067e6 *i386-win32.pe/upx-3.91.exe
bf11d38e3a2cd2c585fdb2173368287e3d82edd5727ea3508c21752b2e7c5a53 *m68k-atari.tos/upx-3.91.ttp
271adf2c52ed777c00a17a19283478e226d76181b1dfb72d6c3aead3923fbad4 *mipsel-linux.elf/upx-3.91
a0152edb1476dc0f713bcd55a7462de200e92e7d057ff4a9af92c7af6abbc3bc *powerpc-linux.elf/upx-3.91
"
expected_sha256sums__t160_compress_all_methods_1_no_filter="\
16d2a30dd00dfd38d3aba417a087a7341314fa8f5769f8c87565728f2f2bf06a *amd64-linux.elf/upx-3.91
560335ec9f89f2cb8b2a2cff51b4d3dee215c68ecfc0a182fab4a838fafee166 *arm-wince.pe/upx-3.91.exe
b3b0a0c277e947cfc19048e142e71c902cfe6d77931b2a1caeba63b9449aa8c7 *armeb-linux.elf/upx-3.91
d5f89d57c0d74923e557410e1b5ec135e3853985965bfe9fd38283851896b4c9 *i386-dos32.djgpp2.coff/upx-3.91.exe
e6586de002ae8ccda77d9088e37412c1069626e807d81722c1cd810a8a7367e6 *i386-linux.elf/upx-3.91
4c73a38e81fe12f36dc37e514f8580c12bdf5d8cb92e9a07b7070db291a2f7eb *i386-win32.pe/upx-3.91.exe
53d9857238472bf4e2822134e8d63c11d6ed32f83615bb7491ba4441ff55c6c2 *m68k-atari.tos/upx-3.91.ttp
6b11b762f1dbf07e3135a2ccfbf558d44f4743e23e73ce7c143396b26ae49c4d *mipsel-linux.elf/upx-3.91
96bcf3f5f0f5168d5cdfc01dee5d35e77e0eb8401ae6623ab559254cc99ffbdc *powerpc-linux.elf/upx-3.91
"
expected_sha256sums__t170_compress_all_methods_no_lzma_5_no_filter="\
3ded78e90066b688f820a35fe10270c2d3c00bc4c09d02f811b14a959003cd6b *amd64-linux.elf/upx-3.91
6f70c268d4686a50db2843efc58ef547048d8ed0d758bad0c95a50b3ce155a1f *arm-wince.pe/upx-3.91.exe
20acc14939776fcd2e643cea83075d84a764fb12942ce52fa0e72dc313df026b *armeb-linux.elf/upx-3.91
45ae9d42b5c6f1eec0ebc75e656c767efe796719b00922ba9e8095ffa5373450 *i386-dos32.djgpp2.coff/upx-3.91.exe
a6f13ec927d6a44fddd1af92190a530ff59282134080cd9881fef47b34e262f2 *i386-linux.elf/upx-3.91
14b192b5419ca0ec2b3f238dc6ed9c17596e82ff2674a299d4b0a76d118a73de *i386-win32.pe/upx-3.91.exe
0c341752253e2c530f3dfefc1b4b3619f931f7e23f032a7feffeb192a6a3c373 *m68k-atari.tos/upx-3.91.ttp
24887c6e98651b663cf04720bf7467501fa260a86ae8f83507669cce05d4b8bf *mipsel-linux.elf/upx-3.91
22947eb320540740b012aa53e85b99b038d5c4bf0ebcd28ea76c1c4f9018878f *powerpc-linux.elf/upx-3.91
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
        #upx_valgrind_flags="--leak-check=full --show-reachable=yes"
        #upx_valgrind_flags="-q --leak-check=no --error-exitcode=1"
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
if ! $upx_run --version; then
    echo "UPX-ERROR: upx --version FAILED"
    exit 1
fi
echo
echo "upx_exe='$upx_exe'"
if [[ "$upx_run" != "$upx_exe" ]]; then
    echo "upx_run='$upx_run'"
fi
if [[ -f "$upx_exe" ]]; then
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
