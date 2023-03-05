OUTPUT_FORMAT("elf32-i386", "elf32-i386", "elf32-i386")
OUTPUT_ARCH(i386)
SECTIONS
{
  . = 0x00100000 + SIZEOF_HEADERS + 12;  /* 12==sizeof(l_info) */
}
