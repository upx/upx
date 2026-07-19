#
# UPX "CMake" build file; see https://cmake.org/
# Copyright (C) Markus Franz Xaver Johannes Oberhumer
#

#***********************************************************************
# test section: self-pack tests
#
# IMPORTANT NOTE: these tests can only work if the host executable format
#   is supported by UPX!
#***********************************************************************

function(upx_self_pack_test)

set(emu "")
if(DEFINED CMAKE_CROSSCOMPILING_EMULATOR)
    set(emu "${CMAKE_CROSSCOMPILING_EMULATOR}")
endif()
set(exe "${CMAKE_EXECUTABLE_SUFFIX}")
set(upx_self_exe "$<TARGET_FILE:upx>")
set(fo "--force-overwrite")

#
# basic tests
#

upx_add_test(upx-self-pack          upx -3               "${upx_self_exe}" ${fo} -o upx-packed${exe})
upx_add_test(upx-self-pack-fa       upx -3 --all-filters "${upx_self_exe}" ${fo} -o upx-packed-fa${exe})
upx_add_test(upx-self-pack-fn       upx -3 --no-filter   "${upx_self_exe}" ${fo} -o upx-packed-fn${exe})
upx_add_test(upx-self-pack-fr       upx -3 --all-filters --debug-use-random-filter "${upx_self_exe}" ${fo} -o upx-packed-fr${exe})
upx_add_test(upx-self-pack-nrv2b    upx -3 --nrv2b       "${upx_self_exe}" ${fo} -o upx-packed-nrv2b${exe})
upx_add_test(upx-self-pack-nrv2d    upx -3 --nrv2d       "${upx_self_exe}" ${fo} -o upx-packed-nrv2d${exe})
upx_add_test(upx-self-pack-nrv2e    upx -3 --nrv2e       "${upx_self_exe}" ${fo} -o upx-packed-nrv2e${exe})
upx_add_test(upx-self-pack-lzma     upx -1 --lzma        "${upx_self_exe}" ${fo} -o upx-packed-lzma${exe})

upx_add_test(upx-list               upx -l         upx-packed${exe} upx-packed-fa${exe} upx-packed-fn${exe} upx-packed-fr${exe} upx-packed-nrv2b${exe} upx-packed-nrv2d${exe} upx-packed-nrv2e${exe} upx-packed-lzma${exe})
upx_add_test(upx-fileinfo           upx --fileinfo upx-packed${exe} upx-packed-fa${exe} upx-packed-fn${exe} upx-packed-fr${exe} upx-packed-nrv2b${exe} upx-packed-nrv2d${exe} upx-packed-nrv2e${exe} upx-packed-lzma${exe})
upx_add_test(upx-test               upx -t         upx-packed${exe} upx-packed-fa${exe} upx-packed-fn${exe} upx-packed-fr${exe} upx-packed-nrv2b${exe} upx-packed-nrv2d${exe} upx-packed-nrv2e${exe} upx-packed-lzma${exe})

upx_add_test(upx-unpack             upx -d upx-packed${exe}       ${fo} -o upx-unpacked${exe})
upx_add_test(upx-unpack-fa          upx -d upx-packed-fa${exe}    ${fo} -o upx-unpacked-fa${exe})
upx_add_test(upx-unpack-fn          upx -d upx-packed-fn${exe}    ${fo} -o upx-unpacked-fn${exe})
upx_add_test(upx-unpack-fr          upx -d upx-packed-fr${exe}    ${fo} -o upx-unpacked-fr${exe})
upx_add_test(upx-unpack-nrv2b       upx -d upx-packed-nrv2b${exe} ${fo} -o upx-unpacked-nrv2b${exe})
upx_add_test(upx-unpack-nrv2d       upx -d upx-packed-nrv2d${exe} ${fo} -o upx-unpacked-nrv2d${exe})
upx_add_test(upx-unpack-nrv2e       upx -d upx-packed-nrv2e${exe} ${fo} -o upx-unpacked-nrv2e${exe})
upx_add_test(upx-unpack-lzma        upx -d upx-packed-lzma${exe}  ${fo} -o upx-unpacked-lzma${exe})

