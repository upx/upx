/* pseudo <config.h> for Linux */

#ifndef __UPX_CONFIG_H
#define __UPX_CONFIG_H

/* $TOP$ */

/* Define to empty if the keyword does not work.  */
/* #undef const */

/* Define if your C compiler doesn't accept -c and -o together.  */
/* #undef NO_MINUS_C_MINUS_O */

/* Define as the return type of signal handlers (int or void).  */
#define RETSIGTYPE void

/* Define to `unsigned' if <sys/types.h> doesn't define.  */
/* #undef size_t */

/* Define if you have the ANSI C header files.  */
#define STDC_HEADERS 1

/* Define if you can safely include both <sys/time.h> and <time.h>.  */
#define TIME_WITH_SYS_TIME 1

/* Define if your memcmp is broken.  */
/* #undef NO_MEMCMP */

/* Define to `long' if <stddef.h> doesn't define.  */
/* #undef ptrdiff_t */

/* The number of bytes in a ptrdiff_t.  */
#define SIZEOF_PTRDIFF_T 4

/* The number of bytes in a size_t.  */
#define SIZEOF_SIZE_T 4

/* Define when using the dmalloc package.  */
/* #undef WITH_DMALLOC */

/* Define if you have the access function.  */
#define HAVE_ACCESS 1

/* Define if you have the atoi function.  */
#define HAVE_ATOI 1

/* Define if you have the chmod function.  */
#define HAVE_CHMOD 1

/* Define if you have the chown function.  */
#define HAVE_CHOWN 1

/* Define if you have the ctime function.  */
#define HAVE_CTIME 1

/* Define if you have the difftime function.  */
#define HAVE_DIFFTIME 1

/* Define if you have the fchmod function.  */
#define HAVE_FCHMOD 1

/* Define if you have the fileno function.  */
#define HAVE_FILENO 1

/* Define if you have the fstat function.  */
#define HAVE_FSTAT 1

/* Define if you have the XXX function.  */
#define HAVE_GETPID 1

/* Define if you have the XXX function.  */
#define HAVE_GETTIMEOFDAY 1

/* Define if you have the getumask function.  */
/* #undef HAVE_GETUMASK */

/* Define if you have the gmtime function.  */
#define HAVE_GMTIME 1

/* Define if you have the index function.  */
#define HAVE_INDEX 1

/* Define if you have the isatty function.  */
#define HAVE_ISATTY 1

/* Define if you have the lstat function.  */
#define HAVE_LSTAT 1

/* Define if you have the localtime function.  */
#define HAVE_LOCALTIME 1

/* Define if you have the memcmp function.  */
#define HAVE_MEMCMP 1

/* Define if you have the memcpy function.  */
#define HAVE_MEMCPY 1

/* Define if you have the memmove function.  */
#define HAVE_MEMMOVE 1

/* Define if you have the memset function.  */
#define HAVE_MEMSET 1

/* Define if you have the mktime function.  */
#define HAVE_MKTIME 1

/* Define if you have the setmode function.  */
/* #undef HAVE_SETMODE */

/* Define if you have the stat function.  */
#define HAVE_STAT 1

/* Define if you have the strcasecmp function.  */
#define HAVE_STRCASECMP 1

/* Define if you have the strchr function.  */
#define HAVE_STRCHR 1

/* Define if you have the strdup function.  */
#define HAVE_STRDUP 1

/* Define if you have the strftime function.  */
#define HAVE_STRFTIME 1

/* Define if you have the stricmp function.  */
/* #undef HAVE_STRICMP */

/* Define if you have the strncasecmp function.  */
#define HAVE_STRNCASECMP 1

/* Define if you have the strnicmp function.  */
/* #undef HAVE_STRNICMP */

/* Define if you have the strstr function.  */
#define HAVE_STRSTR 1

/* Define if you have the tzset function.  */
#define HAVE_TZSET 1

/* Define if you have the umask function.  */
#define HAVE_UMASK 1

/* Define if you have the utime function.  */
#define HAVE_UTIME 1

/* Define if you have the vsnprintf function.  */
#define HAVE_VSNPRINTF 1

/* Define if you have the <assert.h> header file.  */
#define HAVE_ASSERT_H 1

/* Define if you have the <ctype.h> header file.  */
#define HAVE_CTYPE_H 1

/* Define if you have the <curses.h> header file.  */
#define HAVE_CURSES_H 1

/* Define if you have the <limits.h> header file.  */
#define HAVE_LIMITS_H 1

/* Define if you have the <linux/kd.h> header file.  */
#define HAVE_LINUX_KD_H 1

/* Define if you have the <linux/kdev_t.h> header file.  */
#define HAVE_LINUX_KDEV_T_H 1

/* Define if you have the <linux/major.h> header file.  */
#define HAVE_LINUX_MAJOR_H 1

/* Define if you have the <memory.h> header file.  */
#define HAVE_MEMORY_H 1

/* Define if you have the <ncurses.h> header file.  */
#define HAVE_NCURSES_H 1

/* Define if you have the <signal.h> header file.  */
#define HAVE_SIGNAL_H 1

/* Define if you have the <stddef.h> header file.  */
#define HAVE_STDDEF_H 1

/* Define if you have the <sys/param.h> header file.  */
#define HAVE_SYS_PARAM_H 1

/* Define if you have the <sys/resource.h> header file.  */
#define HAVE_SYS_RESOURCE_H 1

/* Define if you have the <sys/time.h> header file.  */
#define HAVE_SYS_TIME_H 1

/* Define if you have the <sys/times.h> header file.  */
#define HAVE_SYS_TIMES_H 1

/* Define if you have the <sys/utime.h> header file.  */
/* #undef HAVE_SYS_UTIME_H */

/* Define if you have the <time.h> header file.  */
#define HAVE_TIME_H 1

/* Define if you have the <unistd.h> header file.  */
#define HAVE_UNISTD_H 1

/* Define if you have the <utime.h> header file.  */
#define HAVE_UTIME_H 1

/* $BOTTOM$ */

#if defined(HAVE_GMTIME) && !defined(TIME_WITH_SYS_TIME)
#  undef /**/ HAVE_GMTIME
#endif

#if defined(HAVE_LOCALTIME) && !defined(TIME_WITH_SYS_TIME)
#  undef /**/ HAVE_LOCALTIME
#endif

#if defined(HAVE_STRFTIME) && !defined(TIME_WITH_SYS_TIME)
#  undef /**/ HAVE_STRFTIME
#endif

#if defined(HAVE_SYS_RESOURCE_H) && !defined(TIME_WITH_SYS_TIME)
#  undef /**/ HAVE_SYS_RESOURCE_H
#endif

#if defined(HAVE_SYS_TIMES_H) && !defined(TIME_WITH_SYS_TIME)
#  undef /**/ HAVE_SYS_TIMES_H
#endif

#if (SIZEOF_PTRDIFF_T <= 0)
#  undef /**/ SIZEOF_PTRDIFF_T
#endif

#if (SIZEOF_SIZE_T <= 0)
#  undef /**/ SIZEOF_SIZE_T
#endif

#endif /* already included */

/*
vi:ts=4
*/
