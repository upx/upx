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
// exceptions
//   ACC_CXX_NOTHROW
**************************************************************************/

#if (ACC_CC_GNUC && (ACC_CC_GNUC < 0x028000ul))
#  define ACC_CXX_NOTHROW
#elif (ACC_CC_BORLANDC && (__BORLANDC__ < 0x0450))
#  define ACC_CXX_NOTHROW
#elif (ACC_CC_TURBOC)
#  define ACC_CXX_NOTHROW
#elif (ACC_CC_MSC && (_MSC_VER < 1100))
#  define ACC_CXX_NOTHROW
#elif (ACC_CC_WATCOMC && !defined(_CPPUNWIND))
#  define ACC_CXX_NOTHROW
#endif

#if !defined(ACC_CXX_NOTHROW)
#  define ACC_CXX_NOTHROW   throw()
#endif


/*************************************************************************
// disable dynamic allocation of an object
//   ACC_CXX_DISABLE_NEW_DELETE
//   ACC_CXX_DISABLE_NEW_DELETE_STRICT
**************************************************************************/

#undef __ACC_CXX_HAVE_ARRAY_NEW
#undef __ACC_CXX_HAVE_PLACEMENT_NEW
#undef __ACC_CXX_HAVE_PLACEMENT_DELETE

#if (ACC_CC_BORLANDC && (__BORLANDC__ < 0x0450))
#elif (ACC_CC_GNUC && (ACC_CC_GNUC < 0x029000ul))
#  define ACC_CXX_DISABLE_NEW_DELETE private:
#elif (ACC_CC_MSC && ACC_MM_HUGE)
#  define ACC_CXX_DISABLE_NEW_DELETE private:
#elif (ACC_CC_MSC && (_MSC_VER < 1100))
#elif (ACC_CC_SYMANTECC || ACC_CC_ZORTECHC)
#elif (ACC_CC_TURBOC)
#elif (ACC_CC_WATCOMC && (__WATCOMC__ < 1100))
#else
#  define __ACC_CXX_HAVE_ARRAY_NEW 1
#endif

#if (__ACC_CXX_HAVE_ARRAY_NEW)
#  define __ACC_CXX_HAVE_PLACEMENT_NEW 1
#endif

#if (__ACC_CXX_HAVE_PLACEMENT_NEW)
#  if (ACC_CC_GNUC >= 0x030000ul)
#    define __ACC_CXX_HAVE_PLACEMENT_DELETE 1
#  elif (ACC_CC_INTELC)
#    define __ACC_CXX_HAVE_PLACEMENT_DELETE 1
#  elif (ACC_CC_MSC && (_MSC_VER >= 1200))
#    define __ACC_CXX_HAVE_PLACEMENT_DELETE 1
#  endif
#endif


#if !defined(__ACC_CXX_PLACEMENT_NEW) && (__ACC_CXX_HAVE_PLACEMENT_NEW)
#  define __ACC_CXX_PLACEMENT_NEW \
            static void* operator new(size_t, void*);
#endif
#if !defined(__ACC_CXX_PLACEMENT_DELETE) && (__ACC_CXX_HAVE_PLACEMENT_DELETE)
#  define __ACC_CXX_PLACEMENT_DELETE \
            static void operator delete(void*, void*) ACC_CXX_NOTHROW { }
#endif
#if !defined(__ACC_CXX_PLACEMENT_NEW)
#  define __ACC_CXX_PLACEMENT_NEW
#endif
#if !defined(__ACC_CXX_PLACEMENT_DELETE)
#  define __ACC_CXX_PLACEMENT_DELETE
#endif


#if !defined(ACC_CXX_DISABLE_NEW_DELETE) && (defined(new) || defined(delete))
#  define ACC_CXX_DISABLE_NEW_DELETE private:
#endif

#if !defined(ACC_CXX_DISABLE_NEW_DELETE) && !(__ACC_CXX_HAVE_ARRAY_NEW)
/* for old compilers use `protected' instead of `private' */
#  define ACC_CXX_DISABLE_NEW_DELETE \
        protected: static void operator delete(void*) ACC_CXX_NOTHROW { } \
        protected: static void* operator new(size_t); \
        private:
#endif

#if !defined(ACC_CXX_DISABLE_NEW_DELETE)
#  define ACC_CXX_DISABLE_NEW_DELETE \
        protected: \
            static void operator delete(void*) ACC_CXX_NOTHROW { } \
            static void operator delete[](void*) ACC_CXX_NOTHROW { } \
        private: \
            static void* operator new(size_t); \
            static void* operator new[](size_t);
#  define ACC_CXX_DISABLE_NEW_DELETE_STRICT \
        protected: \
            static void operator delete(void*) ACC_CXX_NOTHROW { } \
            static void operator delete[](void*) ACC_CXX_NOTHROW { } \
            __ACC_CXX_PLACEMENT_DELETE \
        private: \
            static void* operator new(size_t); \
            static void* operator new[](size_t); \
            __ACC_CXX_PLACEMENT_NEW
#endif

#if !defined(ACC_CXX_DISABLE_NEW_DELETE_STRICT)
#  define ACC_CXX_DISABLE_NEW_DELETE_STRICT ACC_CXX_DISABLE_NEW_DELETE
#endif



#endif /* __cplusplus */
#endif /* already included */


/*
vi:ts=4:et
*/