# all unpacked files must be identical
upx_add_test(upx-compare-fa         "${CMAKE_COMMAND}" -E compare_files upx-unpacked${exe} upx-unpacked-fa${exe})
upx_add_test(upx-compare-fn         "${CMAKE_COMMAND}" -E compare_files upx-unpacked${exe} upx-unpacked-fn${exe})
upx_add_test(upx-compare-fr         "${CMAKE_COMMAND}" -E compare_files upx-unpacked${exe} upx-unpacked-fr${exe})
upx_add_test(upx-compare-nrv2b      "${CMAKE_COMMAND}" -E compare_files upx-unpacked${exe} upx-unpacked-nrv2b${exe})
upx_add_test(upx-compare-nrv2d      "${CMAKE_COMMAND}" -E compare_files upx-unpacked${exe} upx-unpacked-nrv2d${exe})
upx_add_test(upx-compare-nrv2e      "${CMAKE_COMMAND}" -E compare_files upx-unpacked${exe} upx-unpacked-nrv2e${exe})
upx_add_test(upx-compare-lzma       "${CMAKE_COMMAND}" -E compare_files upx-unpacked${exe} upx-unpacked-lzma${exe})

# test dependencies
upx_test_depends(upx-list           "upx-self-pack;upx-self-pack-fa;upx-self-pack-fn;upx-self-pack-fr;upx-self-pack-nrv2b;upx-self-pack-nrv2d;upx-self-pack-nrv2e;upx-self-pack-lzma")
upx_test_depends(upx-fileinfo       "upx-self-pack;upx-self-pack-fa;upx-self-pack-fn;upx-self-pack-fr;upx-self-pack-nrv2b;upx-self-pack-nrv2d;upx-self-pack-nrv2e;upx-self-pack-lzma")
upx_test_depends(upx-test           "upx-self-pack;upx-self-pack-fa;upx-self-pack-fn;upx-self-pack-fr;upx-self-pack-nrv2b;upx-self-pack-nrv2d;upx-self-pack-nrv2e;upx-self-pack-lzma")
upx_test_depends(upx-unpack         upx-self-pack)
upx_test_depends(upx-unpack-fa      upx-self-pack-fa)
upx_test_depends(upx-unpack-fn      upx-self-pack-fn)
upx_test_depends(upx-unpack-fr      upx-self-pack-fr)
upx_test_depends(upx-unpack-nrv2b   upx-self-pack-nrv2b)
upx_test_depends(upx-unpack-nrv2d   upx-self-pack-nrv2d)
upx_test_depends(upx-unpack-nrv2e   upx-self-pack-nrv2e)
upx_test_depends(upx-unpack-lzma    upx-self-pack-lzma)
upx_test_depends(upx-compare-fa     "upx-unpack;upx-unpack-fa")
upx_test_depends(upx-compare-fn     "upx-unpack;upx-unpack-fn")
upx_test_depends(upx-compare-fr     "upx-unpack;upx-unpack-fr")
upx_test_depends(upx-compare-nrv2b  "upx-unpack;upx-unpack-nrv2b")
upx_test_depends(upx-compare-nrv2d  "upx-unpack;upx-unpack-nrv2d")
upx_test_depends(upx-compare-nrv2e  "upx-unpack;upx-unpack-nrv2e")
upx_test_depends(upx-compare-lzma   "upx-unpack;upx-unpack-lzma")
# tests with higher COST values will run first
set_tests_properties(upx-self-pack       PROPERTIES COST 90)
set_tests_properties(upx-self-pack-fa    PROPERTIES COST 20)
set_tests_properties(upx-self-pack-fn    PROPERTIES COST 20)
set_tests_properties(upx-self-pack-fr    PROPERTIES COST 20)
set_tests_properties(upx-self-pack-nrv2b PROPERTIES COST 20)
set_tests_properties(upx-self-pack-nrv2d PROPERTIES COST 20)
set_tests_properties(upx-self-pack-nrv2e PROPERTIES COST 20)
set_tests_properties(upx-self-pack-lzma  PROPERTIES COST 30)
set_tests_properties(upx-unpack          PROPERTIES COST 10)

if(NOT UPX_CONFIG_DISABLE_RUN_UNPACKED_TEST)
    upx_add_test(upx-run-unpacked           ${emu} ./upx-unpacked${exe} --version-short)
    upx_test_depends(upx-run-unpacked       upx-unpack)
endif()

