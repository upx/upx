/* snprintf.h --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2008 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2008 Laszlo Molnar
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
 */


#ifndef __UPX_SNPRINTF_H
#define __UPX_SNPRINTF_H

#ifdef __cplusplus
extern "C" {
#endif


/*************************************************************************
//
**************************************************************************/

int __acc_cdecl    upx_vsnprintf(char *str, size_t count, const char *format, va_list ap);
int __acc_cdecl_va upx_snprintf(char *str, size_t count, const char *format,...);
int __acc_cdecl    upx_vasprintf(char **ptr, const char *format, va_list ap);
int __acc_cdecl_va upx_asprintf(char **ptr, const char *format, ...);

#if 1
#  undef sprintf
#  define sprintf   error_sprintf_is_dangerous_use_snprintf
#endif


#ifdef __cplusplus
}
#endif

#endif /* already included */


/*
vi:ts=4:et:nowrap
*/

