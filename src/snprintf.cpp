/*
 * Copyright Patrick Powell 1995
 * This code is based on code written by Patrick Powell (papowell@astart.com)
 * It may be used for any purpose as long as this notice remains intact
 * on all source code distributions
 */

/**************************************************************
 * Original:
 * Patrick Powell Tue Apr 11 09:48:21 PDT 1995
 * A bombproof version of doprnt (dopr) included.
 * Sigh.  This sort of thing is always nasty do deal with.  Note that
 * the version here does not include floating point...
 *
 * snprintf() is used instead of sprintf() as it does limit checks
 * for string length.  This covers a nasty loophole.
 *
 * The other functions are there to prevent NULL pointers from
 * causing nast effects.
 *
 * More Recently:
 *  Brandon Long <blong@fiction.net> 9/15/96 for mutt 0.43
 *  This was ugly.  It is still ugly.  I opted out of floating point
 *  numbers, but the formatter understands just about everything
 *  from the normal C string format, at least as far as I can tell from
 *  the Solaris 2.5 printf(3S) man page.
 *
 *  Brandon Long <blong@fiction.net> 10/22/97 for mutt 0.87.1
 *    Ok, added some minimal floating point support, which means this
 *    probably requires libm on most operating systems.  Don't yet
 *    support the exponent (e,E) and sigfig (g,G).  Also, fmtint()
 *    was pretty badly broken, it just wasn't being exercised in ways
 *    which showed it, so that's been fixed.  Also, formated the code
 *    to mutt conventions, and removed dead code left over from the
 *    original.  Also, there is now a builtin-test, just compile with:
 *           gcc -DTEST_SNPRINTF -o snprintf snprintf.c -lm
 *    and run snprintf for results.
 *
 *  Thomas Roessler <roessler@guug.de> 01/27/98 for mutt 0.89i
 *    The PGP code was using unsigned hexadecimal formats.
 *    Unfortunately, unsigned formats simply didn't work.
 *
 *  Michael Elkins <me@cs.hmc.edu> 03/05/98 for mutt 0.90.8
 *    The original code assumed that both snprintf() and vsnprintf() were
 *    missing.  Some systems only have snprintf() but not vsnprintf(), so
 *    the code is now broken down under HAVE_SNPRINTF and HAVE_VSNPRINTF.
 *
 *  Andrew Tridgell (tridge@samba.org) Oct 1998
 *    fixed handling of %.0f
 *    added test for HAVE_LONG_DOUBLE
 *
 * tridge@samba.org, idra@samba.org, April 2001
 *    got rid of fcvt code (twas buggy and made testing harder)
 *    added C99 semantics
 *
 * markus@oberhumer.com, August 2002
 *    large modifications for use in UPX
 *
 **************************************************************/

#if 1
#include "conf.h"
#else
#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#undef NDEBUG
#include <assert.h>
#endif

#undef LLONG
#undef ULLONG
#define LLONG upx_int64_t
#define ULLONG upx_uint64_t

#undef NO_FLOAT
#undef LDOUBLE
#if 1
#define NO_FLOAT 1
#define float error no_float
#define double error no_float
#else
#if (HAVE_LONG_DOUBLE)
#define LDOUBLE long double
#else
#define LDOUBLE double
#endif
#endif

/*
 * dopr(): poor man's version of doprintf
 */

/* format read states */
#define DP_S_DEFAULT 0
#define DP_S_FLAGS 1
#define DP_S_MIN 2
#define DP_S_DOT 3
#define DP_S_MAX 4
#define DP_S_MOD 5
#define DP_S_CONV 6
#define DP_S_DONE 7

/* format flags - Bits */
#define DP_F_MINUS (1 << 0)
#define DP_F_PLUS (1 << 1)
#define DP_F_SPACE (1 << 2)
#define DP_F_NUM (1 << 3)
#define DP_F_ZERO (1 << 4)
#define DP_F_UP (1 << 5)
#define DP_F_UNSIGNED (1 << 6)

/* Length modifier */
#define DP_C_CHAR 1
#define DP_C_SHORT 2
#define DP_C_LONG 3
#define DP_C_LLONG 4
#define DP_C_LDOUBLE 5

#define char_to_int(p) ((p) - '0')
#undef MAX
#define MAX(p, q) (((p) >= (q)) ? (p) : (q))

