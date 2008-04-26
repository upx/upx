/* ctok.h -- filter CTO implementation

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2008 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2008 Laszlo Molnar
   Copyright (C) 2000-2008 John F. Reiser
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

static int F(Filter *f)
{
#ifdef U
    // filter
    upx_byte *b = f->buf;
#else
    // scan
    const upx_byte *b = f->buf;
#endif
    const unsigned addvalue = f->addvalue;
    const unsigned size = f->buf_len;
    unsigned const id = f->id;

    unsigned ic, jc, kc;
    unsigned calls = 0, noncalls = 0, noncalls2 = 0;
    unsigned lastnoncall = size, lastcall = 0;

    // find a 16 MiB large empty address space
    {
        unsigned char buf[256];
        memset(buf,0,256);

        for (ic = 0; ic < size - 5; ic++)
        {
            if (!COND(b,ic,lastcall,id))
                continue;
            jc = get_le32(b+ic+1)+ic+1;
            if (jc < size)
            {
                if (jc + addvalue >= (1u << 24)) // hi 8 bits won't be cto8
                    return -1;
            }
            else
                buf[b[ic+1]] |= 1;
        }

        if (getcto(f, buf) < 0)
            return -1;
    }
    const unsigned char cto8 = f->cto;
#ifdef U
    const unsigned cto = (unsigned)f->cto << 24;
#endif

    for (ic = 0; ic < size - 5; ic++)
    {
        if (!COND(b,ic,lastcall,id))
            continue;
        jc = get_le32(b+ic+1)+ic+1;
        // try to detect 'real' calls only
        if (jc < size)
        {
            assert(jc + addvalue < (1u << 24)); // hi 8 bits won't be cto8
#ifdef U
            set_be32(b+ic+1,jc+addvalue+cto);
#endif
            if (ic - lastnoncall < 5)
            {
                // check the last 4 bytes before this call
                for (kc = 4; kc; kc--)
                    if (COND(b,ic-kc,lastcall,id) && b[ic-kc+1] == cto8)
                        break;
                if (kc)
                {
#ifdef U
                    // restore original
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

#if 0 || defined(TESTING)
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
    upx_byte *b = f->buf;
    const unsigned size5 = f->buf_len - 5;
    const unsigned addvalue = f->addvalue;
    const unsigned cto = (unsigned)f->cto << 24;
    unsigned const id = f->id;
    unsigned lastcall = 0;

    unsigned ic, jc;

    for (ic = 0; ic < size5; ic++)
        if (COND(b,ic,lastcall,id))
        {
            jc = get_be32(b+ic+1);
            if (b[ic+1] == f->cto)
            {
                set_le32(b+ic+1,jc-ic-1-addvalue-cto);
                f->calls++;
                ic += 4;
                f->lastcall = lastcall = ic+1;
            }
            else
                f->noncalls++;
        }
    return 0;
}
#endif


#undef F
#undef U


/*
vi:ts=4:et:nowrap
*/

