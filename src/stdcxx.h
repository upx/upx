/* stdcxx.h --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2003 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2003 Laszlo Molnar
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


#ifndef __UPX_STDCXX_H
#define __UPX_STDCXX_H

#ifdef __cplusplus


#if !defined(NOTHROW)
#  define NOTHROW throw()
#endif


/*************************************************************************
// disable dynamic allocation of an object
**************************************************************************/

#if defined(new) || defined(delete) || defined(__EMX__)

// debug
#  define DISABLE_NEW_DELETE private:

#else

#  if 1
#    define DISABLE_NEW_DELETE_PLACEMENT_NEW \
        static void *operator new(size_t, void *);
#  endif
#  if defined(__GNUC__) && (__GNUC__ >= 3)
#    define DISABLE_NEW_DELETE_PLACEMENT_DELETE \
        static void operator delete(void *, void *) NOTHROW { }
#  elif defined(__INTEL_COMPILER)
#    define DISABLE_NEW_DELETE_PLACEMENT_DELETE \
        static void operator delete(void *, void *) NOTHROW { }
#  elif defined(_MSC_VER) && (_MSC_VER >= 1200)
#    define DISABLE_NEW_DELETE_PLACEMENT_DELETE \
        static void operator delete(void *, void *) NOTHROW { }
#  endif

#  if !defined(DISABLE_NEW_DELETE_PLACEMENT_NEW)
#    define DISABLE_NEW_DELETE_PLACEMENT_NEW
#    undef DISABLE_NEW_DELETE_PLACEMENT_DELETE
#  endif
#  if !defined(DISABLE_NEW_DELETE_PLACEMENT_DELETE)
#    define DISABLE_NEW_DELETE_PLACEMENT_DELETE
#  endif

#  define DISABLE_NEW_DELETE \
private: \
    static void *operator new(size_t); \
    static void *operator new[](size_t); \
    DISABLE_NEW_DELETE_PLACEMENT_NEW \
protected: \
    static void operator delete(void *) NOTHROW { } \
    static void operator delete[](void *) NOTHROW { } \
    DISABLE_NEW_DELETE_PLACEMENT_DELETE \
private:

#endif


/*************************************************************************
// exceptions & RTTI
**************************************************************************/

#if defined(__DMC__)

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
class bad_alloc { };
};

#else

#include <exception>
#include <new>
#include <typeinfo>

#endif


#if defined(__EMX__)
#define std
#elif defined(__WATCOMC__)
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

