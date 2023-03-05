OUTPUT_FORMAT("elf64-littleaarch64", "elf64-littleaarch64", "elf64-littleaarch64")
OUTPUT_ARCH(aarch64)

SECTIONS
{
  . = 0x00100000 + SIZEOF_HEADERS + 12;  /* 12==sizeof(l_info) */
}
