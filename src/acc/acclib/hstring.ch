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


#define __ACCLIB_HSTRING_CH_INCLUDED 1
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
    const acc_hchar_p start = s; while (*s) ++s;
    return (acc_hsize_t) (s - start);
#elif 1
    acc_hsize_t n = 0; while (*s) ++s, ++n; return n;
#elif 1
    acc_hsize_t n = 0; while (s[n]) ++n; return n;
#endif
}


/***********************************************************************
// strcmp, strncmp, ascii_stricmp, ascii_strnicmp, ascii_memicmp
************************************************************************/

ACCLIB_PUBLIC(int, acc_hstrcmp) (const acc_hchar_p p, const acc_hchar_p s)
{
    for ( ; *p; ++p, ++s) {
        if (*p != *s)
            break;
    }
    return (unsigned char)*p - (unsigned char)*s;
}


ACCLIB_PUBLIC(int, acc_hstrncmp) (const acc_hchar_p p, const acc_hchar_p s, acc_hsize_t n)
{
    for ( ; *p && n > 0; ++p, ++s, --n) {
        if (*p != *s)
            break;
    }
    return (unsigned char)*p - (unsigned char)*s;
}


ACCLIB_PUBLIC(int, acc_ascii_hstricmp) (const acc_hchar_p p, const acc_hchar_p s)
{
    for ( ; *p; ++p, ++s) {
        if (*p != *s) {
            int d = acc_ascii_utolower(*p) - acc_ascii_utolower(*s);
            if (d != 0)
                return d;
        }
    }
    return acc_ascii_utolower(*p) - acc_ascii_utolower(*s);
}


ACCLIB_PUBLIC(int, acc_ascii_hstrnicmp) (const acc_hchar_p p, const acc_hchar_p s, acc_hsize_t n)
{
    for ( ; *p && n > 0; ++p, ++s, --n) {
        if (*p != *s) {
            int d = acc_ascii_utolower(*p) - acc_ascii_utolower(*s);
            if (d != 0)
                return d;
        }
    }
    return acc_ascii_utolower(*p) - acc_ascii_utolower(*s);
}


ACCLIB_PUBLIC(int, acc_ascii_hmemicmp) (const acc_hvoid_p pp, const acc_hvoid_p ss, acc_hsize_t n)
{
    const acc_hbyte_p p = (const acc_hbyte_p) pp;
    const acc_hbyte_p s = (const acc_hbyte_p) ss;
    for ( ; n > 0; ++p, ++s, --n) {
        if (*p != *s) {
            int d = acc_ascii_utolower(*p) - acc_ascii_utolower(*s);
            if (d != 0)
                return d;
        }
    }
    return acc_ascii_utolower(*p) - acc_ascii_utolower(*s);
}


/***********************************************************************
// strstr, ascii_stristr, memmem, ascii_memimem
************************************************************************/

ACCLIB_PUBLIC(acc_hchar_p, acc_hstrstr) (const acc_hchar_p p, const acc_hchar_p s)
{
    acc_hsize_t pn = __ACCLIB_FUNCNAME(acc_hstrlen)(p);
    acc_hsize_t sn = __ACCLIB_FUNCNAME(acc_hstrlen)(s);
    return (acc_hchar_p) __ACCLIB_FUNCNAME(acc_hmemmem)(p, pn, s, sn);
}


ACCLIB_PUBLIC(acc_hchar_p, acc_ascii_hstristr) (const acc_hchar_p p, const acc_hchar_p s)
{
    acc_hsize_t pn = __ACCLIB_FUNCNAME(acc_hstrlen)(p);
    acc_hsize_t sn = __ACCLIB_FUNCNAME(acc_hstrlen)(s);
    return (acc_hchar_p) __ACCLIB_FUNCNAME(acc_ascii_hmemimem)(p, pn, s, sn);
}


ACCLIB_PUBLIC(acc_hvoid_p, acc_hmemmem) (const acc_hvoid_p p, acc_hsize_t pn, const acc_hvoid_p s, acc_hsize_t sn)
{
#define __ACCLIB_REQUIRE_HMEMCPY_CH 1
    if (sn == 0) __ACCLIB_CONST_CAST_RETURN(acc_hvoid_p, p)
    for ( ; pn >= sn; --pn) {
        if (__ACCLIB_FUNCNAME(acc_hmemcmp)(p, s, sn) == 0)
            __ACCLIB_CONST_CAST_RETURN(acc_hvoid_p, p)
        p = (const acc_hbyte_p)p + 1;
    }
    return (acc_hvoid_p) 0;
}


