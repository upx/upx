/* Copyright (C) 2014-2020 Free Software Foundation, Inc.
   Copying and distribution of this script, with or without modification,
   are permitted in any medium without royalty provided the copyright
   notice and this notice are preserved.  */
/* 2022-05-20  Stripped down greatly from output of:
      ld --verbose -m elf_x86_64
   using binutils-2.35.2-6.fc34.x86_64 .
   So that
      multiarch-ld-2.17 -T this.lds -r ...
   will preserve Sections (Elf64_Shdr) and relocations between them.
*/
OUTPUT_FORMAT("elf64-x86-64", "elf64-x86-64", "elf64-x86-64")
OUTPUT_ARCH(i386:x86-64)
/*
multiarch-ld-2.17 --strip-all \
    -T ./src/amd64-linux.elf-so_fold.lds \
    -Map tmp/amd64-linux.elf-so_fold.map \
       tmp/amd64-linux.elf-so_fold.o \
       tmp/amd64-linux.elf-so_main.o \
       tmp/amd64-linux.elf-entry.o \
    -o tmp/amd64-linux.elf-so_fold.bin
*/

SECTIONS
{
  . = 0x00100000 + SIZEOF_HEADERS + 12;  /* 12==sizeof(l_info) */
/*SO_HEAD : { *(SO_HEAD) }*/
/*
  .text           :
  {
    *(.text.unlikely .text.*_unlikely .text.unlikely.*)
    *(.text.exit .text.exit.*)
    *(.text.startup .text.startup.*)
    *(.text.hot .text.hot.*)
    *(.text .stub .text.* .gnu.linkonce.t.*)
    *(NRV_HEAD)
    *(NRV2B)
    *(NRV2E)
    *(NRV2D)
    *(NRV_TAIL)
    *(LZMA*)
    *(SO_MAIN)
    *(SO_TAIL)
  }
  .data           :
  {
    *(.data .data.* .gnu.linkonce.d.*)
  }
  .data1          : { *(.data1) }
  .bss            :
  {
   *(.bss .bss.* .gnu.linkonce.b.*)
   *(COMMON)
  }
*/
}
