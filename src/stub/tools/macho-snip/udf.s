    .align 4
// start: .globl start  /* for standalone */
_main: .globl _main  /* for -lc */
#ifdef __x86_64__
    ud2
#endif
#ifdef __AARCH64EL__
    .int 0x7b  /* udf 123 */
#endif