if(NOT UPX_CONFIG_DISABLE_RUN_PACKED_TEST)
    upx_add_test(upx-run-packed             ${emu} ./upx-packed${exe}       --version-short)
    upx_add_test(upx-run-packed-fa          ${emu} ./upx-packed-fa${exe}    --version-short)
    upx_add_test(upx-run-packed-fn          ${emu} ./upx-packed-fn${exe}    --version-short)
    upx_add_test(upx-run-packed-fr          ${emu} ./upx-packed-fr${exe}    --version-short)
    upx_add_test(upx-run-packed-nrv2b       ${emu} ./upx-packed-nrv2b${exe} --version-short)
    upx_add_test(upx-run-packed-nrv2d       ${emu} ./upx-packed-nrv2d${exe} --version-short)
    upx_add_test(upx-run-packed-nrv2e       ${emu} ./upx-packed-nrv2e${exe} --version-short)
    upx_add_test(upx-run-packed-lzma        ${emu} ./upx-packed-lzma${exe}  --version-short)
    upx_test_depends(upx-run-packed         upx-self-pack)
    upx_test_depends(upx-run-packed-fa      upx-self-pack-fa)
    upx_test_depends(upx-run-packed-fn      upx-self-pack-fn)
    upx_test_depends(upx-run-packed-fr      upx-self-pack-fr)
    upx_test_depends(upx-run-packed-nrv2b   upx-self-pack-nrv2b)
    upx_test_depends(upx-run-packed-nrv2d   upx-self-pack-nrv2d)
    upx_test_depends(upx-run-packed-nrv2e   upx-self-pack-nrv2e)
    upx_test_depends(upx-run-packed-lzma    upx-self-pack-lzma)
endif()

#
# exhaustive tests
#

if(NOT UPX_CONFIG_DISABLE_EXHAUSTIVE_TESTS)
    foreach(method IN ITEMS nrv2b nrv2d nrv2e lzma)
        foreach(level IN ITEMS 1 2 3 4 5 6 7)
            foreach(small IN ITEMS normal small)
                set(s "${method}-${level}")
                set(ss "")
                if(small MATCHES "small")
                    set(s "${method}-${level}-${small}")
                    set(ss "--small")
                endif()
                upx_add_test(upx-self-pack-${s}         upx --${method} -${level} ${ss} --all-filters --debug-use-random-filter -i "${upx_self_exe}" ${fo} -o upx-packed-${s}${exe})
                upx_add_test(upx-list-${s}              upx -l upx-packed-${s}${exe})
                upx_add_test(upx-fileinfo-${s}          upx --fileinfo upx-packed-${s}${exe})
                upx_add_test(upx-test-${s}              upx -t upx-packed-${s}${exe})
                upx_add_test(upx-unpack-${s}            upx -d upx-packed-${s}${exe} -i ${fo} -o upx-unpacked-${s}${exe})
                upx_add_test(upx-compare-${s}           "${CMAKE_COMMAND}" -E compare_files upx-unpacked${exe} upx-unpacked-${s}${exe})
                upx_test_depends(upx-list-${s}          upx-self-pack-${s})
                upx_test_depends(upx-fileinfo-${s}      upx-self-pack-${s})
                upx_test_depends(upx-test-${s}          upx-self-pack-${s})
                upx_test_depends(upx-unpack-${s}        upx-self-pack-${s})
                upx_test_depends(upx-compare-${s}       "upx-unpack;upx-unpack-${s}")
                if(method MATCHES "lzma")
                    set_tests_properties(upx-self-pack-${s} PROPERTIES COST "3${level}")
                else()
                    set_tests_properties(upx-self-pack-${s} PROPERTIES COST "2${level}")
                endif()
                if(NOT UPX_CONFIG_DISABLE_RUN_PACKED_TEST)
                    set(i 0)
                    while(${i} LESS 20)
                        math(EXPR i "${i} + 1")
                        upx_add_test(upx-run-packed-${s}-${i}     ${emu} ./upx-packed-${s}${exe} --version-short)
                        upx_test_depends(upx-run-packed-${s}-${i} upx-self-pack-${s})
                    endwhile()
                endif()
            endforeach()
        endforeach()
    endforeach()
endif() # UPX_CONFIG_DISABLE_EXHAUSTIVE_TESTS

endfunction()

# vim:set ft=cmake ts=4 sw=4 tw=0 et:
