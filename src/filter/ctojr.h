/* ctojr.h -- filter CTO implementation; renumber destinations MRU

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2001 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2001 Laszlo Molnar
   Copyright (C) 2000-2001 John F. Reiser
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

   Markus F.X.J. Oberhumer   Laszlo Molnar           John F. Reiser
   markus@oberhumer.com      ml1050@cdata.tvnet.hu   jreiser@BitWagon.com
 */



/*************************************************************************
// filter / scan
**************************************************************************/

#ifdef U  //{
static int const N_MRU = 1024;  // does not have to be a power of 2

// Adaptively remember recent destinations.
static void
update_mru(
    unsigned const jc,  // destination address
    int const kh,  // mru[kh] is slot where found
    unsigned mru[N_MRU],  // circular buffer of most recent destinations
    int &hand,  // mru[hand] is most recent destination
    int &tail   // mru[tail] is beyond oldest destination ("cold cache" startup)
)
{
    if (0 > --hand) {
        hand = N_MRU -1;
    }
    unsigned const t = mru[hand];  // entry which will be overwritten by jc
    if (0!=t) { // have seen at least N_MRU destinations
        mru[kh] = t;
    }
    else { // "cold cache": keep active region contiguous
        if (0 > --tail) {
            tail = N_MRU -1;
        }
        unsigned const t2 = mru[tail];
        mru[tail] = 0;
        mru[kh] = t2;
    }
    mru[hand] = jc;
}
#endif  //}


static int F(Filter *f)
{
#ifdef U
    // filter
    upx_byte *const b = f->buf;
    const unsigned addvalue = f->addvalue;
#else
    // scan
    const upx_byte *b = f->buf;
#endif
    const unsigned size = f->buf_len;

    unsigned ic, jc, kc;
    unsigned calls = 0, noncalls = 0, noncalls2 = 0;
    unsigned lastnoncall = size, lastcall = 0;

#ifdef U  //{
    int hand = 0, tail = 0;
    unsigned mru[N_MRU];
    memset(&mru[0], 0, sizeof(mru));
    f->n_mru = N_MRU;
#endif  //}

    // FIXME: We must fit into 8MB because we steal one bit.
    // find a 16MB large empty address space
    {
        unsigned char buf[256];
        memset(buf,0,256);

        for (ic = 0; ic < size - 5; ic++)
            if (CONDF(b,ic,lastcall) && get_le32(b+ic+1)+ic+1 >= size)
            {
                buf[b[ic+1]] |= 1;
            }

        if (getcto(f, buf) < 0)
            return -1;
    }
    const unsigned char cto8 = f->cto;
#ifdef U
    const unsigned cto = (unsigned)cto8 << 24;
#endif

    for (ic = 0; ic < size - 5; ic++)
    {
        if (!CONDF(b,ic,lastcall))
            continue;
        jc = get_le32(b+ic+1)+ic+1;
        // try to detect 'real' calls only
        if (jc < size)
        {
#ifdef U
            if (COND2(b,lastcall,ic,ic-1,ic)) { // 6-byte Jcc <disp32>
                // Prefix 0x0f is constant, but opcode condition 0x80..0x8f
                // varies.  Because we store the destination (or its mru index)
                // in be32 big endian format, the low-addressed bytes
                // will tend to be constant.  Swap prefix and opcode
                // so that constants are together for better compression.
                unsigned char const t =
                b[ic-1];
                b[ic-1] = b[ic];
                          b[ic] = t;
            }
    // FIXME [?]: Extend to 8 bytes if "ADD ESP, byte 4*n" follows CALL.
    //   This will require two related cto's (consecutive, or otherwise).
            {
                // Recode the destination: narrower mru indices
                // should compress better than wider addresses.
                int k;
                for (k = 0; k < N_MRU; ++k) {
                    int kh = hand + k;
                    if (N_MRU <= kh) {
                        kh -= N_MRU;
                    }
                    if (mru[kh] == jc) { // destination was seen recently
                        set_be32(b+ic+1,((k<<1)|0)+cto);
                        update_mru(jc, kh, mru, hand, tail);
                        break;
                    }
                }
                if (k == N_MRU) { // loop failed; jc is not in mru[]
                    set_be32(b+ic+1,((jc<<1)|1)+cto);
                    // Adaptively remember recent destinations.
                    if (0 > --hand) {
                        hand = N_MRU -1;
                    }
                    mru[hand] = jc;
                }
            }
            jc += addvalue;
#endif
            if (ic - lastnoncall < 5)
            {
                // check the last 4 bytes before this call
                for (kc = 4; kc; kc--)
                    if (CONDF(b,ic-kc,lastcall) && b[ic-kc+1] == cto8)
                        break;
                if (kc)
                {
#ifdef U
                    // restore original
                    if (COND2(b,lastcall,ic,ic,ic-1)) {
                        // Unswap prefix and opcode for 6-byte Jcc <disp32>
                        unsigned char const t =
                        b[ic-1];
                        b[ic-1] = b[ic];
                                  b[ic] = t;
                    }
                    set_le32(b+ic+1,jc-ic-1);
#endif
                    if (b[ic+1] == cto8)
                        return 1;           // fail - buffer not restored
                    lastnoncall = ic;
                    noncalls2++;
                    continue;
                }
            }
            calls++;
            ic += 4;
            lastcall = ic+1;
        }
        else
        {
            assert(b[ic+1] != cto8);        // this should not happen
            lastnoncall = ic;
            noncalls++;
        }
    }

    f->calls = calls;
    f->noncalls = noncalls;
    f->lastcall = lastcall;

#ifdef TESTING
    printf("\ncalls=%d noncalls=%d noncalls2=%d text_size=%x calltrickoffset=%x\n",calls,noncalls,noncalls2,size,cto);
#endif
    return 0;
}


/*************************************************************************
// unfilter
**************************************************************************/

#ifdef U
static int U(Filter *f)
{
    unsigned ic, jc;

    upx_byte *const b = f->buf;
    const unsigned size5 = f->buf_len - 5;
    const unsigned cto = (unsigned)f->cto << 24;
    unsigned lastcall = 0;
    int hand = 0, tail = 0;
    unsigned mru[N_MRU];
    memset(&mru[0], 0, sizeof(mru));

    for (ic = 0; ic < size5; ic++) {
        if (CONDU(b,ic,lastcall))
        {
            jc = get_be32(b+ic+1) - cto;
            if (b[ic+1] == f->cto)
            {
                if (1&jc) { // 1st time at this destination
                    jc >>= 1;
                    if (0 > --hand) {
                        hand = N_MRU -1;
                    }
                    mru[hand] = jc;
                }
                else { // not 1st time at this destination
                    jc >>= 1;
                    int kh = jc + hand;
                    if (N_MRU <= kh) {
                        kh -= N_MRU;
                    }
                    jc = mru[kh];
                    update_mru(jc, kh, mru, hand, tail);
                }
                set_le32(b+ic+1,jc-ic-1);

                if (COND2(b,lastcall,ic,ic,ic-1)) {
                    // Unswap prefix and opcode for 6-byte Jcc <disp32>
                    unsigned char const t =
                    b[ic-1];
                    b[ic-1] = b[ic];
                              b[ic] = t;
                }
                f->calls++;
                ic += 4;
                f->lastcall = lastcall = ic+1;
            }
            else
                f->noncalls++;
        }
    }
    return 0;
}
#endif


#undef F
#undef U


/*
vi:ts=4:et:nowrap
*/

