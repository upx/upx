/* ACC -- Automatic Compiler Configuration

   Copyright (C) 1996-2004 Markus Franz Xaver Johannes Oberhumer
   All Rights Reserved.

   This software is a copyrighted work licensed under the terms of
   the GNU General Public License. Please consult the file "ACC_LICENSE"
   for details.

   Markus F.X.J. Oberhumer
   <markus@oberhumer.com>
   http://www.oberhumer.com/
 */


#ifndef __ACC_CXX_H_INCLUDED
#define __ACC_CXX_H_INCLUDED 1
#if defined(__cplusplus)


/*************************************************************************
// exception specification
//   ACC_CXX_NOTHROW
**************************************************************************/

#if defined(ACC_CXX_NOTHROW)
#elif (ACC_CC_GNUC && (ACC_CC_GNUC < 0x020800ul))
#elif (ACC_CC_BORLANDC && (__BORLANDC__ < 0x0450))
#elif (ACC_CC_HIGHC)
#elif (ACC_CC_MSC && (_MSC_VER < 1100))
#elif (ACC_CC_NDPC)
#elif (ACC_CC_TURBOC)
#elif (ACC_CC_WATCOMC && !defined(_CPPUNWIND))
#elif (ACC_CC_ZORTECHC)
#else
#  define ACC_CXX_NOTHROW   throw()
#endif

#if !defined(ACC_CXX_NOTHROW)
#  define ACC_CXX_NOTHROW
#endif


/*************************************************************************
// disable dynamic allocation of an object - private helpers
//   __ACC_CXX_DO_NEW
//   __ACC_CXX_DO_DELETE
**************************************************************************/

#if defined(__ACC_CXX_DO_NEW)
#elif (ACC_CC_NDPC || ACC_CC_PGI)
#  define __ACC_CXX_DO_NEW          { return 0; }
#else
#  define __ACC_CXX_DO_NEW          ;
#endif


/* need an implementation in case a class has virtual members */
#if defined(__ACC_CXX_DO_DELETE)
#elif (ACC_CC_BORLANDC || ACC_CC_TURBOC)
#  define __ACC_CXX_DO_DELETE       { }
#else
#  define __ACC_CXX_DO_DELETE       ACC_CXX_NOTHROW { }
#endif


/*************************************************************************
// disable dynamic allocation of an object
//   ACC_CXX_DISABLE_NEW_DELETE
**************************************************************************/

/*
#undef __ACC_CXX_HAVE_ARRAY_NEW
#undef __ACC_CXX_HAVE_PLACEMENT_NEW
#undef __ACC_CXX_HAVE_PLACEMENT_DELETE
*/

#if (ACC_CC_BORLANDC && (__BORLANDC__ < 0x0450))
#elif (ACC_CC_MSC && ACC_MM_HUGE)
#  define ACC_CXX_DISABLE_NEW_DELETE private:
#elif (ACC_CC_MSC && (_MSC_VER < 1100))
#elif (ACC_CC_NDPC)
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
#  elif (ACC_CC_PGI)
#    define __ACC_CXX_HAVE_PLACEMENT_DELETE 1
#  endif
#endif


#if defined(ACC_CXX_DISABLE_NEW_DELETE)
#elif defined(new) || defined(delete)
#  define ACC_CXX_DISABLE_NEW_DELETE private:
#elif (ACC_CC_GNUC && (ACC_CC_GNUC < 0x025b00ul))
#  define ACC_CXX_DISABLE_NEW_DELETE private:
#elif  (ACC_CC_HIGHC)
#  define ACC_CXX_DISABLE_NEW_DELETE private:
#elif !defined(__ACC_CXX_HAVE_ARRAY_NEW)
   /* for old compilers use `protected' instead of `private' */
#  define ACC_CXX_DISABLE_NEW_DELETE \
        protected: static void operator delete(void*) __ACC_CXX_DO_DELETE \
        protected: static void* operator new(size_t) __ACC_CXX_DO_NEW \
        private:
#else
#  define ACC_CXX_DISABLE_NEW_DELETE \
        protected: static void operator delete(void*) __ACC_CXX_DO_DELETE \
                   static void operator delete[](void*) __ACC_CXX_DO_DELETE \
        private:   static void* operator new(size_t)  __ACC_CXX_DO_NEW \
                   static void* operator new[](size_t) __ACC_CXX_DO_NEW
#endif


/*************************************************************************
// Assist the compiler by defining a unique location for vtables and RTTI.
// Every class which has virtual member functions or derives from a class
// with virtual members should put this macro at the very top of its
// class declaration.
//   ACC_CXX_TRIGGER_FUNCTION
**************************************************************************/

#if defined(ACC_CXX_TRIGGER_FUNCTION)
#else
#  define ACC_CXX_TRIGGER_FUNCTION \
        protected: virtual const void* acc_cxx_trigger_function() const; \
        private:
#endif

#if defined(ACC_CXX_TRIGGER_FUNCTION_IMPL)
#else
#  define ACC_CXX_TRIGGER_FUNCTION_IMPL(klass) \
        const void* klass::acc_cxx_trigger_function() const { return 0; }
#endif


#endif /* __cplusplus */
#endif /* already included */


/*
vi:ts=4:et
*/
