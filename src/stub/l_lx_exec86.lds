OUTPUT_FORMAT("elf32-i386", "elf32-i386", "elf32-i386")
OUTPUT_ARCH(i386)
ENTRY(_start)
SECTIONS
{
  /*  0x08048000: customary Linux/x86 Elf .text start */
  . = 0x08048000 + SIZEOF_HEADERS;
  . = ALIGN(0x80);  /* room for Ehdr, 2*Phdr, l_info */
  .data : { /* put everything together in one Phdr */
    *(.text)
    *(.rodata)
    *(.data)
    *(.bss)
    *(COMMON)
  }
  /* save other Phdr for the overlay */
}
