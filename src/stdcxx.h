/* stdcxx.h --

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


#ifndef __UPX_STDCXX_H
#define __UPX_STDCXX_H

#ifdef __cplusplus

#define NOTHROW             ACC_CXX_NOTHROW
#define DISABLE_NEW_DELETE  ACC_CXX_DISABLE_NEW_DELETE


/*************************************************************************
// exceptions & RTTI
**************************************************************************/

#if (ACC_CC_BORLANDC && (__BORLANDC__ < 0x0530))

#include <stdcomp.h>
#undef RWSTD_MULTI_THREAD
#include <stdexcep.h>
#include <new.h>
#include <typeinfo.h>
namespace std { class bad_alloc { }; }

#elif (ACC_CC_DMC && (__DMC__ < 0x834))

#include <new.h>
#include <typeinfo.h>

namespace std {
class exception
{
public:
    exception() NOTHROW { }
    virtual ~exception() NOTHROW { }
    virtual const char* what() const NOTHROW { return "exception"; }
};
}

#elif (ACC_CC_SYMANTECC)

#include <new.h>
#include <typeinfo.h>

class exception
{
public:
    exception() NOTHROW { }
    virtual ~exception() NOTHROW { }
    virtual const char* what() const NOTHROW { return "exception"; }
};
#define bool int
#define true 1
#define false 0

#else

#include <exception>
#include <new>
#include <typeinfo>

#endif


#if (ACC_CC_BORLANDC)
using namespace std;
#elif (ACC_CC_DMC)
namespace std { class bad_alloc { }; }
#elif (ACC_CC_GNUC && ACC_OS_EMX)
#define std
#elif (ACC_CC_SYMANTECC)
#define std
class bad_alloc { };
#elif (ACC_CC_WATCOMC)
#define std
class bad_alloc { };
#endif


/*************************************************************************
// STL
**************************************************************************/

#ifdef WANT_STL

#if defined(__linux__)
#  define _NOTHREADS
#endif
#if defined(__GNUC__)
#  define __THROW_BAD_ALLOC     throw bad_alloc()
#  define __USE_MALLOC
#  define enable                upx_stl_enable
#endif
#if defined(_MSC_VER)
#  pragma warning(push)
#  pragma warning(disable: 4018 4100 4663)
#endif

#include <vector>

#if defined(_MSC_VER)
#  pragma warning(pop)
#endif

#endif /* WANT_STL */


#endif /* __cplusplus */

#endif /* already included */


/*
vi:ts=4:et:nowrap
*/

