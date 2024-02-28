OUTPUT_FORMAT("elf32-bigarm", "elf32-bigarm", "elf32-littlearm")
OUTPUT_ARCH(arm)

SECTIONS
{
  . = 0x00100000 + SIZEOF_HEADERS + 12;  /* 12==sizeof(l_info) */
  .text : {
  }
  ELFMAINX : {
    *(ELFMAINX)
    *(.text)
  }
}
