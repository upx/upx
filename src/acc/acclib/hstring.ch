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


#if !defined(ACCLIB_PUBLIC)
#  define ACCLIB_PUBLIC(r,f)    r __ACCLIB_FUNCNAME(f)
#endif


/***********************************************************************
// strlen
************************************************************************/

ACCLIB_PUBLIC(acc_hsize_t, acc_hstrlen) (const acc_hchar_p s)
{
    /* TODO: which one is the fastest generic version? */
#if 1
    acc_hsize_t n = 0; while (*s) ++s, ++n; return n;
#elif 1
    acc_hsize_t n = 0; while (s[n]) ++n; return n;
#else
    const acc_hchar_p start = s; while (*s) ++s;
    return (acc_hsize_t) (s - start);
#endif
}


/***********************************************************************
// strlcpy, strlcat
************************************************************************/

ACCLIB_PUBLIC(acc_hsize_t, acc_hstrlcpy) (acc_hchar_p d, const acc_hchar_p s, acc_hsize_t size)
{
    acc_hsize_t n = 0;
    if (n != size) {
        do {
            if ((*d = *s) == 0) return n;
            ++d; ++s; ++n;
        } while (n != size);
        d[-1] = 0;
    }
    while (*s) ++s, ++n;
    return n;
}


ACCLIB_PUBLIC(acc_hsize_t, acc_hstrlcat) (acc_hchar_p d, const acc_hchar_p s, acc_hsize_t size)
{
    acc_hsize_t n = 0;
    while (*d && n != size) ++d, ++n;
    if (n != size) {
        do {
            if ((*d = *s) == 0) return n;
            ++d; ++s; ++n;
        } while (n != size);
        d[-1] = 0;
    }
    while (*s) ++s, ++n;
    return n;
}


/***********************************************************************
// strscpy, strscat - same as strlcpy/strlcat, but just return -1
// in case of failure
************************************************************************/

ACCLIB_PUBLIC(int, acc_hstrscpy) (acc_hchar_p d, const acc_hchar_p s, acc_hsize_t size)
{
    acc_hsize_t n = 0;
    if (n != size) {
        do {
            if ((*d = *s) == 0) return 0;
            ++d; ++s; ++n;
        } while (n != size);
        d[-1] = 0;
    }
    return -1;
}


ACCLIB_PUBLIC(int, acc_hstrscat) (acc_hchar_p d, const acc_hchar_p s, acc_hsize_t size)
{
    acc_hsize_t n = 0;
    while (*d && n != size) ++d, ++n;
    if (n != size) {
        do {
            if ((*d = *s) == 0) return 0;
            ++d; ++s; ++n;
        } while (n != size);
        d[-1] = 0;
    }
    return -1;
}


/***********************************************************************
// strchr, strrchr
************************************************************************/

ACCLIB_PUBLIC(acc_hchar_p, acc_hstrchr) (const acc_hchar_p s, int c)
{
    for ( ; *s; ++s) {
        if (*s == (char) c)
            __ACCLIB_CONST_CAST_RETURN(acc_hchar_p, s)
    }
    if ((char) c == 0) __ACCLIB_CONST_CAST_RETURN(acc_hchar_p, s)
    return (acc_hchar_p)0;
}


ACCLIB_PUBLIC(acc_hchar_p, acc_hstrrchr) (const acc_hchar_p s, int c)
{
    const acc_hchar_p start = s;
    while (*s) ++s;
    if ((char) c == 0) __ACCLIB_CONST_CAST_RETURN(acc_hchar_p, s)
    for (;;) {
        if (s == start) break; --s;
        if (*s == (char) c)
            __ACCLIB_CONST_CAST_RETURN(acc_hchar_p, s)
    }
    return (acc_hchar_p)0;
}


/***********************************************************************
// strspn, strrspn
************************************************************************/

ACCLIB_PUBLIC(acc_hsize_t, acc_hstrspn) (const acc_hchar_p s, const acc_hchar_p accept)
{
    acc_hsize_t n = 0;
    for ( ; *s; ++s) {
        const acc_hchar_p a;
        for (a = accept; *a; ++a)
            if (*s == *a)
                goto acc_label_next;
        break;
    acc_label_next:
        ++n;
    }
    return n;
}


ACCLIB_PUBLIC(acc_hsize_t, acc_hstrrspn) (const acc_hchar_p s, const acc_hchar_p accept)
{
    acc_hsize_t n = 0;
    const acc_hchar_p start = s;
    while (*s) ++s;
    for (;;) {
        const acc_hchar_p a;
        if (s == start) break; --s;
        for (a = accept; *a; ++a)
            if (*s == *a)
                goto acc_label_next;
        break;
    acc_label_next:
        ++n;
    }
    return n;
}


/***********************************************************************
// strcspn, strrcspn
************************************************************************/

ACCLIB_PUBLIC(acc_hsize_t, acc_hstrcspn) (const acc_hchar_p s, const acc_hchar_p reject)
{
    acc_hsize_t n = 0;
    for ( ; *s; ++s, ++n) {
        const acc_hchar_p r;
        for (r = reject; *r; ++r)
            if (*s == *r)
                return n;
    }
    return n;
}


ACCLIB_PUBLIC(acc_hsize_t, acc_hstrrcspn) (const acc_hchar_p s, const acc_hchar_p reject)
{
    acc_hsize_t n = 0;
    const acc_hchar_p start = s;
    while (*s) ++s;
    for ( ; ; ++n) {
        const acc_hchar_p r;
        if (s == start) break; --s;
        for (r = reject; *r; ++r)
            if (*s == *r)
                return n;
    }
    return n;
}


/***********************************************************************
// strpbrk, strrpbrk
************************************************************************/

ACCLIB_PUBLIC(acc_hchar_p, acc_hstrpbrk) (const acc_hchar_p s, const acc_hchar_p accept)
{
    for ( ; *s; ++s) {
        const acc_hchar_p a;
        for (a = accept; *a; ++a) {
            if (*s == *a)
                __ACCLIB_CONST_CAST_RETURN(acc_hchar_p, s)
        }
    }
    return (acc_hchar_p)0;
}


ACCLIB_PUBLIC(acc_hchar_p, acc_hstrrpbrk) (const acc_hchar_p s, const acc_hchar_p accept)
{
    const acc_hchar_p start = s;
    while (*s) ++s;
    for (;;) {
        const acc_hchar_p a;
        if (s == start) break; --s;
        for (a = accept; *a; ++a) {
            if (*s == *a)
                __ACCLIB_CONST_CAST_RETURN(acc_hchar_p, s)
        }
    }
    return (acc_hchar_p)0;
}


/***********************************************************************
// strsep, strrsep
************************************************************************/

ACCLIB_PUBLIC(acc_hchar_p, acc_hstrsep) (acc_hchar_pp ss, const acc_hchar_p delim)
{
    acc_hchar_p s = *ss;
    if (s) {
        acc_hchar_p p = __ACCLIB_FUNCNAME(acc_hstrpbrk)(s, delim);
        if (p) *p++ = 0;
        *ss = p;
    }
    return s;
}


ACCLIB_PUBLIC(acc_hchar_p, acc_hstrrsep) (acc_hchar_pp ss, const acc_hchar_p delim)
{
    acc_hchar_p s = *ss;
    if (s) {
        acc_hchar_p p = __ACCLIB_FUNCNAME(acc_hstrrpbrk)(s, delim);
        if (p) {
            *p++ = 0;
            return p;
        }
        *ss = (acc_hchar_p)0;
    }
    return s;
}


/*
vi:ts=4:et
*/
