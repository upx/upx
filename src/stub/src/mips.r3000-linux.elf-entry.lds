OUTPUT_FORMAT("elf32-bigmips", "elf32-bigmips", "elf32-littlemips")
OUTPUT_ARCH(mips)

SECTIONS
{
  .text : {
  }
  ELFMAINX : {
    *(ELFMAINX)
    *(.text)
  }
}
