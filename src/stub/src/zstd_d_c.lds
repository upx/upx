/* zstd_d_c.lds -- linker script for zstd decompressor stub
 * Places .text, .data, .rodata contiguously so PC-relative references work.
 * Uses a high base address to avoid x86_64 addr32 call optimizations. */
SECTIONS {
    . = 0x100000000;
    .text : { *(.text .text.*) }
    .data : { *(.data .data.*) *(.got) *(.got.plt) }
    .rodata : { *(.rodata .rodata.*) }
    /DISCARD/ : { *(.dynamic) *(.dynsym) *(.dynstr) *(.hash) *(.gnu.hash)
                  *(.plt) *(.comment) *(.note*) *(.riscv.attributes) }
}