/*************************************************************************
 //
**************************************************************************/

__acc_static_forceinline void dopr_outch(char *buffer, size_t *currsize, size_t maxsize, int c) {
    if (*currsize < maxsize)
        buffer[*currsize] = (char) c;
    *currsize += 1;
}

static void fmtstr(char *buffer, size_t *currsize, size_t maxsize, const char *strvalue, int strln,
                   int flags, int min, int max) {
    int padlen; /* amount to pad */
    int cnt = 0;

#ifdef DEBUG_SNPRINTF
    printf("fmtstr min=%d max=%d s=[%s]\n", min, max, strvalue);
#endif
    assert(strvalue != NULL);
    padlen = min - strln;
    if (padlen < 0)
        padlen = 0;
    if (flags & DP_F_MINUS)
        padlen = -padlen; /* Left Justify */

    while (cnt < max && padlen > 0) {
        dopr_outch(buffer, currsize, maxsize, ' ');
        --padlen;
        ++cnt;
    }
    while (cnt < max && *strvalue) {
        dopr_outch(buffer, currsize, maxsize, *strvalue);
        ++strvalue;
        ++cnt;
    }
    while (cnt < max && padlen < 0) {
        dopr_outch(buffer, currsize, maxsize, ' ');
        ++padlen;
        ++cnt;
    }
}

/* Have to handle DP_F_NUM (ie 0x and 0 alternates) */
static void fmtint(char *buffer, size_t *currsize, size_t maxsize, LLONG value, unsigned base,
                   int min, int max, int flags) {
    int signvalue = 0;
    ULLONG uvalue;
    char convert[64 + 1];
    int place = 0;
    int spadlen = 0; /* amount to space pad */
    int zpadlen = 0; /* amount to zero pad */
    const char *digits;

    if (min < 0)
        min = 0;
    if (max < 0)
        max = 0;

    uvalue = value;
    if (!(flags & DP_F_UNSIGNED)) {
        if (value < 0) {
            signvalue = '-';
            uvalue = -value;
        } else {
            if (flags & DP_F_PLUS) /* Do a sign (+/i) */
                signvalue = '+';
            else if (flags & DP_F_SPACE)
                signvalue = ' ';
        }
    }

    digits = (flags & DP_F_UP) ? "0123456789ABCDEF" : "0123456789abcdef";
    do {
        convert[place] = digits[(unsigned) (uvalue % base)];
        uvalue /= base;
    } while (++place < (int) sizeof(convert) - 1 && uvalue);
    convert[place] = 0;

    zpadlen = max - place;
    spadlen = min - MAX(max, place) - (signvalue ? 1 : 0);
    if (zpadlen < 0)
        zpadlen = 0;
    if (spadlen < 0)
        spadlen = 0;
    if (flags & DP_F_ZERO) {
        zpadlen = MAX(zpadlen, spadlen);
        spadlen = 0;
    }
    if (flags & DP_F_MINUS)
        spadlen = -spadlen; /* Left Justifty */

#ifdef DEBUG_SNPRINTF
    printf("zpad: %d, spad: %d, min: %d, max: %d, place: %d\n", zpadlen, spadlen, min, max, place);
#endif

    /* Spaces */
    while (spadlen > 0) {
        dopr_outch(buffer, currsize, maxsize, ' ');
        --spadlen;
    }

    /* Sign */
    if (signvalue)
        dopr_outch(buffer, currsize, maxsize, signvalue);

    /* Zeros */
    while (zpadlen > 0) {
        dopr_outch(buffer, currsize, maxsize, '0');
        --zpadlen;
    }

    /* Digits */
    while (place > 0)
        dopr_outch(buffer, currsize, maxsize, convert[--place]);

    /* Left Justified spaces */
    while (spadlen < 0) {
        dopr_outch(buffer, currsize, maxsize, ' ');
        ++spadlen;
    }
}

/*************************************************************************
// floating format support
**************************************************************************/

#if !(NO_FLOAT)

static LDOUBLE abs_val(LDOUBLE value) {
    LDOUBLE result = value;

    if (value < 0)
        result = -value;

    return result;
}

static LDOUBLE POW10(int exp) {
    LDOUBLE result = 1;

    while (exp) {
        result *= 10;
        exp--;
    }

    return result;
}

static LLONG ROUND(LDOUBLE value) {
    LLONG intpart;

    intpart = (LLONG) value;
    value = value - intpart;
    if (value >= 0.5)
        intpart++;

    return intpart;
}

/* a replacement for modf that doesn't need the math library. Should
   be portable, but slow */
static double my_modf(double x0, double *iptr) {
    int i;
    long l;
    double x = x0;
    double f = 1.0;

    for (i = 0; i < 100; i++) {
        l = (long) x;
        if (l <= (x + 1) && l >= (x - 1))
            break;
        x *= 0.1;
        f *= 10.0;
    }

    if (i == 100) {
        /* yikes! the number is beyond what we can handle. What do we do? */
        *iptr = 0.0;
        return 0;
    }

    if (i != 0) {
        double i2, ret;

        ret = my_modf(x0 - l * f, &i2);
        *iptr = l * f + i2;
        return ret;
    }

    *iptr = l;
    return x - *iptr;
}

static void fmtfp(char *buffer, size_t *currsize, size_t maxsize, LDOUBLE fvalue, int min, int max,
                  int flags) {
/* avoid warnings with 'gcc -Wshadow' */
#undef index
#define index fmtfp_index
    int signvalue = 0;
    double ufvalue;
    char iconvert[311 + 1];
    char fconvert[311 + 1];
    int iplace = 0;
    int fplace = 0;
    int padlen = 0; /* amount to pad */
    int zpadlen = 0;
    const char *digits;
    int index;
    double intpart;
    double fracpart;
    double temp;

    /*
     * AIX manpage says the default is 0, but Solaris says the default
     * is 6, and sprintf on AIX defaults to 6
     */
    if (min < 0)
        min = 0;
    if (max < 0)
        max = 6;

    ufvalue = abs_val(fvalue);

    if (fvalue < 0) {
        signvalue = '-';
    } else {
        if (flags & DP_F_PLUS) { /* Do a sign (+/i) */
            signvalue = '+';
        } else {
            if (flags & DP_F_SPACE)
                signvalue = ' ';
        }
    }

    digits = "0123456789ABCDEF";
#if 0
    digits = (flags & DP_F_UP) ? "0123456789ABCDEF" : "0123456789abcdef";
#endif

#if 0
     if (max == 0) ufvalue += 0.5; /* if max = 0 we must round */
#endif

    /*
     * Sorry, we only support 16 digits past the decimal because of our
     * conversion method
     */
    if (max > 16)
        max = 16;

    /* We "cheat" by converting the fractional part to integer by
     * multiplying by a factor of 10
     */

    temp = ufvalue;
    my_modf(temp, &intpart);

    fracpart = ROUND((POW10(max)) * (ufvalue - intpart));

    if (fracpart >= POW10(max)) {
        intpart++;
        fracpart -= POW10(max);
    }

    /* Convert integer part */
    do {
        temp = intpart;
        my_modf(intpart * 0.1, &intpart);
        temp = temp * 0.1;
        index = (int) ((temp - intpart + 0.05) * 10.0);
        /* index = (int) (((double)(temp*0.1) -intpart +0.05) *10.0); */
        /* printf ("%llf, %f, %x\n", temp, intpart, index); */
        iconvert[iplace] = digits[index];
    } while (++iplace < (int) sizeof(iconvert) - 1 && intpart);
    iconvert[iplace] = 0;

    /* Convert fractional part */
    if (fracpart) {
        do {
            temp = fracpart;
            my_modf(fracpart * 0.1, &fracpart);
            temp = temp * 0.1;
            index = (int) ((temp - fracpart + 0.05) * 10.0);
            /* index = (int) ((((temp/10) -fracpart) +0.05) *10); */
            /* printf ("%lf, %lf, %ld\n", temp, fracpart, index); */
            fconvert[fplace] = digits[index];
        } while (++fplace < (int) sizeof(fconvert) - 1 && fracpart);
    }
    fconvert[fplace] = 0;

    /* -1 for decimal point, another -1 if we are printing a sign */
    padlen = min - iplace - max - 1 - ((signvalue) ? 1 : 0);
    zpadlen = max - fplace;
    if (zpadlen < 0)
        zpadlen = 0;
    if (padlen < 0)
        padlen = 0;
    if (flags & DP_F_MINUS)
        padlen = -padlen; /* Left Justifty */

    if ((flags & DP_F_ZERO) && (padlen > 0)) {
        if (signvalue) {
            dopr_outch(buffer, currsize, maxsize, signvalue);
            --padlen;
            signvalue = 0;
        }
        while (padlen > 0) {
            dopr_outch(buffer, currsize, maxsize, '0');
            --padlen;
        }
    }
    while (padlen > 0) {
        dopr_outch(buffer, currsize, maxsize, ' ');
        --padlen;
    }
    if (signvalue)
        dopr_outch(buffer, currsize, maxsize, signvalue);

    while (iplace > 0)
        dopr_outch(buffer, currsize, maxsize, iconvert[--iplace]);

#ifdef DEBUG_SNPRINTF
    printf("fmtfp: fplace=%d zpadlen=%d\n", fplace, zpadlen);
#endif

    /*
     * Decimal point.  This should probably use locale to find the correct
     * char to print out.
     */
    if (max > 0) {
        dopr_outch(buffer, currsize, maxsize, '.');
        while (fplace > 0)
            dopr_outch(buffer, currsize, maxsize, fconvert[--fplace]);
    }
    while (zpadlen > 0) {
        dopr_outch(buffer, currsize, maxsize, '0');
        --zpadlen;
    }
    while (padlen < 0) {
        dopr_outch(buffer, currsize, maxsize, ' ');
        ++padlen;
    }
#undef index
}

#endif /* !(NO_FLOAT) */

/*************************************************************************
// dopr()
**************************************************************************/

static size_t dopr(char *buffer, size_t maxsize, const char *format, va_list args) {
    char ch;
    LLONG value;
#if !(NO_FLOAT)
    LDOUBLE fvalue;
#endif
    int min;
    int max;
    int state;
    int flags;
    int cflags;
    size_t currsize;

    state = DP_S_DEFAULT;
    currsize = flags = cflags = min = 0;
    max = -1;
    ch = *format++;

    while (state != DP_S_DONE) {
        unsigned base;
        const char *strvalue;
        int strln;

        if (ch == '\0')
            state = DP_S_DONE;

        switch (state) {
        case DP_S_DEFAULT:
            if (ch == '%')
                state = DP_S_FLAGS;
            else
                dopr_outch(buffer, &currsize, maxsize, ch);
            ch = *format++;
            break;
        case DP_S_FLAGS:
            switch (ch) {
            case '-':
                flags |= DP_F_MINUS;
                ch = *format++;
                break;
            case '+':
                flags |= DP_F_PLUS;
                ch = *format++;
                break;
            case ' ':
                flags |= DP_F_SPACE;
                ch = *format++;
                break;
            case '#':
                flags |= DP_F_NUM;
                ch = *format++;
                break;
            case '0':
                flags |= DP_F_ZERO;
                ch = *format++;
                break;
            default:
                state = DP_S_MIN;
                break;
            }
            break;
        case DP_S_MIN:
            if (isdigit((unsigned char) ch)) {
                min = 10 * min + char_to_int(ch);
                ch = *format++;
            } else if (ch == '*') {
                min = va_arg(args, int);
                ch = *format++;
                state = DP_S_DOT;
            } else {
                state = DP_S_DOT;
            }
            assert(min > 0 - UPX_RSIZE_MAX_STR);
            assert(min < UPX_RSIZE_MAX_STR);
            break;
        case DP_S_DOT:
            if (ch == '.') {
                state = DP_S_MAX;
                ch = *format++;
            } else {
                state = DP_S_MOD;
            }
            break;
        case DP_S_MAX:
            if (isdigit((unsigned char) ch)) {
                if (max < 0)
                    max = 0;
                max = 10 * max + char_to_int(ch);
                ch = *format++;
            } else if (ch == '*') {
                max = va_arg(args, int);
                ch = *format++;
                state = DP_S_MOD;
            } else {
                state = DP_S_MOD;
            }
            assert(max > 0 - UPX_RSIZE_MAX_STR);
            assert(max < UPX_RSIZE_MAX_STR);
            break;
        case DP_S_MOD:
            switch (ch) {
            case 'h':
                cflags = DP_C_SHORT;
                ch = *format++;
                if (ch == 'h') {
                    cflags = DP_C_CHAR;
                    ch = *format++;
                }
                break;
            case 'l':
                cflags = DP_C_LONG;
                ch = *format++;
                if (ch == 'l') {
                    cflags = DP_C_LLONG;
                    ch = *format++;
                }
                break;
            case 'j': // intmax_t
                cflags = DP_C_LLONG;
                ch = *format++;
                break;
            case 'z': // size_t
                cflags = sizeof(size_t) == sizeof(LLONG) ? DP_C_LLONG : 0;
                ch = *format++;
                break;
            case 't': // ptrdiff_t
                cflags = sizeof(ptrdiff_t) == sizeof(LLONG) ? DP_C_LLONG : 0;
                ch = *format++;
                break;
            case 'L':
                cflags = DP_C_LDOUBLE;
                ch = *format++;
                break;
            default:
                break;
            }
            state = DP_S_CONV;
            break;
        case DP_S_CONV:
            switch (ch) {
            case 'd':
            case 'i':
                if (cflags == DP_C_CHAR)
                    value = (LLONG)(signed char) va_arg(args, int);
                else if (cflags == DP_C_SHORT)
                    value = (LLONG)(short) va_arg(args, int);
                else if (cflags == DP_C_LONG)
                    value = (LLONG) va_arg(args, long);
                else if (cflags == DP_C_LLONG)
                    value = (LLONG) va_arg(args, LLONG);
                else
                    value = (LLONG) va_arg(args, int);
                fmtint(buffer, &currsize, maxsize, value, 10, min, max, flags);
                break;
            case 'X':
                flags |= DP_F_UP;
            /*fallthrough*/
            case 'x':
                base = 16;
            l_fmtuint:
                flags |= DP_F_UNSIGNED;
                if (cflags == DP_C_CHAR)
                    value = (ULLONG)(unsigned char) va_arg(args, unsigned);
                else if (cflags == DP_C_SHORT)
                    value = (ULLONG)(unsigned short) va_arg(args, unsigned);
                else if (cflags == DP_C_LONG)
                    value = (ULLONG) va_arg(args, unsigned long);
                else if (cflags == DP_C_LLONG)
                    value = (ULLONG) va_arg(args, ULLONG);
                else
                    value = (ULLONG) va_arg(args, unsigned);
                fmtint(buffer, &currsize, maxsize, value, base, min, max, flags);
                break;
            case 'u':
                base = 10;
                goto l_fmtuint;
            case 'o':
                base = 8;
                goto l_fmtuint;
            case 'c':
                // TODO: wint_t
                dopr_outch(buffer, &currsize, maxsize, va_arg(args, int));
                break;
            case 's':
                // TODO: wchar_t
                strvalue = va_arg(args, const char *);
                if (!strvalue)
                    strvalue = "(NULL)";
                strln = (int) strlen(strvalue);
                if (max == -1)
                    max = strln;
                if (min > 0 && max >= 0 && min > max)
                    max = min;
                fmtstr(buffer, &currsize, maxsize, strvalue, strln, flags, min, max);
                break;
            case 'p':
                strvalue = (const char *) va_arg(args, const void *);
                fmtint(buffer, &currsize, maxsize, (LLONG)(upx_uintptr_t) strvalue, 16, min, max,
                       flags);
                break;
            case 'n':
                if (cflags == DP_C_CHAR) {
                    signed char *num;
                    num = va_arg(args, signed char *);
                    assert(num != NULL);
                    *num = (signed char) currsize;
                } else if (cflags == DP_C_SHORT) {
                    short *num;
                    num = va_arg(args, short *);
                    assert(num != NULL);
                    *num = (short) currsize;
                } else if (cflags == DP_C_LONG) {
                    long *num;
                    num = va_arg(args, long *);
                    assert(num != NULL);
                    *num = (long) currsize;
                } else if (cflags == DP_C_LLONG) {
                    LLONG *num;
                    num = va_arg(args, LLONG *);
                    assert(num != NULL);
                    *num = (LLONG) currsize;
                } else {
                    int *num;
                    num = va_arg(args, int *);
                    assert(num != NULL);
                    *num = (int) currsize;
                }
                break;
            case '%':
                dopr_outch(buffer, &currsize, maxsize, ch);
                break;
#if !(NO_FLOAT)
            case 'F':
                flags |= DP_F_UP;
            /*fallthrough*/
            case 'f':
                if (cflags == DP_C_LDOUBLE)
                    fvalue = va_arg(args, LDOUBLE);
                else
                    fvalue = va_arg(args, double);
                /* um, floating point? */
                fmtfp(buffer, &currsize, maxsize, fvalue, min, max, flags);
                break;
            case 'E':
                flags |= DP_F_UP;
            /*fallthrough*/
            case 'e':
                if (cflags == DP_C_LDOUBLE)
                    fvalue = va_arg(args, LDOUBLE);
                else
                    fvalue = va_arg(args, double);
                break;
            case 'G':
                flags |= DP_F_UP;
            /*fallthrough*/
            case 'g':
                if (cflags == DP_C_LDOUBLE)
                    fvalue = va_arg(args, LDOUBLE);
                else
                    fvalue = va_arg(args, double);
                break;
#else
            case 'F':
            case 'f':
            case 'E':
            case 'e':
            case 'G':
            case 'g':
            case 'A':
            case 'a':
                assert(0);
                exit(255);
#endif /* !(NO_FLOAT) */
            default:
                /* Unknown, skip */
                break;
            }
            ch = *format++;
            state = DP_S_DEFAULT;
            flags = cflags = min = 0;
            max = -1;
            break;
        case DP_S_DONE:
            break;
        default:
            /* hmm? */
            break; /* some picky compilers need this */
        }
    }
    dopr_outch(buffer, &currsize, maxsize, '\0');
    return currsize; // returns size, not length
}

