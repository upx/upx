OUTPUT_FORMAT("elf32-i386", "elf32-i386", "elf32-i386")
OUTPUT_ARCH(i386)
ENTRY(_start)
SECTIONS
{
  . = 0x00400000 + SIZEOF_HEADERS;
  .text : {
    *(.text)
    *(.data)
  }
  /*  0x08048000: customary Linux/x86 Elf .text start */
  . = 0x08048000 + (0xfff & .);
  .data : {
  }
}
