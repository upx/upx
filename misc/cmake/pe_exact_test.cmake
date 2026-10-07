#
# UPX "CMake" build file; see https://upx.github.io/
# Copyright (C) Markus Franz Xaver Johannes Oberhumer
#

function(upx_pe_exact_roundtrip name input extension runner)

set(packed "${CMAKE_CURRENT_BINARY_DIR}/XTesting/$<CONFIG>/upx-test-pe-${name}-packed.${extension}")
set(unpacked "${CMAKE_CURRENT_BINARY_DIR}/XTesting/$<CONFIG>/upx-test-pe-${name}-unpacked.${extension}")
upx_add_test(upx-pe-exact-pack-${name} upx --exact "${input}" ${fo} -o "${packed}")
if(NOT UPX_CONFIG_DISABLE_RUN_UNPACKED_TEST)
    upx_add_test(upx-pe-${name}-run-original ${runner} "${input}")
endif()
if(NOT UPX_CONFIG_DISABLE_RUN_PACKED_TEST)
    upx_add_test(upx-pe-exact-run-${name} ${runner} "${packed}")
    upx_test_depends(upx-pe-exact-run-${name} upx-pe-exact-pack-${name})
endif()
upx_add_test(upx-pe-exact-unpack-${name} upx -d "${packed}" ${fo} -o "${unpacked}")
upx_add_test(upx-pe-exact-compare-${name} "${CMAKE_COMMAND}" -E compare_files "${input}" "${unpacked}")
upx_add_test(upx-pe-exact-list-${name} upx -l "${packed}")
upx_add_test(upx-pe-exact-fileinfo-${name} upx --fileinfo "${packed}")
upx_add_test(upx-pe-exact-test-${name} upx -t "${packed}")
foreach(action IN ITEMS unpack list fileinfo test)
    upx_test_depends(upx-pe-exact-${action}-${name} upx-pe-exact-pack-${name})
endforeach()
upx_test_depends(upx-pe-exact-compare-${name} upx-pe-exact-unpack-${name})
if(TEST upx-pe-${name}-prepare)
    upx_test_depends(upx-pe-exact-pack-${name} upx-pe-${name}-prepare)
    if(TEST upx-pe-${name}-run-original)
        upx_test_depends(upx-pe-${name}-run-original upx-pe-${name}-prepare)
    endif()
endif()

endfunction()

function(upx_pe_exact_test)

set(exact_dll "$<TARGET_FILE:upx-test-pe-exact-dll>")
set(fo "--force-overwrite")
set(wd "${CMAKE_CURRENT_BINARY_DIR}/XTesting/$<CONFIG>")
upx_pe_exact_roundtrip(dll "${exact_dll}" dll upx-test-pe-exact-dll-loader)

set(input "${wd}/upx-test-pe-gap.dll")
upx_add_test(upx-pe-gap-prepare upx-test-pe-exact-fixture "${exact_dll}" "${input}")
upx_pe_exact_roundtrip(gap "${input}" dll upx-test-pe-exact-dll-loader)

upx_add_test(upx-pe-normal-pack-gap upx --force "${wd}/upx-test-pe-gap.dll" ${fo}
             -o upx-test-pe-gap-normal-packed.dll)
upx_add_test(upx-pe-normal-unpack-gap upx -d upx-test-pe-gap-normal-packed.dll ${fo}
             -o upx-test-pe-gap-normal-unpacked.dll)
upx_test_depends(upx-pe-normal-pack-gap upx-pe-gap-prepare)
upx_test_depends(upx-pe-normal-unpack-gap upx-pe-normal-pack-gap)

upx_add_test(upx-pe-normal-pack-stripped upx --force --strip-relocs=1 "$<TARGET_FILE:upx>" ${fo}
             -o upx-test-pe-stripped-packed.exe)
upx_add_test(upx-pe-normal-unpack-stripped upx -d upx-test-pe-stripped-packed.exe ${fo}
             -o upx-test-pe-stripped-unpacked.exe)
upx_test_depends(upx-pe-normal-unpack-stripped upx-pe-normal-pack-stripped)

upx_add_test(upx-pe-exact-reject-strip-relocs upx --exact --strip-relocs=1 "${exact_dll}" ${fo}
             -o "${wd}/upx-test-pe-strip-relocs-rejected.dll")
set_tests_properties(upx-pe-exact-reject-strip-relocs PROPERTIES
                     PASS_REGULAR_EXPRESSION "cannot use both '--exact' and '--strip-relocs'")

endfunction()

# vim:set ft=cmake ts=4 sw=4 tw=0 et:
