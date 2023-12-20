OUTPUT_FORMAT("elf64-littleaarch64", "elf64-littleaarch64", "elf64-littleaarch64")
OUTPUT_ARCH(aarch64)

SECTIONS
{
  . = SIZEOF_HEADERS + 12;  /* 12==sizeof(l_info) */
}
