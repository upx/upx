## vim:set ts=4 sw=4 et:

# Support for Travis CI -- https://travis-ci.org/upx/upx/builds
# Copyright (C) Markus Franz Xaver Johannes Oberhumer

#set -x # debug
umask 022

# rename short Build-Matrix variables to more readable names
#   C is COMPILER
#   B is BUILD_TYPE
#   T is TESTSUITE_FLAGS
BM_C=$C; BM_B=$B; BM_CROSS=$CROSS; BM_T=$T
unset C B CROSS T
[[ -z $BM_C ]] && BM_C=gcc
[[ -z $BM_B ]] && BM_B=release

# just in case, unset variable for passing extra UPX options
UPX=

# compatibility wrappers
if [[ $TRAVIS_OS_NAME == osx ]]; then
# use GNU coreutils ("brew install coreutils")
readlink() {
    greadlink "$@"
}
sha256sum() {
    gsha256sum "$@"
}
fi
if [[ -n $APPVEYOR_JOB_ID ]]; then
# for some reason this is needed for bash on AppVeyor
sort() {
    /usr/bin/sort "$@"
}
fi

# set CC and CXX from BM_C
if [[ -z $CC_OVERRIDE ]]; then
CC=false CXX=false SCAN_BUILD=false
AR=ar SIZE=size
if [[ -n $APPVEYOR_JOB_ID ]]; then
    BUILD_LOCAL_ZLIB=1
    export exeext=.exe
    # dir c:\cygwin
    case $BM_C in
        gcc-m32 | gcc-4.9-m32)
            export upx_EXTRA_LDFLAGS="-static-libgcc -static-libstdc++"
            x=i686-w64-mingw32; AR="$x-ar"; CC="$x-gcc -m32"; CXX="$x-g++ -m32" ;;
        gcc-m64 | gcc-4.9-m64)
            export upx_EXTRA_LDFLAGS="-static-libgcc -static-libstdc++"
            x=x86_64-w64-mingw32; AR="$x-ar"; CC="$x-gcc -m64"; CXX="$x-g++ -m64" ;;
        msvc | msvc-*)
            AR="link -lib"; CC="cl"; CXX="cl" ;; # standard system compiler
    esac
fi # APPVEYOR_JOB_ID
if [[ -n $CIRCLE_BUILD_NUM ]]; then
    case $BM_C in
        gcc | gcc-m??)
            CC="gcc"; CXX="g++" ;; # standard system compiler
        gcc-[34].[0-9]-m??)
            v=${BM_C:4:3}; CC="gcc-$v"; CXX="g++-$v" ;;
    esac
fi # $CIRCLE_BUILD_NUM
if [[ -n $TRAVIS_JOB_ID ]]; then
if [[ -n $BM_CROSS ]]; then
    BUILD_LOCAL_ZLIB=1
    case $BM_CROSS-$BM_C in
        arm-linux-gnueabi-gcc | arm-linux-gnueabi-gcc-4.6)
            export upx_EXTRA_LDFLAGS="-static-libgcc -static-libstdc++"
            [[ -z $upx_qemu ]] && upx_qemu="qemu-arm-static -L/usr/arm-linux-gnueabi"
            x=arm-linux-gnueabi; AR="$x-ar"; CC="$x-gcc"; CXX="$x-g++" ;;
        arm-linux-gnueabihf-gcc | arm-linux-gnueabihf-gcc-4.6)
            export upx_EXTRA_LDFLAGS="-static-libgcc -static-libstdc++"
            [[ -z $upx_qemu ]] && upx_qemu="qemu-arm -L/usr/arm-linux-gnueabihf"
            x=arm-linux-gnueabihf; AR="$x-ar"; CC="$x-gcc"; CXX="$x-g++" ;;
        i[36]86-w64-mingw32-gcc | i[36]86-w64-mingw32-gcc-4.6)
            export upx_EXTRA_LDFLAGS="-static-libgcc -static-libstdc++"
            [[ -z $upx_wine ]] && upx_wine="wine"
            x=i686-w64-mingw32; AR="$x-ar"; CC="$x-gcc -m32"; CXX="$x-g++ -m32" ;;
        x86_64-w64-mingw32-gcc | x86_64-w64-mingw32-gcc-4.6)
            export upx_EXTRA_LDFLAGS="-static-libgcc -static-libstdc++"
            [[ -z $upx_wine ]] && upx_wine="wine"
            x=x86_64-w64-mingw32; AR="$x-ar"; CC="$x-gcc -m64"; CXX="$x-g++ -m64" ;;
    esac
