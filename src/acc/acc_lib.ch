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



#if !defined(ACC_VERSION)
#include "acc.h"
#endif
#ifndef __ACC_INCE_H_INCLUDED
#include ACC_CONFIG_INCLUDE("acc_ince.h")
#endif
#ifndef __ACC_INCI_H_INCLUDED
#include ACC_CONFIG_INCLUDE("acc_inci.h")
#endif


/*************************************************************************
// wrap <dirent.h>
**************************************************************************/

#if (HAVE_DIRENT_H)

int acc_opendir(ACC_DIR* dirp, const char* name)
{
    dirp->dirp = (ACC_DIR*) opendir(name);
    return dirp->dirp ? 0 : -1;
}

int acc_closedir(ACC_DIR* dirp)
{
    return closedir((DIR*) dirp->dirp);
}

int acc_readdir(ACC_DIR* dirp, struct acc_dirent* d)
{
    const struct dirent *dp;
    dp = readdir((DIR*) dirp->dirp);
    if (!dp || !dp->d_name[0])
        return -1;
    if (strlen(dp->d_name) >= sizeof(d->d_name))
        return -1;
    strcpy(d->d_name, dp->d_name);
    return 0;
}

#else

int acc_opendir(ACC_DIR* dirp, const char* name)
{
    ACC_UNUSED(dirp); ACC_UNUSED(name);
    return -1;
}

int acc_closedir(ACC_DIR* dirp)
{
    ACC_UNUSED(dirp);
    return -1;
}

int acc_readdir(ACC_DIR* dirp, struct acc_dirent* d)
{
    ACC_UNUSED(dirp); ACC_UNUSED(d);
    return -1;
}

#endif


/*************************************************************************
// wrap <fnmatch.h>
**************************************************************************/


/*************************************************************************
// wrap <getopt.h>
**************************************************************************/


/*************************************************************************
// wrap misc
**************************************************************************/

long acc_get_osfhandle(int fd)
{
    if (fd < 0)
        return -1;
#if (ACC_OS_CYGWIN)
    return get_osfhandle(fd);
#elif (ACC_OS_EMX && defined(__RSXNT__))
    /* FIXME */
    return -1;
#elif (ACC_OS_WIN32 || ACC_OS_WIN64)
    return _get_osfhandle(fd);
#else
    return fd;
#endif
}


int acc_set_binmode(int fd, int binary)
{
#if (ACC_OS_TOS && defined(__MINT__))
    int old_binary;
    FILE* f;
    if (fd == STDIN_FILENO) f = stdin;
    else if (fd == STDOUT_FILENO) f = stdout;
    else if (fd == STDERR_FILENO) f = stderr;
    else return -1;
    old_binary = f->__mode.__binary;
    __set_binmode(f, binary ? 1 : 0);
    return old_binary ? 1 : 0;
#elif (ACC_OS_TOS)
    ACC_UNUSED(fd); ACC_UNUSED(binary);
    return -1;
#elif (ACC_OS_DOS16 && (ACC_CC_AZTEC_C || ACC_CC_PACIFIC))
    ACC_UNUSED(fd); ACC_UNUSED(binary);
    return -1;
#elif defined(__DJGPP__)
    int r;
    unsigned old_flags = __djgpp_hwint_flags;
    ACC_COMPILE_TIME_ASSERT(O_BINARY > 0)
    ACC_COMPILE_TIME_ASSERT(O_TEXT > 0)
    if (fd < 0)
        return -1;
    r = setmode(fd, binary ? O_BINARY : O_TEXT);
    if ((old_flags & 1) != (__djgpp_hwint_flags & 1))
        __djgpp_set_ctrl_c(!(old_flags & 1));
    if (r == -1)
        return -1;
    return r & O_BINARY ? 1 : 0;
#elif (ACC_OS_DOS16 || ACC_OS_DOS32 || ACC_OS_OS2 || ACC_OS_OS216 || ACC_OS_WIN16 || ACC_OS_WIN32 || ACC_OS_WIN64 || ACC_OS_CYGWIN || ACC_OS_EMX)
    int r;
    ACC_COMPILE_TIME_ASSERT(O_BINARY > 0)
    ACC_COMPILE_TIME_ASSERT(O_TEXT > 0)
    if (fd < 0)
        return -1;
    r = setmode(fd, binary ? O_BINARY : O_TEXT);
    if (r == -1)
        return -1;
    return r & O_BINARY ? 1 : 0;
#else
    if (fd < 0)
        return -1;
    ACC_UNUSED(binary);
    return 1;
#endif
}


int acc_isatty(int fd)
{
    if (fd < 0)
        return 0;
#if (ACC_H_WINDOWS_H)
    /* work around naive library implementations that think that
     * any character device like `nul' is a tty */
    {
        long h = acc_get_osfhandle(fd);
        if (h != -1)
        {
            DWORD d = 0;
            int r = GetConsoleMode((HANDLE)h, &d);
            /* fprintf(stderr, "GetConsoleMode %d 0x%lx\n", r, (long) d); */
            if (!r)
                return 0;   /* GetConsoleMode failed -> not a tty */
        }
    }
#endif
    return isatty(fd);
}


int acc_mkdir(const char* name, unsigned mode)
{
    ACC_UNUSED(mode);
#if (ACC_OS_POSIX || ACC_OS_CYGWIN || ACC_OS_EMX)
    return mkdir(name, mode);
#elif defined(__DJGPP__) || defined(__MINT__)
    return mkdir(name, mode);
#elif (ACC_OS_TOS && (ACC_CC_PUREC || ACC_CC_TURBOC))
    return Dcreate(name);
#elif (ACC_OS_DOS16 && ACC_CC_PACIFIC)
    return mkdir((char*)name);
#else
    return mkdir(name);
#endif
}


#if 0
int acc_response(int* argc, char*** argv)
{
}
#endif


/*
vi:ts=4:et
*/
