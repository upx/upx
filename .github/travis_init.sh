## vim:set ts=4 sw=4 et:

# Support for Travis CI -- https://travis-ci.org/upx/upx/builds
# Copyright (C) Markus Franz Xaver Johannes Oberhumer

#set -x # debug
umask 022
export GIT_PAGER=

# rename short variables to more readable Build-Matrix BM_ names
#   C is COMPILER
#   B is BUILD_OPTIONS
#   T is TESTSUITE_OPTIONS
#   X is EXTRA_OPTIONS
BM_CROSS=$CROSS; BM_C=$C; BM_B=$B; BM_T=$T; BM_X=$X
unset CROSS C B T X
[[ -z $BM_C ]] && BM_C=gcc
[[ -z $BM_B ]] && BM_B=release

# just in case, unset variable for passing extra UPX options
export UPX=
# un-export some vars
declare +x UPX_AUTOMATIC_BUILDS_SSL_KEY UPX_AUTOMATIC_BUILDS_SSL_IV

# compatibility wrappers
if [[ $TRAVIS_OS_NAME == osx ]]; then
# use GNU coreutils ("brew install coreutils")
date() {
    gdate "$@"
}
readlink() {
    greadlink "$@"
}
sha256sum() {
    gsha256sum "$@"
}
fi
if [[ -n $APPVEYOR_JOB_ID ]]; then
openssl() {
    /usr/bin/openssl "$@"
}
sort() {
    /usr/bin/sort "$@"
}
fi

# set CC and CXX from BM_C and BM_CROSS
if [[ -z $CC_OVERRIDE ]]; then
CC=false CXX=false SCAN_BUILD=false
AR=ar SIZE=size
if [[ -n $APPVEYOR_JOB_ID ]]; then
    BUILD_LOCAL_UCL=1
    BUILD_LOCAL_ZLIB=1
    if [[ $BM_C =~ (^|\-)(clang|gcc)($|\-) ]]; then
        export upx_EXTRA_LDFLAGS="-static-libgcc -static-libstdc++"
    fi
    upx_exeext=.exe
    # dir c:\cygwin
    case $BM_C in
        gcc-m32 | gcc-4.9-m32)
            x=i686-w64-mingw32; AR="$x-ar"; CC="$x-gcc -m32"; CXX="$x-g++ -m32" ;;
        gcc-m64 | gcc-4.9-m64)
            x=x86_64-w64-mingw32; AR="$x-ar"; CC="$x-gcc -m64"; CXX="$x-g++ -m64" ;;
        msvc | msvc-*)
            AR="link -lib"; CC="cl"; CXX="cl" ;; # standard system compiler
    esac