/*************************************************************************
// public entries
**************************************************************************/

// UPX version with assertions
int upx_vsnprintf(char *str, upx_rsize_t max_size, const char *format, va_list ap) {
    size_t size;

    // preconditions
    assert(max_size <= UPX_RSIZE_MAX_STR);
    if (str != NULL)
        assert(max_size > 0);
    else
        assert(max_size == 0);

    size = dopr(str, max_size, format, ap);

    // postconditions
    assert(size > 0);
    assert(size <= UPX_RSIZE_MAX_STR);
    if (str != NULL) {
        assert(size <= max_size);
        assert(str[size - 1] == '\0');
    }

    return ACC_ICONV(int, size - 1); // snprintf() returns length, not size
}

int __acc_cdecl_va upx_snprintf(char *str, upx_rsize_t max_size, const char *format, ...) {
    va_list ap;
    int len;

    va_start(ap, format);
    len = upx_vsnprintf(str, max_size, format, ap);
    va_end(ap);
    return len;
}

int upx_vasprintf(char **ptr, const char *format, va_list ap) {
    int len;

    assert(ptr != NULL);
    *ptr = NULL;
#if defined(va_copy)
    va_list ap_copy;
    va_copy(ap_copy, ap);
    len = upx_vsnprintf(NULL, 0, format, ap_copy);
    va_end(ap_copy);
#else
    len = upx_vsnprintf(NULL, 0, format, ap);
#endif
    if (len >= 0) {
        *ptr = (char *) malloc(len + 1);
        assert(*ptr != NULL);
        if (*ptr == NULL)
            return -1;
        int len2 = upx_vsnprintf(*ptr, len + 1, format, ap);
        assert(len2 == len);
    }
    return len;
}

