/* auipc.h -- auipc filter for RiscV

   This file is part of the UPX executable compressor.

   Copyright (C) John F. Reiser
   All Rights Reserved.

   UPX and the UCL library are free software; you can redistribute them
   and/or modify them under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.
   If not, write to the Free Software Foundation, Inc.,
   59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

   John F. Reiser
   <jreiser@users.sourceforge.net>
 */

/* clang-format off */

// Do not change columns or whitespace!
// I write code with adjacent lines that should match spacing.
// filter/auipc.h has been added to CLANG_FORMAT_EXCLUDE_FILES in Makefile.

// Use 'int' instead of 'unsigned' because gcc for RiscV does not realize
// that everything fits in 32 bits, so often uses "slli reg,32; srli reg,32"
// to force 32-bit unsigned; but this is not needed.

/*************************************************************************
// filter / scan
**************************************************************************/
#if 0  //{ wide ASCII-art documentation
// The original layout is:
 31                                              12 11          7 6          0
+--------------------------------------------------+-------------+------------+
|                      imm[31:12]                  |   r_aui     |   AUIPC    |
+-------------+-------------+-------------+--------+-------------+------------+
|        imm[11:0]          |    r_aui    |  func3 |     rd      |   opcode   |
+-------------+-------------+-------------+--------+-------------+------------+
 31         25 24         20 19         15 14    12 11          7 6          0

// The filtered layout is:
 31                24 23                16 15    12 11       8  7 6          0
+--------------------+--------------------+-------------------+--+------------+
|    addr_mid_lo     |    addr_mid_hi     |     addr_MSB      |a0|   AUIPC    |
+-----------+--------+------------+-------+--------+----------+--+------------+
|    rs1    |  func3 |     rd     |    opcode      |    r_aui    | a_lsB[7:1] |
+-----------+--------+------------+----------------+-------------+------------+
 31       27 26    24 23        19 18            12 11          7 6          0

 where 'addr' has its top 20 bits from word1 and its bottom 12 bits from the
 top 12 bits of word2, even if the opcode of word2 does not have top 12 bits
 of immediate and rs1 the same as r_aui.  Why? to make unfiltering reliable
 and easy.  The effect of AUIPC is included in 'addr' by adding the buffer
 offset of word1, which makes 'addr' absolute, and thus more compressable.
 Unfiltering need only re-constitute 'addr' from big-endian, subtract the
 buffer offset, and re-position the bits.

#endif  //}

#ifndef CONDf //{ once-only during multiple #include of this file
// Input to filtering/scanning
#define AUIPC 0x17
#define opf(w) (0x7f& (w))
#define     rd(i_type) (037& ((i_type) >> 7))
#define func3f(i_type) (  7& ((i_type) >>12))
#define   rs1f(i_type) (037& ((i_type) >>15))

#define COND(word1) (AUIPC==opf(word1))

#define CONDf(word2, r_aui) ( r_aui==rs1f(word2) && ( \
         0x03==opf(word2) /*FETCH*/ \
     || (0x67==opf(word2) && 0==func3f(word2)) /*JALR*/ \
     || (0x13==opf(word2) && 0==func3f(word2)) /*ADDI*/ \
    ) )
// NYI: STOREs are ugly because immediate field is not contiguous
//    ||  0x23==opf(word2) /*STORE*/

static int get_ilen(int w)  // instruction length in bytes
{
    int ilen = 2;
    if (3 == (3& (w >>0))) {
        ilen += 2;
        if (7 == (7& (w >>2))) {
            ilen += 2;
            if (3 == (3& (w >>5))) { // same as stub/src/arch/riscv/64/bxx.S
                ilen += 2;
            }
            // NYI: 10 bytes or longer
        }
    }
    return ilen;
}
#endif  //}