fi
fi # TRAVIS_JOB_ID
if [[ "$CC" == "false" ]]; then # generic
if [[ -z $BM_CROSS ]]; then
    case $BM_C in
        clang | clang-m?? | clang-3.4-m?? | clang-[678][0-9][0-9]-m??)
            CC="clang"; CXX="clang++" ;; # standard system compiler
        clang-[34].[0-9]-m??)
            v=${BM_C:6:3}; CC="clang-$v"; CXX="clang++-$v"; SCAN_BUILD="scan-build-$v" ;;
        gcc | gcc-m??)
            CC="gcc"; CXX="g++" ;; # standard system compiler
        gcc-[34].[0-9]-m??)
            v=${BM_C:4:3}; CC="gcc-$v"; CXX="g++-$v" ;;
        gcc-[56]-m?? | gcc-[56].[0-9]-m??)
            v=${BM_C:4:1}; CC="gcc-$v"; CXX="g++-$v" ;;
    esac
fi
fi # generic
case $BM_C in
    clang*-m32) CC="$CC -m32"; CXX="$CXX -m32" ;;
    clang*-m64) CC="$CC -m64"; CXX="$CXX -m64" ;;
    gcc*-m32)   CC="$CC -m32"; CXX="$CXX -m32" ;;
    gcc*-m64)   CC="$CC -m64"; CXX="$CXX -m64" ;;
esac
case $BM_C in
    clang* | gcc*) CC="$CC -std=gnu89" ;;
esac
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
# search for an existing toptop builddir
for d in ./build/travis ./build ../build/travis ../build; do
    [[ -z $toptop_builddir && -d $d ]] && toptop_builddir=$(readlink -mn -- "$d")
done
[[ -z $toptop_builddir ]] && toptop_builddir=$(readlink -mn -- ./build)
[[ -z $toptop_bdir ]] && toptop_bdir=$(readlink -mn -- "$toptop_builddir/$BM_C/$BM_B")

[[ -z $upx_BUILDDIR ]] && upx_BUILDDIR=$(readlink -mn -- "$toptop_bdir/upx")
[[ -z $ucl_BUILDDIR ]] && ucl_BUILDDIR=$(readlink -mn -- "$toptop_bdir/ucl-1.03")
[[ -z $upx_testsuite_BUILDDIR ]] && upx_testsuite_BUILDDIR=$(readlink -mn -- "$toptop_bdir/upx-testsuite")
[[ -z $zlib_BUILDDIR ]] && zlib_BUILDDIR=$(readlink -mn -- "$toptop_bdir/zlib-1.2.8")

[[ -z $lcov_OUTPUTDIR ]] && lcov_OUTPUTDIR=$(readlink -mn -- "$toptop_bdir/.lcov-results")
unset toptop_builddir toptop_bdir

# ensure absolute dirs
for var_prefix in ucl upx upx_testsuite zlib; do
for var_suffix in _BUILDDIR _SRCDIR; do
    var_name=${var_prefix}${var_suffix}
    if [[ -n ${!var_name} ]]; then
        d=$(readlink -mn -- "${!var_name}")
        eval $var_name="$d"
    fi
done
done
unset var_name var_prefix var_suffix

print_settings() {
    local v var_prefix var_suffix
    # Build Matrix
    for v in TRAVIS_OS_NAME BM_C BM_B BM_CROSS BM_T; do
        [[ -n ${!v} ]] && echo "${v}='${!v}'"
    done
    # BM_C related
    for v in AR CC CXX CPPFLAGS CFLAGS CXXFLAGS LDFLAGS LIBS SCAN_BUILD; do
        [[ -n ${!v} ]] && echo "${v}='${!v}'"
        v=EXTRA_${v}
        [[ -n ${!v} ]] && echo "${v}='${!v}'"
    done
    # dirs and other info
    for v in TRAVIS_XCODE_SDK UPX_UCLDIR; do
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
