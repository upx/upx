// This file exists so that the LZMA_DEC10 and LZMA_DEC20 sections
// can contain endian-dependent code in the "64" and "64le" directories,
// but otherwise use lzma_d-common.S

#define BIG_ENDIAN 1

#include "../64le/lzma_d-common.S"

  section LZMA_DEC10
        bl   lzma_dec10
        b    eof_dec10
lzma_dec10:
#include "lzma_d_cs.S"
eof_dec10:

  section LZMA_DEC20
        bl   lzma_dec20
        b    eof_dec20
lzma_dec20:
#include "lzma_d_cf.S"
eof_dec20:

// vi:ts=8:et
