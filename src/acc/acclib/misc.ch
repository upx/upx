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


#define __ACCLIB_MISC_CH_INCLUDED 1
#if !defined(ACCLIB_PUBLIC)
#  define ACCLIB_PUBLIC(r,f)    r __ACCLIB_FUNCNAME(f)
#endif


/*************************************************************************
// wrap misc
**************************************************************************/

ACCLIB_PUBLIC(acclib_handle_t, acc_get_osfhandle) (int fd)
{
    if (fd < 0)
        return -1;
#if (ACC_OS_CYGWIN)
    return get_osfhandle(fd);
#elif (ACC_OS_EMX && defined(__RSXNT__))
    return -1; /* FIXME */
#elif (ACC_OS_WIN32 && ACC_CC_GNUC) && defined(__PW32__)
    return -1; /* FIXME */
#elif (ACC_OS_WIN32 || ACC_OS_WIN64)
# if (ACC_CC_PELLESC && (__POCC__ < 280))
    return -1; /* FIXME */
# elif (ACC_CC_WATCOMC && (__WATCOMC__ < 1000))
    return -1; /* FIXME */
# elif (ACC_CC_WATCOMC && (__WATCOMC__ < 1100))
    return _os_handle(fd);
# else
    return _get_osfhandle(fd);
# endif
#else
    return fd;
#endif
}


ACCLIB_PUBLIC(int, acc_set_binmode) (int fd, int binary)
{
#if (ACC_OS_TOS && defined(__MINT__))
    FILE* fp; int old_binary;
    if (fd == STDIN_FILENO) fp = stdin;
    else if (fd == STDOUT_FILENO) fp = stdout;
    else if (fd == STDERR_FILENO) fp = stderr;
    else return -1;
    old_binary = fp->__mode.__binary;
    __set_binmode(fp, binary ? 1 : 0);
    return old_binary ? 1 : 0;
#elif (ACC_OS_TOS)
    ACC_UNUSED(fd); ACC_UNUSED(binary);
    return -1; /* FIXME */
#elif (ACC_OS_DOS16 && (ACC_CC_AZTECC || ACC_CC_PACIFICC))
    ACC_UNUSED(fd); ACC_UNUSED(binary);
    return -1; /* FIXME */
#elif (ACC_OS_DOS32 && ACC_CC_GNUC) && defined(__DJGPP__)
    int r; unsigned old_flags = __djgpp_hwint_flags;
    ACC_COMPILE_TIME_ASSERT(O_BINARY > 0)
    ACC_COMPILE_TIME_ASSERT(O_TEXT > 0)
    if (fd < 0) return -1;
    r = setmode(fd, binary ? O_BINARY : O_TEXT);
    if ((old_flags & 1u) != (__djgpp_hwint_flags & 1u))
        __djgpp_set_ctrl_c(!(old_flags & 1));
    if (r == -1) return -1;
    return (r & O_TEXT) ? 0 : 1;
#elif (ACC_OS_WIN32 && ACC_CC_GNUC) && defined(__PW32__)
    if (fd < 0) return -1;
    ACC_UNUSED(binary);
    return 1;
#elif (ACC_OS_DOS32 && ACC_CC_HIGHC)
    FILE* fp; int r;
    if (fd == fileno(stdin)) fp = stdin;
    else if (fd == fileno(stdout)) fp = stdout;
    else if (fd == fileno(stderr)) fp = stderr;
    else return -1;
    r = _setmode(fp, binary ? _BINARY : _TEXT);
    if (r == -1) return -1;
    return (r & _BINARY) ? 1 : 0;
#elif (ACC_OS_WIN32 && ACC_CC_MWERKS) && defined(__MSL__)
    ACC_UNUSED(fd); ACC_UNUSED(binary);
    return -1; /* FIXME */
#elif (ACC_OS_CYGWIN && (ACC_CC_GNUC < 0x025a00ul))
    ACC_UNUSED(fd); ACC_UNUSED(binary);
    return -1; /* FIXME */
#elif (ACC_OS_CYGWIN || ACC_OS_DOS16 || ACC_OS_DOS32 || ACC_OS_EMX || ACC_OS_OS2 || ACC_OS_OS216 || ACC_OS_WIN16 || ACC_OS_WIN32 || ACC_OS_WIN64)
    int r;
#if !defined(ACC_CC_ZORTECHC)
    ACC_COMPILE_TIME_ASSERT(O_BINARY > 0)
#endif
    ACC_COMPILE_TIME_ASSERT(O_TEXT > 0)
    if (fd < 0) return -1;
    r = setmode(fd, binary ? O_BINARY : O_TEXT);
    if (r == -1) return -1;
    return (r & O_TEXT) ? 0 : 1;
#else
    if (fd < 0) return -1;
    ACC_UNUSED(binary);
    return 1;
#endif
}


