/* fctl_ml.ch -- filter CTO implementation by ML1050

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2000 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2000 Laszlo Molnar
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

   Markus F.X.J. Oberhumer                   Laszlo Molnar
   markus.oberhumer@jk.uni-linz.ac.at        ml1050@cdata.tvnet.hu
 */



/*************************************************************************
// these are not implemented here
**************************************************************************/

// filter: e8, e9, e8e9
#define f_cto32_e8              NULL
#define f_cto32_e9              NULL
#define f_cto32_e8e9            NULL

// unfilter: e8, e9, e8e9
#define u_cto32_e8              NULL
#define u_cto32_e9              NULL
#define u_cto32_e8e9            NULL

// scan: e8, e9, e8e9
#define s_cto32_e8              NULL
#define s_cto32_e9              NULL
#define s_cto32_e8e9            NULL

// filter: e8, e9, e8e9 with bswap be->le
#define f_cto32_e8_bswap_be     NULL
#define f_cto32_e9_bswap_be     NULL
#define f_cto32_e8e9_bswap_be   NULL

// unfilter: e8, e9, e8e9 with bswap be->le
#define u_cto32_e8_bswap_be     NULL
#define u_cto32_e9_bswap_be     NULL
#define u_cto32_e8e9_bswap_be   NULL

// scan: e8, e9, e8e9 with bswap be->le
#define s_cto32_e8_bswap_be     NULL
#define s_cto32_e9_bswap_be     NULL
#define s_cto32_e8e9_bswap_be   NULL


/*************************************************************************
//
**************************************************************************/

#define COND(b,x,lastcall)   (b[x] == 0xe8)
#define F           f_cto32_e8_bswap_le
#define U           u_cto32_e8_bswap_le
#include "fcto_ml2.ch"
#undef U
#undef F
#define F           s_cto32_e8_bswap_le
#include "fcto_ml2.ch"
#undef F
#undef COND

#define COND(b,x,lastcall)   (b[x] == 0xe9)
#define F           f_cto32_e9_bswap_le
#define U           u_cto32_e9_bswap_le
#include "fcto_ml2.ch"
#undef U
#undef F
#define F           s_cto32_e9_bswap_le
#include "fcto_ml2.ch"
#undef F
#undef COND

#define COND(b,x,lastcall)  (b[x] == 0xe8 || b[x] == 0xe9 \
                            || (lastcall!=(x) && 0xf==b[(x)-1] \
                               && 0x80<=b[x] && b[x]<=0x8f) )
#define F           f_cto32_e8e9_bswap_le
#define U           u_cto32_e8e9_bswap_le
#include "fcto_ml2.ch"
#undef U
#undef F
#define F           s_cto32_e8e9_bswap_le
#include "fcto_ml2.ch"
#undef F
#undef COND


/*
vi:ts=4:et
*/

