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
52d0189f9ed16162332fbe359d723753ca1b7d7374a249d6b16144e363073384 *amd64-linux.elf/upx-3.91
0af0b3df9ca5b34ebc8ed06b3bf46807e585032961f053abfeefcb60bd63907e *arm-wince.pe/upx-3.91.exe
851ea686cc1d6ef7c63978829f0bfe40c761d00e9f07c3d4f9ce59201da4409c *armeb-linux.elf/upx-3.91
3345fda466927211ed3291ea1255fa09937fcfad974ffb1120deeefe6f655a9e *i386-dos32.djgpp2.coff/upx-3.91.exe
8631629945812b6766da0187390b4d26e830f39a79d80aacda196e678a4be10c *i386-linux.elf/upx-3.91
06794bdb9a8085ca5ef577648bf39402868a507f2a0bd02285c807929499aba4 *i386-win32.pe/upx-3.91.exe
6c9e164baa12c45ca8d55788b34e8fabdddb52374200444d42ff48c1d84103eb *m68k-atari.tos/upx-3.91.ttp
1f3ae2bde7c72e60d3c12fccf572c5aa5dbee566d1d28029e62a8b921c6d77bd *mipsel-linux.elf/upx-3.91
181345ff1dd1c35944ec79d1197ee223913617cdd148a576cd3b9b776b8e86c2 *powerpc-linux.elf/upx-3.91
"
expected_sha256sums__t120_compress_ucl_nrv2d_3_no_filter="\
d90f1e94fb8a1d5c123e1c13b30db148870c36ff6f00fc5677bd9f05bae1fc02 *amd64-linux.elf/upx-3.91
e09a089e642f33502a35ab9a416142d3db4dbb67930366054e25c390678c3f7c *arm-wince.pe/upx-3.91.exe
b794cc520ab507887324fe04e1c36e1ed0b63ecdb29241213a3a3cf980e16cc2 *armeb-linux.elf/upx-3.91
ea5df377a14457fc4a4791e67859f77f8fe8b94ed81fd13666618fc690d70987 *i386-dos32.djgpp2.coff/upx-3.91.exe
5f0876049576446707270baa651eaa724891cccbee268e18450812d5be5e9990 *i386-linux.elf/upx-3.91
4a65a19b430d50b9f15786342e3de86feb3627af07743b50a9ebfedb10c59a08 *i386-win32.pe/upx-3.91.exe
d7514121a811a36112cb2f3e42c3284e03a7807e972778229c62c075656d796f *m68k-atari.tos/upx-3.91.ttp
0effcd336be6dcb092e37bafc9611c99c1d19c1c53a8ca6a26834d9f5f9eb56c *mipsel-linux.elf/upx-3.91
dfe218814633aa9ed47477bf021ac0bae0b491ebf9d4ff44194f3d7195e5a200 *powerpc-linux.elf/upx-3.91
"
expected_sha256sums__t130_compress_ucl_nrv2e_3_no_filter="\
00b67a1cc766b13a6167e0f672cf46b62f0879b6318ed9f548f670b011b3f185 *amd64-linux.elf/upx-3.91
a377c263a3d981af1ec278a418fcdd09298d76cece0e7c7c9af63f20ccba769d *arm-wince.pe/upx-3.91.exe
dbf2fa9b4fa5319c52b25d34aa6f57577b2a63e7241b6d4600915de0f3400190 *armeb-linux.elf/upx-3.91
4d925cff00b8959324c0838284f4619831b8080136cef05131906149fdec5bbc *i386-dos32.djgpp2.coff/upx-3.91.exe
b114b257ff5de11b75eb4489d2cb2bd3be6cf9cf141e36f02b0ab781d0f9a3ff *i386-linux.elf/upx-3.91
ce874eafe18c0f09928992a3aa1f5fe9f0be9d89b87b39ecbcf1c460e4b8e0f7 *i386-win32.pe/upx-3.91.exe
c60b3faccc59ff37c2e1e3c908d4fe01a2532e7ac1abd4c4ec4ecee974a0e8bd *m68k-atari.tos/upx-3.91.ttp
2b74a6a831c7f9f4df22c65f8a5748a2b6cf6fd68232b0d250737938de27efc6 *mipsel-linux.elf/upx-3.91
47ddad5acc39e21d66e471c8af8d9b52e8dd8d7ad89631f771c2ea4d218f441e *powerpc-linux.elf/upx-3.91
"
expected_sha256sums__t140_compress_lzma_2_no_filter="\
f67ce2b6d4d36367a249faeb802df51dbc56019754c0f4606b5a6f97b72a54d1 *amd64-linux.elf/upx-3.91
57be6aa4f1e5ee6ef6f5eb8694b22eb07c3ea5cc9b9ebfde75db4c9a04a71a5a *arm-wince.pe/upx-3.91.exe
ef18c1ae56a037d15f0494f4b835c1336a43745edba87c783f62745488008783 *armeb-linux.elf/upx-3.91
54a6158bc6bd8cc28d3abc53cc5cf9b63c3c63ae884e96fb6d6ea974c7426206 *i386-dos32.djgpp2.coff/upx-3.91.exe
801d1761289f94d7f51effb88e100832d14ae96398844084b9d10852886c4731 *i386-linux.elf/upx-3.91
f828953a66d7b3394dd2514b5972680a0cc651bd198b7607f6d3f6f6cc390a0e *i386-win32.pe/upx-3.91.exe
587b5f1f529da60f5820b8f811743aaad59e3fa9407930de1ab433863f7262a4 *m68k-atari.tos/upx-3.91.ttp
84c811f5d27bac393610bfad4180258494c43d691c13eb57394ab95d4736e621 *mipsel-linux.elf/upx-3.91
ed83830ab5e1753a742cf4981401fd8d5bdaabcfdeb385a2e8087aeb46f93512 *powerpc-linux.elf/upx-3.91
"
expected_sha256sums__t150_compress_ucl_2_all_filters="\
2849771c8799d7a446b54ad9914d679e315a9f0d9440f9728c6d5ab6541ed494 *amd64-linux.elf/upx-3.91
7ccbe7d7822cd9fed9b3c902c2bcb80f85df4f5bbb4e9e850257435ffa375a38 *arm-wince.pe/upx-3.91.exe
c6849deffc7bb1976c18a4815bca0b8fd0b6d19e4136965108daa6382efae2d5 *armeb-linux.elf/upx-3.91
c04818c54d68c99f345ee7995634dffbe7f1c2f2db8f2b510bba461d9125f5a9 *i386-dos32.djgpp2.coff/upx-3.91.exe
a3758350a0454fb78060b6d6e48d8f000e17a628b6041e8c82e3990e0b3aabe9 *i386-linux.elf/upx-3.91
b4f27ba234a5c9b4f68e2067602a180c02f95fb044830f54a45267fe1c50bb83 *i386-win32.pe/upx-3.91.exe
dd9fb59d57a7157af1a44559f6ebd8a293b0988c66b4d0951c949f88f814bf8f *m68k-atari.tos/upx-3.91.ttp
c0bc81487114b9d158ffe1dbd4081b9e3636695c63f11b3e204033b56fc69706 *mipsel-linux.elf/upx-3.91
0a700713f0c7ff19a534963f5ab95a63d2f185f82d0c42ce8c1a485aba4c1147 *powerpc-linux.elf/upx-3.91
"
expected_sha256sums__t160_compress_all_methods_1_no_filter="\
831b0dd82d2c8fb058e8300bda5dd51657a709e76c45e0c372d9cf83c2e06f31 *amd64-linux.elf/upx-3.91
057b08389af5a31a0bf4baf03d09eb07f4698f8ddec615ae35795cec62132966 *arm-wince.pe/upx-3.91.exe
49b6b8d4f04d9078686372a315e7768fc882ad66bebb67225fa0b2255b5dd1c9 *armeb-linux.elf/upx-3.91
aef085cb86ef29d7aa380a3daea403f53659af1615ceeaa76e609dc793f50cfd *i386-dos32.djgpp2.coff/upx-3.91.exe
52087d4a2c17445b0e8d5238ec3abd2ca5cd909513cff76bf42232f33d93903f *i386-linux.elf/upx-3.91
e669d7c4fec901130dd387dc8f95e3b3f5e739d6ef0bfac10080dbd257084ae6 *i386-win32.pe/upx-3.91.exe
40cd5a94039a1703986a68dd404290aa7c27fd8c184fa4c651467b8c2d621a5d *m68k-atari.tos/upx-3.91.ttp
b1db1d04e350972aa06216ec43d12a451abf5c928c885b0c442f9436c804e84e *mipsel-linux.elf/upx-3.91
65602c3e1e3e152d44d9f803281d4021bfd91aab825b72d8901faf4d7d9b593e *powerpc-linux.elf/upx-3.91
"
expected_sha256sums__t170_compress_all_methods_no_lzma_5_no_filter="\
bce397f12372437802d887d203bb2f8e4755dbe1a0e3c6bb9fa9cccd3306eef7 *amd64-linux.elf/upx-3.91
84b1fe14dbff7d4e2f1bcfee9b6e3c73f68eb5b95b37cbf2c655340bd959bfa3 *arm-wince.pe/upx-3.91.exe
1b6ac9c0893b3093d47e403a14a5d346ae3a0f135ecdbe3bb04fc0542abd469b *armeb-linux.elf/upx-3.91
d27237cd4b55c80d2b6c7da8d70ffc4f390290fcc550eec06c795e92d934b72a *i386-dos32.djgpp2.coff/upx-3.91.exe
2032623990d22b2cf8df5f2c0b581de5fd53869869b65818ea023a6857c4eff0 *i386-linux.elf/upx-3.91
51e91ff8226517ad8cd06d240e2c6c2f3d3e4033af1e593a5b294356d1bb5d33 *i386-win32.pe/upx-3.91.exe
b371684bbb2693797dd1164159de3bff9618bf6b99536542f57da07093dbb649 *m68k-atari.tos/upx-3.91.ttp
7c1fa54b86f3e5f3410216aa627bd32680d7bd8210776a14762566cb2ea37d52 *mipsel-linux.elf/upx-3.91
5c60d0817feeb4dde55be90e552ccddc81591fbaecb899c3a21afc91c0ceb3b9 *powerpc-linux.elf/upx-3.91
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
