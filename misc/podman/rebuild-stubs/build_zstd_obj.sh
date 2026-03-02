#! /usr/bin/env bash
## Build zstd decompressor object for a given architecture.
## Usage: build_zstd_obj.sh <arch> <cc> <ld> <objcopy> <bfdname>
## Example: build_zstd_obj.sh amd64 "/usr/bin/gcc -m64" /usr/bin/ld /usr/bin/objcopy elf64-x86-64
set -e
ARCH="$1"; CC="$2"; LD="$3"; OBJCOPY="$4"; BFDNAME="$5"
cd /home/upx/src/upx/src/stub
mkdir -p tmp
top_srcdir=../..

ZSTD_CFLAGS="-O3 -fpic -fvisibility=hidden -fno-exceptions -fno-asynchronous-unwind-tables"
ZSTD_CFLAGS="$ZSTD_CFLAGS -fno-stack-protector -fno-builtin-memmove -fno-builtin-memcpy"
ZSTD_CFLAGS="$ZSTD_CFLAGS -fno-builtin-memset -fno-builtin-memcmp -fno-tree-loop-distribute-patterns"
ZSTD_CFLAGS="$ZSTD_CFLAGS -I$top_srcdir/vendor/zstd/lib -DZSTD_DISABLE_ASM=1 -DDYNAMIC_BMI2=0"
ZSTD_CFLAGS="$ZSTD_CFLAGS -DZSTD_NO_INLINE=1 -DHUF_FORCE_DECOMPRESS_X2=1 -DZSTD_TRACE=0"

# Disable CET on x86
case "$ARCH" in amd64|i386) ZSTD_CFLAGS="$ZSTD_CFLAGS -fcf-protection=none";; esac
# riscv64 -Os generates memcpy/memset calls; use -O2 instead

echo "=== Building zstd_d_c for $ARCH ==="
env PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin \
    $CC $ZSTD_CFLAGS -c src/c/zstd_d_c.c -o tmp/zstd_d_c_${ARCH}_raw.o 2>/dev/null

$LD -shared -Bsymbolic -T src/zstd_d_c.lds -o tmp/zstd_d_c_${ARCH}.so tmp/zstd_d_c_${ARCH}_raw.o 2>/dev/null

$OBJCOPY -j .text -j .data -j .rodata -j .rodata.cst16 -j .rodata.cst8 -j .rodata.cst32 \
    --output-target=$BFDNAME tmp/zstd_d_c_${ARCH}.so tmp/zstd_d_c_${ARCH}.o 2>/dev/null

# Patch ET_DYN -> ET_REL
python3 -c "
import struct
d = bytearray(open('tmp/zstd_d_c_${ARCH}.o','rb').read())
struct.pack_into('<H', d, 16, 1)
open('tmp/zstd_d_c_${ARCH}.o','wb').write(d)
"

$OBJCOPY --rename-section .text=ZSTD_DEC --rename-section .data=ZSTD_DEC \
    --rename-section .rodata=ZSTD_DEC \
    -R .comment -R .note -R .note.GNU-stack tmp/zstd_d_c_${ARCH}.o

# arm64 uses ADRP which requires page-aligned .text section
case "$ARCH" in arm64)
    python3 -c "
import struct
d = bytearray(open('tmp/zstd_d_c_${ARCH}.o','rb').read())
e_shoff = struct.unpack_from('<Q', d, 40)[0]
e_shnum = struct.unpack_from('<H', d, 60)[0]
first = True
for i in range(e_shnum):
    off = e_shoff + i * 64
    sh_type = struct.unpack_from('<I', d, off + 4)[0]
    sh_flags = struct.unpack_from('<Q', d, off + 8)[0]
    if sh_type == 1 and (sh_flags & 4):  # SHT_PROGBITS + SHF_EXECINSTR (code)
        struct.pack_into('<Q', d, off + 48, 4096)  # page-align code section only
open('tmp/zstd_d_c_${ARCH}.o','wb').write(d)
"
;; esac

echo "  OK: $(wc -c < tmp/zstd_d_c_${ARCH}.o) bytes"
