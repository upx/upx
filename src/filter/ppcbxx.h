/* ppcbxx.h -- PowerPC Branch trick

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2008 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2008 Laszlo Molnar
   Copyright (C) 2004-2008 John F. Reiser
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

   Markus F.X.J. Oberhumer              Laszlo Molnar
   <markus@oberhumer.com>               <ml1050@users.sourceforge.net>

   John F. Reiser
   <jreiser@users.sourceforge.net>
 */



/*************************************************************************
// filter / scan
**************************************************************************/

#define W_CTO 4  /* width of cto; must match stub/ppc_bxx.S */

static int F(Filter *f)
{
#ifdef U
    // filter
    upx_byte *b = f->buf;
    const unsigned addvalue = f->addvalue;
#else
    // scan
    const upx_byte *b = f->buf;
#endif
    const unsigned size  = umin(f->buf_len, -(~0u<<(32 - (6+ W_CTO))));
    const unsigned size4 = size -4;

    unsigned ic;
    unsigned calls = 0, noncalls = 0;
    unsigned lastnoncall = size, lastcall = 0;

    // find 26-2-W_CTO bits of displacements that contain no targets
    {
        unsigned char buf[256];
        unsigned short wbuf[256];
        const size_t WW = (size_t)0 - ((~(size_t)0) << W_CTO); // ???
        memset(wbuf, 0, sizeof(wbuf));
        memset(buf     , 0,       WW);
        memset(buf + WW, 1, 256 - WW);

        for (ic = 0; ic<=size4; ic+=4) if (COND(b,ic)) {
            unsigned const off = (int)(get_be32(b+ic)<<6) >>6;
            if (size <= (off & (~0u<<2))+ic) {
                buf[(~(~0u<<W_CTO)) & (off>>(26 - W_CTO))] |= 1;
                ++wbuf[0xff&(off>>18)];
            }
        }

        if (getcto(f, buf) < 0) {
            if (0!=W_CTO)  // FIXME: what is this ???
                return -1;
            f->cto = 0;
        }
    }
    const unsigned char cto8 = f->cto;
#ifdef U
    const unsigned cto = (unsigned)f->cto << (24+2 - W_CTO);
#endif

    for (ic = 0; ic<=size4; ic+=4) if (COND(b,ic)) {
        unsigned const word = get_be32(b+ic);
        unsigned const off = (int)(word<<6) >>6;
        unsigned const jc = (off & (~0u<<2))+ic;
        // try to detect 'real' calls only
        if (jc < size) {
#ifdef U
            set_be32(b+ic,(0xfc000003&word) | (jc+addvalue+cto));
#endif
            calls++;
            lastcall = ic;
        }
        else {
            assert(0==W_CTO  // FIXME: what is this ???
            || (~(~0u<<W_CTO) & (word>>(24+2 - W_CTO))) != cto8);  // this should not happen
            lastnoncall = ic;
            noncalls++;
        }
    }

    f->calls = calls;
    f->noncalls = noncalls;
    f->lastcall = lastcall;

#if 0 || defined(TESTING)
    printf("\ncalls=%d noncalls=%d text_size=%x calltrickoffset=%x\n",
        calls,noncalls,size,cto8);
#endif
    return 0;
}


/*************************************************************************
// unfilter
**************************************************************************/

#ifdef U
static int U(Filter *f)
{
    upx_byte *b = f->buf;
    const unsigned size4 = umin(f->buf_len - 4, -(~0u<<(32 - (6+ W_CTO))));
    const unsigned addvalue = f->addvalue;

    unsigned ic;

   for (ic = 0; ic<=size4; ic+=4) if (COND(b,ic)) {
        unsigned const word = get_be32(b+ic);
        if ((~(~0u<<W_CTO) & (word>>(24+2 - W_CTO))) == f->cto) {
            unsigned const jc = word & (~(~0u<<(26 - W_CTO)) & (~0u<<2));
            set_be32(b+ic, (0xfc000003&word)|(0x03fffffc&(jc-ic-addvalue)));
            f->calls++;
            f->lastcall = ic;
        }
        else {
            f->noncalls++;
        }
    }
    return 0;
}
#endif


#undef F
#undef U

#undef W_CTO

/*
vi:ts=4:et:nowrap
*/

