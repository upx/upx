OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
OUTPUT_ARCH(arm)

SECTIONS
{
  SO_HEAD : {
    *(SO_HEAD)
    *(.text)
  }
}