ACCLIB_PUBLIC(acc_hvoid_p, acc_ascii_hmemimem) (const acc_hvoid_p p, acc_hsize_t pn, const acc_hvoid_p s, acc_hsize_t sn)
{
    if (sn == 0) __ACCLIB_CONST_CAST_RETURN(acc_hvoid_p, p)
    for ( ; pn >= sn; --pn) {
        if (__ACCLIB_FUNCNAME(acc_ascii_hmemicmp)(p, s, sn) == 0)
            __ACCLIB_CONST_CAST_RETURN(acc_hvoid_p, p)
        p = (const acc_hbyte_p)p + 1;
    }
    return (acc_hvoid_p) 0;
}


/***********************************************************************
// strcpy, strcat
************************************************************************/

ACCLIB_PUBLIC(acc_hchar_p, acc_hstrcpy) (acc_hchar_p d, const acc_hchar_p s)
{
    acc_hchar_p dest = d;
    while ((*d = *s) != 0) {
        ++d; ++s;
    }
    return dest;
}


ACCLIB_PUBLIC(acc_hchar_p, acc_hstrcat) (acc_hchar_p d, const acc_hchar_p s)
{
    acc_hchar_p dest = d;
    while (*d) ++d;
    while ((*d = *s) != 0) {
        ++d; ++s;
    }
    return dest;
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
// strscpy, strscat
// (same as strlcpy/strlcat, but just return -1 in case of failure)
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
// strccpy, memccpy
************************************************************************/

ACCLIB_PUBLIC(acc_hchar_p, acc_hstrccpy) (acc_hchar_p d, const acc_hchar_p s, int c)
{
    for (;;) {
        if ((*d = *s) == 0) break;
        if (*d++ == (char) c) return d;
        ++s;
    }
    return (acc_hchar_p) 0;
}


ACCLIB_PUBLIC(acc_hvoid_p, acc_hmemccpy) (acc_hvoid_p d, const acc_hvoid_p s, int c, acc_hsize_t n)
{
    acc_hbyte_p a = (acc_hbyte_p) d;
    const acc_hbyte_p p = (const acc_hbyte_p) s;
    const acc_hbyte_p e = (const acc_hbyte_p) s + n;
    while (p != e) {
        if ((*a++ = *p++) == (unsigned char) c)
            return a;
    }
    return (acc_hvoid_p) 0;
}


/***********************************************************************
// strchr, strrchr, ascii_strichr, ascii_strrichr
************************************************************************/

ACCLIB_PUBLIC(acc_hchar_p, acc_hstrchr) (const acc_hchar_p s, int c)
{
    for ( ; *s; ++s) {
        if (*s == (char) c)
            __ACCLIB_CONST_CAST_RETURN(acc_hchar_p, s)
    }
    if ((char) c == 0) __ACCLIB_CONST_CAST_RETURN(acc_hchar_p, s)
    return (acc_hchar_p) 0;
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
    return (acc_hchar_p) 0;
}


ACCLIB_PUBLIC(acc_hchar_p, acc_ascii_hstrichr) (const acc_hchar_p s, int c)
{
    c = acc_ascii_utolower(c);
    for ( ; *s; ++s) {
        if (acc_ascii_utolower(*s) == c)
            __ACCLIB_CONST_CAST_RETURN(acc_hchar_p, s)
    }
    if (c == 0) __ACCLIB_CONST_CAST_RETURN(acc_hchar_p, s)
    return (acc_hchar_p) 0;
}


ACCLIB_PUBLIC(acc_hchar_p, acc_ascii_hstrrichr) (const acc_hchar_p s, int c)
{
    const acc_hchar_p start = s;
    c = acc_ascii_utolower(c);
    while (*s) ++s;
    if (c == 0) __ACCLIB_CONST_CAST_RETURN(acc_hchar_p, s)
    for (;;) {
        if (s == start) break; --s;
        if (acc_ascii_utolower(*s) == c)
            __ACCLIB_CONST_CAST_RETURN(acc_hchar_p, s)
    }
    return (acc_hchar_p) 0;
}


/***********************************************************************
// memchr, memrchr, ascii_memichr, ascii_memrichr
************************************************************************/

ACCLIB_PUBLIC(acc_hvoid_p, acc_hmemchr) (const acc_hvoid_p s, int c, acc_hsize_t n)
{
    const acc_hbyte_p p = (const acc_hbyte_p) s;
    const acc_hbyte_p e = (const acc_hbyte_p) s + n;
    while (p != e) {
        if (*p == (unsigned char) c)
            __ACCLIB_CONST_CAST_RETURN(acc_hvoid_p, p)
        ++p;
    }
    return (acc_hvoid_p) 0;
}


ACCLIB_PUBLIC(acc_hvoid_p, acc_hmemrchr) (const acc_hvoid_p s, int c, acc_hsize_t n)
{
    const acc_hbyte_p p = (const acc_hbyte_p) s + n;
    const acc_hbyte_p e = (const acc_hbyte_p) s;
    while (p != e) {
        --p;
        if (*p == (unsigned char) c)
            __ACCLIB_CONST_CAST_RETURN(acc_hvoid_p, p)
    }
    return (acc_hvoid_p) 0;
}


ACCLIB_PUBLIC(acc_hvoid_p, acc_ascii_hmemichr) (const acc_hvoid_p s, int c, acc_hsize_t n)
{
    const acc_hbyte_p p = (const acc_hbyte_p) s;
    const acc_hbyte_p e = (const acc_hbyte_p) s + n;
    c = acc_ascii_utolower(c);
    while (p != e) {
        if (acc_ascii_utolower(*p) == c)
            __ACCLIB_CONST_CAST_RETURN(acc_hvoid_p, p)
        ++p;
    }
    return (acc_hvoid_p) 0;
}


ACCLIB_PUBLIC(acc_hvoid_p, acc_ascii_hmemrichr) (const acc_hvoid_p s, int c, acc_hsize_t n)
{
    const acc_hbyte_p p = (const acc_hbyte_p) s + n;
    const acc_hbyte_p e = (const acc_hbyte_p) s;
    c = acc_ascii_utolower(c);
    while (p != e) {
        --p;
        if (acc_ascii_utolower(*p) == c)
            __ACCLIB_CONST_CAST_RETURN(acc_hvoid_p, p)
    }
    return (acc_hvoid_p) 0;
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
    return (acc_hchar_p) 0;
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
    return (acc_hchar_p) 0;
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
        *ss = (acc_hchar_p) 0;
    }
    return s;
}


/***********************************************************************
// ascii_strlwr, ascii_strupr, ascii_memlwr, ascii_memupr
************************************************************************/

ACCLIB_PUBLIC(acc_hchar_p, acc_ascii_hstrlwr) (acc_hchar_p s)
{
    acc_hbyte_p p = (acc_hbyte_p) s;
    for ( ; *p; ++p)
        *p = (unsigned char) acc_ascii_utolower(*p);
    return s;
}


ACCLIB_PUBLIC(acc_hchar_p, acc_ascii_hstrupr) (acc_hchar_p s)
{
    acc_hbyte_p p = (acc_hbyte_p) s;
    for ( ; *p; ++p)
        *p = (unsigned char) acc_ascii_utoupper(*p);
    return s;
}


ACCLIB_PUBLIC(acc_hvoid_p, acc_ascii_hmemlwr) (acc_hvoid_p s, acc_hsize_t n)
{
    acc_hbyte_p p = (acc_hbyte_p) s;
    acc_hbyte_p e = (acc_hbyte_p) s + n;
    for ( ; p != e; ++p)
        *p = (unsigned char) acc_ascii_utolower(*p);
    return s;
}


ACCLIB_PUBLIC(acc_hvoid_p, acc_ascii_hmemupr) (acc_hvoid_p s, acc_hsize_t n)
{
    acc_hbyte_p p = (acc_hbyte_p) s;
    acc_hbyte_p e = (acc_hbyte_p) s + n;
    for ( ; p != e; ++p)
        *p = (unsigned char) acc_ascii_utoupper(*p);
    return s;
}


/*
vi:ts=4:et
*/
