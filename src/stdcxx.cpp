/* stdcxx.cpp --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2002 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2002 Laszlo Molnar
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
 */


//#define WANT_STL
#include "conf.h"
#include "stdcxx.h"


#ifdef WANT_STL

#if defined(__GNUC__)
// provide missing oom_handler
void (*__malloc_alloc_template<0>::__malloc_alloc_oom_handler)() = 0;
# if !defined(__USE_MALLOC)
// instantiate default allocator
template class __default_alloc_template<false, 0>;
# endif
#endif

#endif



/*
vi:ts=4:et:nowrap
*/

