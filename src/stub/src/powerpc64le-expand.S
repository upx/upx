#if !defined(BIG_ENDIAN) || BIG_ENDIAN
#error must "#define BIG_ENDIAN 0" for powerpc64le
#endif

#include "../src/powerpc64-expand.S"