ACCLIB_PUBLIC(int, acc_isatty) (int fd)
{
    /* work around library implementations that think that
     * any character device like `nul' is a tty */
    if (fd < 0)
        return 0;
#if (ACC_OS_DOS16 && !defined(ACC_CC_AZTECC))
    {
        union REGS ri, ro;
        ri.x.ax = 0x4400; ri.x.bx = fd;
        int86(0x21, &ri, &ro);
        if ((ro.x.cflag & 1) == 0)  /* if carry flag not set */
            if ((ro.x.ax & 0x83) != 0x83)
                return 0;
    }
#elif (ACC_OS_DOS32 && ACC_CC_WATCOMC)
    {
        union REGS ri, ro;
        ri.w.ax = 0x4400; ri.w.bx = (unsigned short) fd;
        int386(0x21, &ri, &ro);
        if ((ro.w.cflag & 1) == 0)  /* if carry flag not set */
            if ((ro.w.ax & 0x83) != 0x83)
                return 0;
    }
#elif (ACC_HAVE_WINDOWS_H)
    {
        acclib_handle_t h = __ACCLIB_FUNCNAME(acc_get_osfhandle)(fd);
        if ((HANDLE)h != INVALID_HANDLE_VALUE)
        {
            DWORD d = 0;
            if (GetConsoleMode((HANDLE)h, &d) == 0)
                return 0;   /* GetConsoleMode failed -> not a tty */
        }
    }
#endif
    return (isatty(fd)) ? 1 : 0;
}


ACCLIB_PUBLIC(int, acc_mkdir) (const char* name, unsigned mode)
{
#if (ACC_OS_TOS && (ACC_CC_PUREC || ACC_CC_TURBOC))
    ACC_UNUSED(mode);
    return Dcreate(name);
#elif (ACC_OS_DOS32 && ACC_CC_GNUC) && defined(__DJGPP__)
    return mkdir(name, mode);
#elif (ACC_OS_WIN32 && ACC_CC_GNUC) && defined(__PW32__)
    return mkdir(name, mode);
#elif (ACC_OS_DOS16 || ACC_OS_DOS32 || ACC_OS_OS2 || ACC_OS_OS216 || ACC_OS_WIN16 || ACC_OS_WIN32 || ACC_OS_WIN64)
    ACC_UNUSED(mode);
# if (ACC_CC_HIGHC || ACC_CC_PACIFICC)
    return mkdir((char*) name);
# else
    return mkdir(name);
# endif
#else
    return mkdir(name, mode);
#endif
}


ACCLIB_PUBLIC(acc_int32l_t, acc_muldiv32) (acc_int32l_t a, acc_int32l_t b, acc_int32l_t x)
{
    acc_int32l_t r = 0;
    if (x == 0) return x;
    /* FIXME */
    ACC_UNUSED(a); ACC_UNUSED(b);
    return r;
}


ACCLIB_PUBLIC(acc_uint32l_t, acc_umuldiv32) (acc_uint32l_t a, acc_uint32l_t b, acc_uint32l_t x)
{
    acc_uint32l_t r = 0;
    if (x == 0) return x;
    /* FIXME */
    ACC_UNUSED(a); ACC_UNUSED(b);
    return r;
}


/*
vi:ts=4:et
*/
