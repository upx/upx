/* ACC -- Automatic Compiler Configuration

   Copyright (C) 1996-2003 Markus Franz Xaver Johannes Oberhumer
   All Rights Reserved.

   This software is a copyrighted work licensed under the terms of
   the GNU General Public License. Please consult the file "ACC_LICENSE"
   for details.

   Markus F.X.J. Oberhumer
   <markus@oberhumer.com>
   http://www.oberhumer.com/
 */


#ifndef __ACC_CXX_H_INCLUDED
#define __ACC_CXX_H_INCLUDED
#if defined(__cplusplus)


/*************************************************************************
//
**************************************************************************/

#if !defined(ACC_CXX_NOTHROW)
#  define ACC_CXX_NOTHROW   throw()
#endif


/*************************************************************************
// disable dynamic allocation of an object
**************************************************************************/

#if defined(new) || defined(delete)
#elif (ACC_CC_SYMANTECC || ACC_CC_ZORTECHC)
#elif (ACC_CC_GNUC && (ACC_CC_GNUC < 0x029000ul))
#else
#  define __ACC_CXX_DISABLE_NEW_DELETE_PLACEMENT_NEW \
        static void* operator new(size_t, void*);
#  if (ACC_CC_GNUC >= 0x030000ul)
#    define __ACC_CXX_DISABLE_NEW_DELETE_PLACEMENT_DELETE \
        static void operator delete(void*, void*) ACC_CXX_NOTHROW { }
#  elif (ACC_CC_INTELC)
#    define __ACC_CXX_DISABLE_NEW_DELETE_PLACEMENT_DELETE \
        static void operator delete(void*, void*) ACC_CXX_NOTHROW { }
#  elif (ACC_CC_MSC && (_MSC_VER >= 1200))
#    define __ACC_CXX_DISABLE_NEW_DELETE_PLACEMENT_DELETE \
        static void operator delete(void*, void*) ACC_CXX_NOTHROW { }
#  endif
#  if !defined(__ACC_CXX_DISABLE_NEW_DELETE_PLACEMENT_NEW)
#    define __ACC_CXX_DISABLE_NEW_DELETE_PLACEMENT_NEW
#    undef __ACC_CXX_DISABLE_NEW_DELETE_PLACEMENT_DELETE
#  endif
#  if !defined(__ACC_CXX_DISABLE_NEW_DELETE_PLACEMENT_DELETE)
#    define __ACC_CXX_DISABLE_NEW_DELETE_PLACEMENT_DELETE
#  endif

#  define ACC_CXX_DISABLE_NEW_DELETE \
    private: \
        static void* operator new(size_t); \
        static void* operator new[](size_t); \
    protected: \
        static void operator delete(void*) ACC_CXX_NOTHROW { } \
        static void operator delete[](void*) ACC_CXX_NOTHROW { } \
    private:

#  define ACC_CXX_DISABLE_NEW_DELETE_STRICT \
    private: \
        static void* operator new(size_t); \
        static void* operator new[](size_t); \
        __ACC_CXX_DISABLE_NEW_DELETE_PLACEMENT_NEW \
    protected: \
        static void operator delete(void*) ACC_CXX_NOTHROW { } \
        static void operator delete[](void*) ACC_CXX_NOTHROW { } \
        __ACC_CXX_DISABLE_NEW_DELETE_PLACEMENT_DELETE \
    private:

#endif


#if !defined(ACC_CXX_DISABLE_NEW_DELETE)
#  define ACC_CXX_DISABLE_NEW_DELETE private:
#endif
#if !defined(ACC_CXX_DISABLE_NEW_DELETE_STRICT)
#  define ACC_CXX_DISABLE_NEW_DELETE_STRICT ACC_CXX_DISABLE_NEW_DELETE
#endif



#endif /* __cplusplus */
#endif /* already included */


/*
vi:ts=4:et
*/