static int F(Filter *f) {
#ifdef U
    // If both F and U defined then action is Filter; else Scan.
    // filter
    byte *b = f->buf;
#else
    // scan only
    const byte *b = f->buf -8;
#endif
    const int size = f->buf_len -8;

    int ic;
    int calls = 0, noncalls = 0;
    int lastcall = 0;
    int gi;
    int constexpr g_end = 1024;  // 32-bit host: 2GiB must hold input, output, upx, and stack
    int gang[g_end];

    int ilen = 0; (void)gang; // not needed, but placate ignorant compiler
    for (ic = 0; ic <= size; ic += ilen) {
        int b1 = *(ic + b);  // opcode and length are in first byte
        if (!COND(b1)) { // not interesting at all
            ilen = get_ilen(b1);
            continue;  // advance to next instr
        }
        // find the 'gang' of (nearly-)consecutive AUIPC
        gi = 0; gang[gi++] = ic;  // AUIPC gang leader
        for (int jc= ic; jc <= size; ) {
            // The previous instruction was AUIPC.  Filtering will
            // change it and the following 4 bytes.  If another AUIPC
            // starts in that "shadow", then that will create problems
            // for both filtering and unfiltering.  So find the "gang".
            int kc, b2;
            if ((  (kc= 4+ jc), (b2= *(kc+b)), (COND(b2)))
            ||  ((3!= (3& b2))  // after AUIPC is 2-byte instr
               && ((kc= 6+ jc), (b2= *(kc+b)), (COND(b2))) ) ) {
                if (size < kc) { // filtering changes 8 bytes; same limit as unfiltering
                    ilen = kc - ic;  // beyond 'size': also ends the outer 'for'
                    break;  // gang has ended
                }
                if (g_end <= gi)
                    throwCantPack("Cannot pack: gang of AUIPC aliases > 1024.  Use --no-filter.");
                gang[gi++] = kc;
                jc = kc;  // continue looking, starting after newest AUIPC
            }
            else { // gang has ended: no AUIPC in shadow of last one
                ilen = (get_ilen(b2) + kc) - ic;  // to next un-examined byte
                break;  // quit the inner 'for'; resume the outer 'for'
            }
        }
#ifdef U  //{ filtering
        // Process the gang in reverse order so that forward un-filtering works
        do {
            int jc = gang[--gi];
            int word1 = get_le32(0+jc+b); int r_aui = rd(word1);
            int word2 = get_le32(4+jc+b);
            int addr = (~0xfff& word1) + (word2 >>20);  // sign extend imm
            addr += jc;  // AUIPC result value: the true "hoisted" address

            *(0+jc+b) = ((1& addr) <<7) | AUIPC;  // lo bit of addr goes next to AUIPC
            set_be32(1+jc+b, addr);  // Note BIG_ENDIAN store at offset of 1 byte
            set_le32(4+jc+b, (word2 <<12) | (r_aui <<7) | (0x7f& (addr >>1)));
            ++calls;

            if (0) { // DEBUG
                char line[100];
                int len = snprintf(line, sizeof(line), "%#6x  %#8.8x %#8.8x  %#8.8x %#8.8x\n",
                    jc, word1, word2, get_le32(0+jc+b), get_le32(4+jc+b));
                if (len != write(7, line, len)) {
                    exit(7);
                }
            }
        } while (gi);
#endif  //}
    }

    f->calls = calls;
    f->noncalls = noncalls;
    f->lastcall = lastcall;

#if 0 || defined(TESTING)
    printf("\ncalls=%d  noncalls=%d  text_size=%#x\n",calls,noncalls,size);
#endif
    return 0;
}

#ifdef U  //{
// Input to un-filtering: then the second instruction has been rotated left
// by 12 bits, and the Immediate field replaced by r_aui and a_lsB[7:1] .
#define opu(w) opf((w) >>12)
#define func3u(i_type) (  7& ((i_type) >>24))
#define   rs1u(i_type) (037& ((i_type) >>27))

#define CONDu(word2, r_aui) ( r_aui==rs1u(word2) && ( \
         0x03==opu(word2) /*FETCH*/ \
     || (0x67==opu(word2) && 0==func3u(word2)) /*JALR*/ \
     || (0x13==opu(word2) && 0==func3u(word2)) /*ADDI*/ \
    ) )

static int U(Filter *f) {
    byte *b = f->buf;
    const int size = f->buf_len -8;

    int ic;
    int calls = 0, noncalls = 0;
    int lastcall = 0;

    int ilen;
    for (ic = 0; ic <= size; ic += ilen) {
        int word1 = get_le32(  ic+b);
        if (!COND(word1)) { // not interesting at all
            ilen = get_ilen(word1);
            continue;  // advance to next instr
        }
        int word2 = get_le32(4+ic+b);
        int r_aui = 037& (word2 >> 7);
        if (CONDu(word2, r_aui)) {
            lastcall = ic;
            ++calls;
        }
        else {
            ++noncalls;
        }

        int addr = get_be32(1+ic+b);  // Note BIG_ENDIAN fetch at 1-byte offset
        addr = (~0xff & addr) | ((0x7f& addr) <<1) | (1& (word1 >>7));
        addr -= ic;
        addr += ((1u <<11)& addr) <<1;  // 12-bit imm is sign-extended

        set_le32(0+ic+b, (~0xfff& addr) | (r_aui <<7) | AUIPC);
        set_le32(4+ic+b,    (addr <<20) | ((unsigned)word2 >>12));
        ilen = 4;  // resume scanning after AUIPC

        if (0) { // DEBUG
            char line[100];
            int len = snprintf(line, sizeof(line), "%#6x  %#8.8x %#8.8x  %#8.8x %#8.8x\n",
                ic, get_le32(0+ic+b), get_le32(4+ic+b), word1, word2);
            if (len != write(8, line, len)) {
                exit(8);
            }
        }
    }

    f->calls = calls;
    f->noncalls = noncalls;
    f->lastcall = lastcall;

#if 0 || defined(TESTING)
    printf("\ncalls=%d  noncalls=%d  text_size=%#x\n",calls,noncalls,size);
#endif
    return 0;
}
#endif  //}

/* vim:set ts=4 sw=4 et: */
