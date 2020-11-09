# Fuzzing

NOTE: Work in progress

## Overview

The focus of this fuzzing work is to uncover issues in the decompression mode of
UPX. This mode is of greater interest as this is the time when someone is more
likely to be dealing with untrusted input.

The fuzzing will make use of libFuzzer. The goal is to integrate this with
[OSS-Fuzz](https://github.com/google/oss-fuzz).

## Building

1.  Build ucl

```
cd /tmp/
wget http://www.oberhumer.com/opensource/ucl/download/ucl-1.03.tar.gz
tar -zxf ucl-1.03.tar.gz
cd ucl-1.03/
./configure CC="gcc -std=gnu89"
make all
```

1.  Build UPX

```
export UPX_UCLDIR=/tmp/ucl-1.03
export BUILD_TYPE_SANITIZE=1
make all
```

1.  Build fuzzer

NOTE: requires changes to src/main.cpp to remove the main() function and allow
the fuzzer to link as a library. See this branch for the changes.

```
cd fuzz && clang++ -g -fsanitize=address,fuzzer unpacker_fuzzer.cpp -lz \
  -L../src -v ../src/c_file.o ../src/c_init.o ../src/c_none.o ../src/compress.o \
  ../src/compress_lzma.o ../src/compress_ucl.o ../src/compress_zlib.o \
  ../src/c_screen.o ../src/except.o ../src/file.o ../src/filter.o ../src/filteri.o \
  ../src/help.o ../src/lefile.o ../src/linker.o ../src/main.o ../src/mem.o \
  ../src/msg.o ../src/packer_c.o ../src/packer.o ../src/packer_f.o \
  ../src/packhead.o ../src/packmast.o ../src/p_armpe.o ../src/p_com.o \
  ../src/p_djgpp2.o ../src/pefile.o ../src/p_exe.o ../src/p_lx_elf.o \
  ../src/p_lx_exc.o ../src/p_lx_interp.o ../src/p_lx_sh.o ../src/p_mach.o \
  ../src/p_ps1.o ../src/p_sys.o ../src/p_tmt.o ../src/p_tos.o ../src/p_unix.o \
  ../src/p_vmlinx.o ../src/p_vmlinz.o ../src/p_w32pe.o ../src/p_w64pep.o \
  ../src/p_wcle.o ../src/s_djgpp2.o ../src/snprintf.o ../src/s_object.o \
  ../src/stdcxx.o ../src/s_vcsa.o ../src/s_win32.o ../src/ui.o ../src/util.o \
  ../src/work.o $UPX_UCLDIR/src/alloc.o $UPX_UCLDIR/src/n2b_99.o \
  $UPX_UCLDIR/src/n2b_d.o $UPX_UCLDIR/src/n2b_ds.o $UPX_UCLDIR/src/n2b_to.o \
  $UPX_UCLDIR/src/n2d_99.o $UPX_UCLDIR/src/n2d_d.o $UPX_UCLDIR/src/n2d_ds.o \
  $UPX_UCLDIR/src/n2d_to.o $UPX_UCLDIR/src/n2e_99.o $UPX_UCLDIR/src/n2e_d.o \
  $UPX_UCLDIR/src/n2e_ds.o $UPX_UCLDIR/src/n2e_to.o $UPX_UCLDIR/src/ucl_crc.o \
  $UPX_UCLDIR/src/ucl_init.o $UPX_UCLDIR/src/ucl_ptr.o $UPX_UCLDIR/src/ucl_str.o \
  $UPX_UCLDIR/src/ucl_util.o
```

1. Get a corpus of samples.

```./fuzzer/get_corpus.sh```

1. Fuzz

```fuzz/a.out /tmp/upx_fuzzing_corpus```

