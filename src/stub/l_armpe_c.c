/* l_armpe_c.c -- ARM/PE decompressor for NRV2E

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2006 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2006 Laszlo Molnar
   Copyright (C) 2000-2006 John F. Reiser
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
   <mfx@users.sourceforge.net>          <ml1050@users.sourceforge.net>

   John F. Reiser
   <jreiser@users.sourceforge.net>
*/

#ifndef UCL_DECOMPRESS
# error please define UCL_DECOMPRESS
#endif

int UCL_DECOMPRESS(const unsigned char * src, unsigned src_len,
                   unsigned char * dst, unsigned * dst_len);

void *LoadLibraryW(const unsigned short *);
void *GetProcAddressA(const void *, const void *);
void *get_le32(const void *);
void reloc_main();

static void handle_imports(const unsigned char *imp, unsigned name_offset,
                           unsigned iat_offset)
{
    unsigned short buf[64];
    while (1)
    {
        unsigned short *b;
        //printf("name=%p iat=%p\n", get_le32(imp), get_le32(imp + 4));
        unsigned char *name = get_le32(imp);
        if (name == 0)
            break;
        name += name_offset;
        unsigned *iat = get_le32(imp + 4) + iat_offset;
        //printf("name=%p iat=%p\n", name, iat);
        for (b = buf; *name; name++, b++)
            *b = *name;
        *b = 0;

        void *dll = LoadLibraryW(buf);
        imp += 8;
        unsigned ord;

        while (*imp)
        {
            switch (*imp++)
            {
            case 1:
                // by name
                *iat++ = (unsigned) GetProcAddressA(dll, imp);
                while (*imp++)
                    ;
                break;
            case 0xff:
                // by ordinal
                ord = ((unsigned) imp[0]) + imp[1] * 0x100;
                imp += 2;
                *iat++ = (unsigned) GetProcAddressA(dll, (void *) ord);
                break;
            default:
                // *(int*) 1 = 0;
                break;
            }
        }
        imp++;
    }
}

// debugging stuff
int CFwrap(short *, int, int, int, int, int, int);
void WFwrap(int, const void *, int, int *, int);
void CHwrap(int);
#define WRITEFILE2(name0, buf, len) \
    do { short b[3]; b[0] = '\\'; b[1] = name0; b[2] = 0; \
    int h = CFwrap(b, 0x40000000L, 3, 0, 2, 0x80, 0);\
    int l; WFwrap(h, buf, len, &l, 0); \
    CHwrap(h); \
    } while (0)

void upx_main(const unsigned *info)
{
    unsigned dlen = 0;
#if 0
    unsigned src0 = *info++;
    unsigned srcl = *info++;
    unsigned dst0 = *info++;
    unsigned dstl = *info++;
    unsigned bimp = *info++;
    unsigned onam = *info++;
    unsigned getp = *info++;
    unsigned load = *info++;
    unsigned entr = *info++;
#else
    unsigned src0 = info[0];
    unsigned srcl = info[1];
    unsigned dst0 = info[2];
//    unsigned dstl = info[3];
    unsigned bimp = info[4];
    unsigned onam = info[5];
//    unsigned getp = info[6];
//    unsigned load = info[7];
//    unsigned entr = info[8];
#endif

#ifdef SAFE
    dlen = info[3];
#endif
    //WRITEFILE2('0', (void*) dst0, info[7] + 256 - dst0);
    UCL_DECOMPRESS((void *) src0, srcl, (void *) dst0, &dlen);
    //WRITEFILE2('1', (void*) dst0, info[7] + 256 - dst0);
    handle_imports((void *) bimp, onam, dst0);
    //WRITEFILE2('2', (void*) dst0, info[7] + 256 - dst0);
#ifdef STUB_FOR_DLL
    reloc_main();
    //WRITEFILE2('3', (void*) dst0, info[7] + 256 - dst0);
#endif
}
