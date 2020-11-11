#!/bin/bash
UPX_DIR="$( dirname ${BASH_SOURCE[0]} )/.."

clang++ -g -fsanitize=address,fuzzer $UPX_DIR/fuzz/unpacker_fuzzer.cpp -o $UPX_DIR/fuzz/unpacker_fuzzer -lz \
  -L$UPX_DIR/src -v $UPX_DIR/src/c_file.o $UPX_DIR/src/c_init.o $UPX_DIR/src/c_none.o $UPX_DIR/src/compress.o \
  $UPX_DIR/src/compress_lzma.o $UPX_DIR/src/compress_ucl.o $UPX_DIR/src/compress_zlib.o \
  $UPX_DIR/src/c_screen.o $UPX_DIR/src/except.o $UPX_DIR/src/file.o $UPX_DIR/src/filter.o $UPX_DIR/src/filteri.o \
  $UPX_DIR/src/help.o $UPX_DIR/src/lefile.o $UPX_DIR/src/linker.o $UPX_DIR/src/main.o $UPX_DIR/src/mem.o \
  $UPX_DIR/src/msg.o $UPX_DIR/src/packer_c.o $UPX_DIR/src/packer.o $UPX_DIR/src/packer_f.o \
  $UPX_DIR/src/packhead.o $UPX_DIR/src/packmast.o $UPX_DIR/src/p_armpe.o $UPX_DIR/src/p_com.o \
  $UPX_DIR/src/p_djgpp2.o $UPX_DIR/src/pefile.o $UPX_DIR/src/p_exe.o $UPX_DIR/src/p_lx_elf.o \
  $UPX_DIR/src/p_lx_exc.o $UPX_DIR/src/p_lx_interp.o $UPX_DIR/src/p_lx_sh.o $UPX_DIR/src/p_mach.o \
  $UPX_DIR/src/p_ps1.o $UPX_DIR/src/p_sys.o $UPX_DIR/src/p_tmt.o $UPX_DIR/src/p_tos.o $UPX_DIR/src/p_unix.o \
  $UPX_DIR/src/p_vmlinx.o $UPX_DIR/src/p_vmlinz.o $UPX_DIR/src/p_w32pe.o $UPX_DIR/src/p_w64pep.o \
  $UPX_DIR/src/p_wcle.o $UPX_DIR/src/s_djgpp2.o $UPX_DIR/src/snprintf.o $UPX_DIR/src/s_object.o \
  $UPX_DIR/src/stdcxx.o $UPX_DIR/src/s_vcsa.o $UPX_DIR/src/s_win32.o $UPX_DIR/src/ui.o $UPX_DIR/src/util.o \
  $UPX_DIR/src/work.o $UPX_UCLDIR/src/alloc.o $UPX_UCLDIR/src/n2b_99.o \
  $UPX_UCLDIR/src/n2b_d.o $UPX_UCLDIR/src/n2b_ds.o $UPX_UCLDIR/src/n2b_to.o \
  $UPX_UCLDIR/src/n2d_99.o $UPX_UCLDIR/src/n2d_d.o $UPX_UCLDIR/src/n2d_ds.o \
  $UPX_UCLDIR/src/n2d_to.o $UPX_UCLDIR/src/n2e_99.o $UPX_UCLDIR/src/n2e_d.o \
  $UPX_UCLDIR/src/n2e_ds.o $UPX_UCLDIR/src/n2e_to.o $UPX_UCLDIR/src/ucl_crc.o \
  $UPX_UCLDIR/src/ucl_init.o $UPX_UCLDIR/src/ucl_ptr.o $UPX_UCLDIR/src/ucl_str.o \
  $UPX_UCLDIR/src/ucl_util.o

