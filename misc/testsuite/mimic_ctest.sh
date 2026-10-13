#! /usr/bin/env bash
## vim:set ts=4 sw=4 et:
set -e; set -o pipefail
argv0=$0; argv0abs=$(readlink -fn "$argv0"); argv0dir=$(dirname "$argv0abs")

#
# Copyright (C) Markus Franz Xaver Johannes Oberhumer
#
# mimic running "ctest", i.e. the "test" section of CMakeLists.txt
#   - does not redirect stdout
#   - allows freely setting $upx_exe_runner, while CMake is restricted to configure-time settings
#
# requires:
#   $upx_exe                (required, but with convenience fallback "./upx")
# optional settings:
#   $upx_exe_runner         (e.g. "qemu-x86_64 -cpu Nehalem" or "valgrind")
#

# Debugging aid for locating failing commands.  Depends on 'bash' shell.
# Notice single-quoting of entire first argument.
trap '  # Diagnostic at non-zero exit of process invoked by 'bash' shell
    echo  # A blank line to separate from other output

    echo FAILED in directory $PWD

    # A shell list of words separated by $IFS:
    argv=( $( eval echo ${BASH_COMMAND[@]} ) )

    echo Command: "${argv[*]}"

    # Search the $PATH
    echo PATH="$PATH"
    type ${argv[0]}
    echo LINENO= $LINENO "(can be wrong)"
' ERR

#***********************************************************************
# init & checks
#***********************************************************************

# upx_exe
[[ -z $upx_exe && -f ./upx && -x ./upx ]] && upx_exe=./upx # convenience fallback
if [[ -z $upx_exe ]]; then echo "UPX-ERROR: please set \$upx_exe"; exit 1; fi
if [[ ! -f $upx_exe ]]; then echo "UPX-ERROR: file '$upx_exe' does not exist"; exit 1; fi
upx_exe=$(readlink -fn "$upx_exe") # make absolute
[[ -f $upx_exe ]] || exit 1

# set emu and run_upx
emu=()
if [[ -n $upx_exe_runner ]]; then
    # usage examples:
    #   export upx_exe_runner="qemu-x86_64 -cpu Nehalem"
    #   export upx_exe_runner="valgrind --leak-check=no --error-exitcode=1 --quiet"
    #   export upx_exe_runner="wine"
    IFS=' ' read -r -a emu <<< "$upx_exe_runner" # split at spaces into array
elif [[ -n $CMAKE_CROSSCOMPILING_EMULATOR ]]; then
    IFS=';' read -r -a emu <<< "$CMAKE_CROSSCOMPILING_EMULATOR" # split at semicolons into array
fi
run_upx=( "${emu[@]}" "$upx_exe" )
echo "run_upx='${run_upx[*]}'"

# run_upx sanity check
"${run_upx[@]}" --version-short || true
if ! "${run_upx[@]}" --version-short >/dev/null; then echo "UPX-ERROR: FATAL: upx --version-short FAILED"; exit 1; fi
if ! "${run_upx[@]}" -L >/dev/null 2>&1; then echo "UPX-ERROR: FATAL: upx -L FAILED"; exit 1; fi
if ! "${run_upx[@]}" --help >/dev/null;  then echo "UPX-ERROR: FATAL: upx --help FAILED"; exit 1; fi

#***********************************************************************
# see CMakeLists.txt
#
# IDEA: create a Makefile and use "make -j8" so that these tests can
#   run in parallel much like "ctest --parallel 8"
#***********************************************************************

# similar to cmake function upx_cache_bool_vars()
set_cmake_bool_vars() {
    local default_value="$1"; shift
    local var_name
    for var_name do
        case "${!var_name}" in
            0 | FALSE | OFF | false | off) eval "export $var_name=OFF" ;;
            1 | TRUE | ON | true | on) eval "export $var_name=ON" ;;
            *) eval "export $var_name=$default_value" ;;
        esac
    done
}

set -x
set_cmake_bool_vars OFF UPX_CONFIG_DISABLE_SELF_PACK_TEST
set_cmake_bool_vars OFF UPX_CONFIG_DISABLE_RUN_UNPACKED_TEST
set_cmake_bool_vars OFF UPX_CONFIG_DISABLE_RUN_PACKED_TEST
if [[ "${emu[0]}" == *valgrind* ]]; then # valgrind is SLOW
    set_cmake_bool_vars ON  UPX_CONFIG_DISABLE_EXHAUSTIVE_TESTS
else
    set_cmake_bool_vars OFF UPX_CONFIG_DISABLE_EXHAUSTIVE_TESTS
fi

export UPX="--prefer-ucl --no-color --no-progress"
export UPX_DEBUG_DISABLE_GITREV_WARNING=1
export UPX_DEBUG_DOCTEST_DISABLE=1 # already checked above

"${run_upx[@]}" --version
"${run_upx[@]}" --version-short
"${run_upx[@]}" --license
"${run_upx[@]}" --help
"${run_upx[@]}" --help-short
"${run_upx[@]}" --help-verbose
"${run_upx[@]}" --sysinfo
"${run_upx[@]}" --sysinfo -v
"${run_upx[@]}" --sysinfo -vv

if [[ $UPX_CONFIG_DISABLE_SELF_PACK_TEST == ON ]]; then
    echo "Self-pack test disabled. All done."; exit 0
fi

exe=".out"
upx_self_exe=$upx_exe
fo="--force-overwrite"

"${run_upx[@]}" -3               "${upx_self_exe}" ${fo} -o upx-packed${exe}
"${run_upx[@]}" -3 --all-filters "${upx_self_exe}" ${fo} -o upx-packed-fa${exe}
"${run_upx[@]}" -3 --no-filter   "${upx_self_exe}" ${fo} -o upx-packed-fn${exe}
"${run_upx[@]}" -3 --all-filters --debug-use-random-filter "${upx_self_exe}" ${fo} -o upx-packed-fr${exe}
"${run_upx[@]}" -3 --nrv2b       "${upx_self_exe}" ${fo} -o upx-packed-nrv2b${exe}
"${run_upx[@]}" -3 --nrv2d       "${upx_self_exe}" ${fo} -o upx-packed-nrv2d${exe}
"${run_upx[@]}" -3 --nrv2e       "${upx_self_exe}" ${fo} -o upx-packed-nrv2e${exe}
"${run_upx[@]}" -1 --lzma        "${upx_self_exe}" ${fo} -o upx-packed-lzma${exe}

packed_exact=()
if "${run_upx[@]}" --fileinfo "${upx_self_exe}" | grep -E '\[[^]]*-win(32|64)\.pe,' >/dev/null; then
    "${run_upx[@]}" -3 --exact "${upx_self_exe}" ${fo} -o upx-packed-exact${exe}
    packed_exact=(upx-packed-exact${exe})
fi

"${run_upx[@]}" -l         upx-packed${exe} upx-packed-fa${exe} upx-packed-fn${exe} upx-packed-fr${exe} upx-packed-nrv2b${exe} upx-packed-nrv2d${exe} upx-packed-nrv2e${exe} upx-packed-lzma${exe} "${packed_exact[@]}"
"${run_upx[@]}" --fileinfo upx-packed${exe} upx-packed-fa${exe} upx-packed-fn${exe} upx-packed-fr${exe} upx-packed-nrv2b${exe} upx-packed-nrv2d${exe} upx-packed-nrv2e${exe} upx-packed-lzma${exe} "${packed_exact[@]}"
"${run_upx[@]}" -t         upx-packed${exe} upx-packed-fa${exe} upx-packed-fn${exe} upx-packed-fr${exe} upx-packed-nrv2b${exe} upx-packed-nrv2d${exe} upx-packed-nrv2e${exe} upx-packed-lzma${exe} "${packed_exact[@]}"

"${run_upx[@]}" -d upx-packed${exe}       ${fo} -o upx-unpacked${exe}
"${run_upx[@]}" -d upx-packed-fa${exe}    ${fo} -o upx-unpacked-fa${exe}
"${run_upx[@]}" -d upx-packed-fn${exe}    ${fo} -o upx-unpacked-fn${exe}
"${run_upx[@]}" -d upx-packed-fr${exe}    ${fo} -o upx-unpacked-fr${exe}
"${run_upx[@]}" -d upx-packed-nrv2b${exe} ${fo} -o upx-unpacked-nrv2b${exe}
"${run_upx[@]}" -d upx-packed-nrv2d${exe} ${fo} -o upx-unpacked-nrv2d${exe}
"${run_upx[@]}" -d upx-packed-nrv2e${exe} ${fo} -o upx-unpacked-nrv2e${exe}
"${run_upx[@]}" -d upx-packed-lzma${exe}  ${fo} -o upx-unpacked-lzma${exe}
if (( ${#packed_exact[@]} )); then
    "${run_upx[@]}" -d "${packed_exact[0]}" ${fo} -o upx-unpacked-exact${exe}
    cmp -s "${upx_self_exe}" upx-unpacked-exact${exe}
fi

# all unpacked files must be identical
cmp -s upx-unpacked${exe} upx-unpacked-fa${exe}
cmp -s upx-unpacked${exe} upx-unpacked-fn${exe}
cmp -s upx-unpacked${exe} upx-unpacked-fr${exe}
cmp -s upx-unpacked${exe} upx-unpacked-nrv2b${exe}
cmp -s upx-unpacked${exe} upx-unpacked-nrv2d${exe}
cmp -s upx-unpacked${exe} upx-unpacked-nrv2e${exe}
cmp -s upx-unpacked${exe} upx-unpacked-lzma${exe}

if [[ $UPX_CONFIG_DISABLE_RUN_UNPACKED_TEST != ON ]]; then
    "${emu[@]}" ./upx-unpacked${exe} --version-short
fi

if [[ $UPX_CONFIG_DISABLE_RUN_PACKED_TEST != ON ]]; then
    "${emu[@]}" ./upx-packed${exe}       --version-short
    "${emu[@]}" ./upx-packed-fa${exe}    --version-short
    "${emu[@]}" ./upx-packed-fn${exe}    --version-short
    "${emu[@]}" ./upx-packed-fr${exe}    --version-short
    "${emu[@]}" ./upx-packed-nrv2b${exe} --version-short
    "${emu[@]}" ./upx-packed-nrv2d${exe} --version-short
    "${emu[@]}" ./upx-packed-nrv2e${exe} --version-short
    "${emu[@]}" ./upx-packed-lzma${exe}  --version-short
    if (( ${#packed_exact[@]} )); then
        "${emu[@]}" ./"${packed_exact[0]}" --version-short
    fi
fi

if [[ $UPX_CONFIG_DISABLE_EXHAUSTIVE_TESTS != ON ]]; then
    set +x
    for method in nrv2b nrv2d nrv2e lzma; do
        for level in 1 2 3 4 5 6 7; do
            for small in normal small; do
                s="${method}-${level}"
                ss=
                if [[ $small == "small" ]]; then
                    s="${method}-${level}-${small}"
                    ss="--small"
                fi
                echo "========== $s =========="
                "${run_upx[@]}" -qq --${method} -${level} ${ss} --all-filters --debug-use-random-filter "${upx_self_exe}" ${fo} -o upx-packed-${s}${exe}
                "${run_upx[@]}" -qq -l upx-packed-${s}${exe}
                "${run_upx[@]}" -qq --fileinfo upx-packed-${s}${exe}
                "${run_upx[@]}" -qq -t upx-packed-${s}${exe}
                "${run_upx[@]}" -qq -d upx-packed-${s}${exe} ${fo} -o upx-unpacked-${s}${exe}
                cmp -s upx-unpacked${exe} upx-unpacked-${s}${exe}
                if [[ $UPX_CONFIG_DISABLE_RUN_PACKED_TEST != ON ]]; then
                    : ${upx_run_packed_test_count:=20}
                    for ((i = 0; i < $upx_run_packed_test_count; i++)); do
                        "${emu[@]}" ./upx-packed-${s}${exe} --version-short
                    done
                fi
            done
        done
    done
fi

echo "run_upx='${run_upx[*]}'"
echo "All done."