int __acc_cdecl_va upx_asprintf(char **ptr, const char *format, ...) {
    va_list ap;
    int len;

    va_start(ap, format);
    len = upx_vasprintf(ptr, format, ap);
    va_end(ap);
    return len;
}

#undef strlen
upx_rsize_t upx_strlen(const char *s) {
    assert(s != NULL);
    size_t len = strlen(s);
    assert(len < UPX_RSIZE_MAX_STR);
    return len;
}

/*************************************************************************
 //
**************************************************************************/

#if 0 || defined(TEST_SNPRINTF)

#undef sprintf
#include <stdio.h>
#include <string.h>
#include <math.h>

#undef snprintf
#define snprintf upx_snprintf
//int sprintf(char *str,const char *fmt,...);

int main(void)
{
    char buf1[1024];
    char buf2[1024];
    const char *fp_fmt[] = {
        "%1.1f",
        "%-1.5f",
        "%1.5f",
        "%123.9f",
        "%10.5f",
        "% 10.5f",
        "%+22.9f",
        "%+4.9f",
        "%01.3f",
        "%4f",
        "%3.1f",
        "%3.2f",
        "%.0f",
        "%f",
        "-16.16f",
        NULL
    };
    const double fp_nums[] = {
        6442452944.1234, -1.5, 134.21, 91340.2, 341.1234, 0203.9, 0.96, 0.996,
        0.9996, 1.996, 4.136,  0
    };
    const char *int_fmt[] = {
        "%-1.5d",
        "%1.5d",
        "%123.9d",
        "%5.5d",
        "%10.5d",
        "% 10.5d",
        "%+22.33d",
        "%01.3d",
        "%4d",
        "%d",
        NULL
    };
    const long int_nums[] = { -1, 134, 91340, 341, 0203, 0 };
    const char *str_fmt[] = {
        "10.5s",
        "5.10s",
        "10.1s",
        "0.10s",
        "10.0s",
        "1.10s",
        "%s",
        "%.1s",
        "%.10s",
        "%10s",
        0
    };
    const char *str_vals[] = {"hello", "a", "", "a longer string", NULL};
    int x, y;
    int fail = 0;
    int num = 0;

    printf ("Testing snprintf format codes against system sprintf...\n");

    for (x = 0; fp_fmt[x] ; x++) {
        for (y = 0; fp_nums[y] != 0 ; y++) {
            int l1 = snprintf(NULL, 0, fp_fmt[x], fp_nums[y]);
            int l2 = snprintf(buf1, sizeof(buf1), fp_fmt[x], fp_nums[y]);
            sprintf (buf2, fp_fmt[x], fp_nums[y]);
            if (strcmp (buf1, buf2)) {
                printf("snprintf doesn't match Format: %s\n\tsnprintf = [%s]\n\t sprintf = [%s]\n",
                       fp_fmt[x], buf1, buf2);
                fail++;
            }
            if (l1 != l2) {
                printf("snprintf l1 != l2 (%d %d) %s\n", l1, l2, fp_fmt[x]);
                fail++;
            }
            num++;
        }
    }

    for (x = 0; int_fmt[x] ; x++) {
        for (y = 0; int_nums[y] != 0 ; y++) {
            int l1 = snprintf(0, 0, int_fmt[x], int_nums[y]);
            int l2 = snprintf(buf1, sizeof(buf1), int_fmt[x], int_nums[y]);
            sprintf (buf2, int_fmt[x], int_nums[y]);
            if (strcmp (buf1, buf2)) {
                printf("snprintf doesn't match Format: %s\n\tsnprintf = [%s]\n\t sprintf = [%s]\n",
                       int_fmt[x], buf1, buf2);
                fail++;
            }
            if (l1 != l2) {
                printf("snprintf l1 != l2 (%d %d) %s\n", l1, l2, int_fmt[x]);
                fail++;
            }
            num++;
        }
    }

    for (x = 0; str_fmt[x] ; x++) {
        for (y = 0; str_vals[y] != 0 ; y++) {
            int l1 = snprintf(NULL, 0, str_fmt[x], str_vals[y]);
            int l2 = snprintf(buf1, sizeof(buf1), str_fmt[x], str_vals[y]);
            sprintf (buf2, str_fmt[x], str_vals[y]);
            if (strcmp (buf1, buf2)) {
                printf("snprintf doesn't match Format: %s\n\tsnprintf = [%s]\n\t sprintf = [%s]\n",
                       str_fmt[x], buf1, buf2);
                fail++;
            }
            if (l1 != l2) {
                printf("snprintf l1 != l2 (%d %d) %s\n", l1, l2, str_fmt[x]);
                fail++;
            }
            num++;
        }
    }

    printf ("%d tests failed out of %d.\n", fail, num);

    printf("seeing how many digits we support\n");
    {
        double v0 = 0.12345678901234567890123456789012345678901;
        for (x=0; x<100; x++) {
            snprintf(buf1, sizeof(buf1), "%1.1f", v0*pow(10, x));
            sprintf(buf2,                "%1.1f", v0*pow(10, x));
            if (strcmp(buf1, buf2) != 0) {
                printf("we seem to support %d digits\n", x-1);
                break;
            }
        }
    }

    return 0;
}
#endif /* SNPRINTF_TEST */

/* vim:set ts=4 sw=4 et: */
