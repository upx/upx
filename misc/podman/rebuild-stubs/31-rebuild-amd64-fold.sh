#! /usr/bin/env bash
set -ex
export PATH=/home/upx/.local/bin/bin-upx:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin
cd /home/upx/src/upx/src/stub
mkdir -p tmp
top_srcdir=../..
T=amd64-linux.elf-fold

amd64-linux-gcc-3.4.4 -fPIC -m64 -nostdinc -fno-exceptions -fno-asynchronous-unwind-tables -Werror -c src/$T.S -o tmp/$T.o
multiarch-objcopy-2.17 -F elf64-x86-64 -R .comment -R .note -R .note.GNU-stack -R .reginfo tmp/$T.o

amd64-linux-gcc-3.4.4 -fPIC -m64 -nostdinc -fno-exceptions -fno-asynchronous-unwind-tables -Werror -c src/amd64-expand.S -o tmp/amd64-expand.o

amd64-linux-gcc-3.4.4 -fPIC -m64 -nostdinc -fno-exceptions -fno-asynchronous-unwind-tables -Werror -c -O src/upxfd_linux.c -o tmp/amd64-linux.elf-upxfd_linux.o
multiarch-objcopy-2.17 -F elf64-x86-64 --rename-section .text=UMF_LINUX -R .comment -R .data -R .bss -R .note.GNU-stack tmp/amd64-linux.elf-upxfd_linux.o

amd64-linux-gcc-3.4.4 -fPIC -m64 -nostdinc -fno-exceptions -fno-asynchronous-unwind-tables -Werror -c -Os src/amd64-linux.elf-main2.c -o tmp/amd64-linux.elf-main2.o
multiarch-objcopy-2.17 -F elf64-x86-64 -R .comment -R .note -R .note.GNU-stack -R .reginfo tmp/amd64-linux.elf-main2.o

# Use /usr/bin/gcc directly (bypass the stubtools gcc wrapper which just exits 1)
env PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin \
/usr/bin/gcc -m64 -O3 -fpic -fvisibility=hidden \
    -fno-exceptions -fno-asynchronous-unwind-tables -fno-stack-protector \
    -fcf-protection=none \
    -fno-builtin-memmove -fno-builtin-memcpy \
    -fno-builtin-memset -fno-builtin-memcmp -fno-tree-loop-distribute-patterns \
    -I$top_srcdir/vendor/zstd/lib -DZSTD_DISABLE_ASM=1 -DDYNAMIC_BMI2=0 \
    -DZSTD_NO_INLINE=1 -DHUF_FORCE_DECOMPRESS_X2=1 -DZSTD_TRACE=0 \
    -c src/c/zstd_d_c.c -o tmp/zstd_d_c_raw.o 2>/dev/null
/usr/bin/ld -shared -Bsymbolic -T src/zstd_d_c.lds -o tmp/zstd_d_c.so tmp/zstd_d_c_raw.o 2>/dev/null
/usr/bin/objcopy -j .text -j .data -j .rodata -j .rodata.cst16 -j .rodata.cst8 -j .rodata.cst32 \
    --output-target=elf64-x86-64 tmp/zstd_d_c.so tmp/zstd_d_c.o 2>/dev/null
# Patch ET_DYN -> ET_REL
python3 -c "
import struct
d = bytearray(open('tmp/zstd_d_c.o','rb').read())
struct.pack_into('<H', d, 16, 1)
open('tmp/zstd_d_c.o','wb').write(d)
"
/usr/bin/objcopy --rename-section .text=ZSTD_DEC --rename-section .data=ZSTD_DEC \
    --rename-section .rodata=ZSTD_DEC \
    -R .comment -R .note -R .note.GNU-stack tmp/zstd_d_c.o

echo "=== addr32 calls in zstd_d_c.o (should be 0) ==="
/usr/bin/objdump -d tmp/zstd_d_c.o | grep -c "addr32" || echo "0"

/usr/bin/ld -r -T src/$T.lds -Map tmp/$T.map \
    tmp/$T.o tmp/amd64-expand.o tmp/amd64-linux.elf-upxfd_linux.o \
    tmp/amd64-linux.elf-main2.o tmp/zstd_d_c.o -o tmp/$T.bin
/usr/bin/objcopy --rename-section ZSTD_DEC.1=ZSTD_DEC tmp/$T.bin 2>/dev/null || true

multiarch-objcopy-2.17 -F elf64-x86-64 \
    -R .data -R .bss -R .comment -R .note -R .note.GNU-stack -R .reginfo tmp/$T.bin
multiarch-objcopy-2.17 -F elf64-x86-64 \
    --strip-unneeded --keep-symbol=_start --keep-symbol=upx_main2 \
    --keep-symbol=upx_so_main --keep-symbol=str_upx tmp/$T.bin

# Embed section/symbol/reloc info for UPX linker (skip slow -Dr disasm)
multiarch-objdump-2.17 -b elf64-x86-64 -htr -w tmp/$T.bin | \
    sed -e '1s/^.*: *file format/file format/' \
        -e 's/	/ /g' \
        -e 's/ 00*/ 0/g' \
        -e 's/CONTENTS.*/CONTENTS/' \
    >> tmp/$T.bin

python3 scripts/bin2h.py --ident=auto-stub --mode=c tmp/$T.bin $T.h
echo "=== Done: $(wc -c < $T.h) bytes in $T.h ==="