fi # APPVEYOR_JOB_ID
if [[ -n $BM_CROSS ]]; then
    BUILD_LOCAL_UCL=1
    BUILD_LOCAL_ZLIB=1
    if [[ $BM_C =~ (^|\-)(clang|gcc)($|\-) ]]; then
        export upx_EXTRA_LDFLAGS="-static-libgcc -static-libstdc++"
    fi
    cat /etc/os-release || true
    if egrep -q '^PRETTY_NAME="?Ubuntu .*12\.04' /etc/os-release; then
        case $BM_CROSS-$BM_C in
            arm-linux-gnueabi-gcc | arm-linux-gnueabi-gcc-4.6)
                [[ -z $upx_qemu ]] && upx_qemu="qemu-arm -L /usr/arm-linux-gnueabi"
                x=arm-linux-gnueabi; AR="$x-ar"; CC="$x-gcc"; CXX="$x-g++" ;;
            arm-linux-gnueabihf-gcc | arm-linux-gnueabihf-gcc-4.6)
                [[ -z $upx_qemu ]] && upx_qemu="qemu-arm -L /usr/arm-linux-gnueabihf"
                x=arm-linux-gnueabihf; AR="$x-ar"; CC="$x-gcc"; CXX="$x-g++" ;;
            i[36]86-w64-mingw32-gcc | i[36]86-w64-mingw32-gcc-4.6)
                upx_exeext=.exe
                [[ -z $upx_wine ]] && upx_wine="wine"
                x=i686-w64-mingw32; AR="$x-ar"; CC="$x-gcc -m32"; CXX="$x-g++ -m32" ;;
            x86_64-w64-mingw32-gcc | x86_64-w64-mingw32-gcc-4.6)
                upx_exeext=.exe
                [[ -z $upx_wine ]] && upx_wine="wine"
                x=x86_64-w64-mingw32; AR="$x-ar"; CC="$x-gcc -m64"; CXX="$x-g++ -m64" ;;
        esac
    elif egrep -q '^PRETTY_NAME="?Ubuntu 16\.04' /etc/os-release; then
        case $BM_CROSS-$BM_C in
            aarch64-linux-gnu-gcc-5)
                [[ -z $upx_qemu ]] && upx_qemu="qemu-aarch64 -L /usr/aarch64-linux-gnu"
                x=aarch64-linux-gnu; AR="$x-ar"; CC="$x-gcc-5"; CXX="$x-g++-5" ;;
            arm-linux-gnueabi-gcc-5)
                [[ -z $upx_qemu ]] && upx_qemu="qemu-arm -L /usr/arm-linux-gnueabi"
                x=arm-linux-gnueabi; AR="$x-ar"; CC="$x-gcc-5"; CXX="$x-g++-5" ;;
            arm-linux-gnueabihf-gcc-5)
                [[ -z $upx_qemu ]] && upx_qemu="qemu-arm -L /usr/arm-linux-gnueabihf"
                x=arm-linux-gnueabihf; AR="$x-ar"; CC="$x-gcc-5"; CXX="$x-g++-5" ;;
            i[36]86-w64-mingw32-gcc-5)
                upx_exeext=.exe
                [[ -z $upx_wine ]] && upx_wine="wine"
                x=i686-w64-mingw32; AR="$x-ar"; CC="$x-gcc -m32"; CXX="$x-g++ -m32" ;;
            mips-linux-gnu-gcc-5)
                [[ -z $upx_qemu ]] && upx_qemu="qemu-mips -L /usr/mips-linux-gnu"
                x=mips-linux-gnu; AR="$x-ar"; CC="$x-gcc-5"; CXX="$x-g++-5" ;;
            mipsel-linux-gnu-gcc-5)
                [[ -z $upx_qemu ]] && upx_qemu="qemu-mipsel -L /usr/mipsel-linux-gnu"
                x=mipsel-linux-gnu; AR="$x-ar"; CC="$x-gcc-5"; CXX="$x-g++-5" ;;
            powerpc-linux-gnu-gcc-5)
                [[ -z $upx_qemu ]] && upx_qemu="qemu-ppc -L /usr/powerpc-linux-gnu"
                x=powerpc-linux-gnu; AR="$x-ar"; CC="$x-gcc-5"; CXX="$x-g++-5" ;;
            powerpc64-linux-gnu-gcc-5)
                [[ -z $upx_qemu ]] && upx_qemu="qemu-ppc64 -L /usr/powerpc64-linux-gnu"
                x=powerpc64-linux-gnu; AR="$x-ar"; CC="$x-gcc-5"; CXX="$x-g++-5" ;;
            powerpc64le-linux-gnu-gcc-5)
                [[ -z $upx_qemu ]] && upx_qemu="qemu-ppc64le -L /usr/powerpc64le-linux-gnu"
                x=powerpc64le-linux-gnu; AR="$x-ar"; CC="$x-gcc-5"; CXX="$x-g++-5" ;;
            s390x-linux-gnu-gcc-5)
                [[ -z $upx_qemu ]] && upx_qemu="qemu-s390x -L /usr/s390x-linux-gnu"
                x=s390x-linux-gnu; AR="$x-ar"; CC="$x-gcc-5"; CXX="$x-g++-5" ;;
            x86_64-w64-mingw32-gcc-5)
                upx_exeext=.exe
                [[ -z $upx_wine ]] && upx_wine="wine"
                x=x86_64-w64-mingw32; AR="$x-ar"; CC="$x-gcc -m64"; CXX="$x-g++ -m64" ;;
        esac
    fi
fi # BM_CROSS
if [[ "$CC" == "false" ]]; then # generic
if [[ -z $BM_CROSS ]]; then
    case $BM_C in
        clang | clang-m?? | clang-3.4-m?? | clang-[678][0-9][0-9]-m??)
            CC="clang"; CXX="clang++" ;; # standard system compiler
        clang-[3].[0-9]-m??)
            v=${BM_C:6:3}; CC="clang-$v"; CXX="clang++-$v"; SCAN_BUILD="scan-build-$v" ;;
        gcc | gcc-m?? | gcc-mx32)
            CC="gcc"; CXX="g++" ;; # standard system compiler
        gcc-[34].[0-9]-m?? | gcc-[34].[0-9]-mx32)
            v=${BM_C:4:3}; CC="gcc-$v"; CXX="g++-$v" ;;
        gcc-[56]-m?? | gcc-[56]-mx32)
            v=${BM_C:4:1}; CC="gcc-$v"; CXX="g++-$v" ;;
    esac
fi
fi # generic
case $BM_C in
    clang*-m32) CC="$CC -m32";  CXX="$CXX -m32" ;;
    clang*-m64) CC="$CC -m64";  CXX="$CXX -m64" ;;
    gcc*-m32)   CC="$CC -m32";  CXX="$CXX -m32" ;;
    gcc*-m64)   CC="$CC -m64";  CXX="$CXX -m64" ;;
    gcc*-mx32)  CC="$CC -mx32"; CXX="$CXX -mx32"; BUILD_LOCAL_UCL=1; BUILD_LOCAL_ZLIB=1 ;;
esac
if [[ $BM_C =~ (^|\-)(clang|gcc)($|\-) ]]; then
    CC="$CC -std=gnu89"
fi
unset v x
export AR CC CXX
fi # CC_OVERRIDE

# source dirs
[[ -z $upx_SRCDIR ]] && upx_SRCDIR=$(readlink -mn -- $argv0dir/..)
[[ -z $ucl_SRCDIR ]] && ucl_SRCDIR=$(readlink -mn -- $upx_SRCDIR/../deps/ucl-1.03)
[[ -z $upx_testsuite_SRCDIR ]] && upx_testsuite_SRCDIR=$(readlink -mn -- $upx_SRCDIR/../deps/upx-testsuite)
[[ -z $zlib_SRCDIR ]] && zlib_SRCDIR=$(readlink -mn -- $upx_SRCDIR/../deps/zlib-1.2.8)

# build dirs
mkbuilddirs() {
    local d
    for d in "$@"; do
        mkdir -p -v "$d" || exit 1
        [[ -f "$d/.mfxnobackup" ]] || touch "$d/.mfxnobackup"
    done
}
# search for an existing $toptop_builddir
if [[ -z $toptop_builddir ]]; then
    for d in . ..;  do
        for subdir in "local" appveyor circle gitlab travis .; do
            dd=$d/build/$subdir
            if [[ -d $dd ]]; then
                toptop_builddir=$(readlink -en -- "$dd")
                break 2
            fi
        done
    done
    unset d subdir dd
fi
[[ -z $toptop_builddir ]] && toptop_builddir=$(readlink -mn -- ./build)
[[ -z $toptop_bdir ]] && toptop_bdir=$(readlink -mn -- "$toptop_builddir/${BM_CROSS:+$BM_CROSS-}$BM_C/$BM_B")
[[ -z $upx_BUILDDIR ]] && upx_BUILDDIR=$(readlink -mn -- "$toptop_bdir/upx")
[[ -z $ucl_BUILDDIR ]] && ucl_BUILDDIR=$(readlink -mn -- "$toptop_bdir/ucl-1.03")
[[ -z $upx_testsuite_BUILDDIR ]] && upx_testsuite_BUILDDIR=$(readlink -mn -- "$toptop_bdir/upx-testsuite")
[[ -z $zlib_BUILDDIR ]] && zlib_BUILDDIR=$(readlink -mn -- "$toptop_bdir/zlib-1.2.8")
[[ -z $lcov_OUTPUTDIR ]] && lcov_OUTPUTDIR=$(readlink -mn -- "$toptop_bdir/.lcov-results")
unset toptop_builddir toptop_bdir

# ensure absolute directories
make_absolute() {
    local d
    while [[ $# -gt 0 ]]; do
        if [[ -n ${!1} ]]; then
            d=$(readlink -mn -- "${!1}")
            eval $1="$d"
        fi
        shift
    done
}
for var_prefix in ucl upx upx_testsuite zlib; do
    for var_suffix in _BUILDDIR _SRCDIR; do
        make_absolute ${var_prefix}${var_suffix}
    done
done
make_absolute lcov_OUTPUTDIR
unset var_prefix var_suffix

print_header() {
    local x="==========="; x="$x$x$x$x$x$x$x"
    echo -e "\n${x}\n${1}\n${x}\n"
}

print_settings() {
    local v var_prefix var_suffix
    # Build Matrix
    for v in TRAVIS_OS_NAME BM_CROSS BM_C BM_B BM_T; do
        [[ -n ${!v} ]] && echo "${v}='${!v}'"
    done
    # BM_C related
    for v in AR CC CXX CPPFLAGS CFLAGS CXXFLAGS LDFLAGS LIBS SCAN_BUILD; do
        [[ -n ${!v} ]] && echo "${v}='${!v}'"
        v=EXTRA_${v}
        [[ -n ${!v} ]] && echo "${v}='${!v}'"
    done
    # directories and other info
    for v in TRAVIS_XCODE_SDK UPX_UCLDIR lcov_OUTPUTDIR; do
        [[ -n ${!v} ]] && echo "${v}='${!v}'"
    done
    for var_prefix in ucl upx upx_testsuite zlib; do
    for var_suffix in _BUILDDIR _SRCDIR; do
        v=${var_prefix}${var_suffix}
        [[ -n ${!v} ]] && echo "${v}='${!v}'"
    done
    done
    ##env | LC_ALL=C sort
}

true
