/*
;  rename.ash --
;
;  This file is part of the UCL data compression library.
;
;  Copyright (C) 1996-2008 Markus Franz Xaver Johannes Oberhumer
;  All Rights Reserved.
;
;  The UCL library is free software; you can redistribute it and/or
;  modify it under the terms of the GNU General Public License as
;  published by the Free Software Foundation; either version 2 of
;  the License, or (at your option) any later version.
;
;  The UCL library is distributed in the hope that it will be useful,
;  but WITHOUT ANY WARRANTY; without even the implied warranty of
;  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;  GNU General Public License for more details.
;
;  You should have received a copy of the GNU General Public License
;  along with the UCL library; see the file COPYING.
;  If not, write to the Free Software Foundation, Inc.,
;  59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
;
;  Markus F.X.J. Oberhumer
;  <markus@oberhumer.com>
;  http://www.oberhumer.com/opensource/ucl/
;
*/


#undef ADDBITS
#undef ADDXBITS
#undef FILLBITS
#undef FILLBYTES_8
#undef FILLBYTES_LE32
#undef GETBIT

#undef decompr_break1
#undef decompr_end
#undef decompr_get_mlen
#undef decompr_get_mlen1
#undef decompr_get_mlen2
#undef decompr_got_mlen
#undef decompr_l1
#undef decompr_l2
#undef decompr_literal
#undef decompr_loop
#undef decompr_match
#undef decompr_start
#undef fillbytes_sr

#undef Lbreak
#undef Lbreak2
#undef Lcontinue
#undef Lcopy


#if defined(N)
#define ADDBITS                 N(ADDBITS)
#define ADDXBITS                N(ADDXBITS)
#define FILLBITS                N(FILLBITS)
#define FILLBYTES_8             N(FILLBYTES_8)
#define FILLBYTES_LE32          N(FILLBYTES_LE32)
#define GETBIT                  N(GETBIT)

#define decompr_break1          N(decompr_break1)
#define decompr_end             N(decompr_end)
#define decompr_get_mlen        N(decompr_get_mlen)
#define decompr_get_mlen1       N(decompr_get_mlen1)
#define decompr_get_mlen2       N(decompr_get_mlen2)
#define decompr_got_mlen        N(decompr_got_mlen)
#define decompr_l1              N(decompr_l1)
#define decompr_l2              N(decompr_l2)
#define decompr_literal         N(decompr_literal)
#define decompr_loop            N(decompr_loop)
#define decompr_match           N(decompr_match)
#define decompr_prev_dist       N(decompr_prev_dist)
#define decompr_start           N(decompr_start)
#define fillbytes_sr            N(fillbytes_sr)

#define Lbreak                  N(Lbreak)
#define Lbreak2                 N(Lbreak2)
#define Lcontinue               N(Lcontinue)
#define Lcopy                   N(Lcopy)
#endif


// vi:ts=8:et

