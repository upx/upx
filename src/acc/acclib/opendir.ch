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


#define __ACCLIB_OPENDIR_CH_INCLUDED 1
#if !defined(ACCLIB_PUBLIC)
#  define ACCLIB_PUBLIC(r,f)    r __ACCLIB_FUNCNAME(f)
#endif


/*************************************************************************
// wrap <dirent.h>
**************************************************************************/

#if !defined(__ACCLIB_USE_OPENDIR)
#if (ACC_OS_DOS16 || ACC_OS_DOS32 || ACC_OS_OS2 || ACC_OS_OS216 || ACC_OS_TOS || ACC_OS_WIN16 || ACC_OS_WIN32 || ACC_OS_WIN64)

static int __ACCLIB_FUNCNAME(acc_opendir_init)(acc_dir_p f, const char* path, char* buf, size_t bufsize)
{
    size_t l; char* p;
    f->f_name[0] = 0; buf[0] = 0;
    l = strlen(path);
    if (l == 0 || bufsize <= 4 || l >= bufsize - 4)
        return -1;
    strcpy(buf, path); p = buf + l;
    if (p[-1] == ':' || p[-1] == '\\' || p[-1] == '/')
        strcpy(p, "*.*");
    else
        strcpy(p, "\\*.*");
    return 0;
}

#endif
#endif


#if defined(__ACCLIB_USE_OPENDIR)

ACCLIB_PUBLIC(int, acc_opendir) (acc_dir_p f, const char* path)
{
    f->u_dirp = opendir(path);
    if (!f->u_dirp)
        return -2;
    return __ACCLIB_FUNCNAME(acc_readdir)(f);
}

ACCLIB_PUBLIC(int, acc_readdir) (acc_dir_p f)
{
    const struct dirent* dp;
    f->f_name[0] = 0;
    if (!f->u_dirp)
        return -1;
    dp = (const struct dirent*) readdir((DIR*) f->u_dirp);
    if (!dp)
        return -1;
    if (!dp->d_name[0] || strlen(dp->d_name) >= sizeof(f->f_name))
        return -1;
    strcpy(f->f_name, dp->d_name);
#if (ACC_CC_WATCOMC)
    ACC_COMPILE_TIME_ASSERT(sizeof(f->f_name) >= sizeof(dp->d_name))
    f->f_time = dp->d_time;
    f->f_date = dp->d_date;
    f->f_size = dp->d_size;
#endif
    return 0;
}

ACCLIB_PUBLIC(int, acc_closedir) (acc_dir_p f)
{
    int r = -1;
    if (f->u_dirp)
        r = closedir((DIR*) f->u_dirp);
    f->u_dirp = 0;
    return r;
}


#elif (ACC_OS_WIN32 || ACC_OS_WIN64)

ACCLIB_PUBLIC(int, acc_opendir) (acc_dir_p f, const char* path)
{
    WIN32_FIND_DATAA d;
    HANDLE h;
    if (__ACCLIB_FUNCNAME(acc_opendir_init)(f, path, f->f_name, sizeof(f->f_name)) != 0)
        return -1;
    h = FindFirstFileA(f->f_name, &d);
    f->f_name[0] = 0;
    if ((f->u_handle = (acclib_handle_t) h) == -1)
        return -1;
    if (!d.cFileName[0] || strlen(d.cFileName) >= sizeof(f->f_name))
        return -1;
    strcpy(f->f_name, d.cFileName);
    f->f_attr = d.dwFileAttributes;
    f->f_size_high = d.nFileSizeHigh;
    f->f_size_low = d.nFileSizeLow;
    return 0;
}

ACCLIB_PUBLIC(int, acc_readdir) (acc_dir_p f)
{
    WIN32_FIND_DATAA d;
    f->f_name[0] = 0;
    if (f->u_handle == -1 || FindNextFileA((HANDLE) f->u_handle, &d) == 0)
        return -1;
    if (!d.cFileName[0] || strlen(d.cFileName) >= sizeof(f->f_name))
        return -1;
    strcpy(f->f_name, d.cFileName);
    f->f_attr = d.dwFileAttributes;
    f->f_size_high = d.nFileSizeHigh;
    f->f_size_low = d.nFileSizeLow;
    return 0;
}

ACCLIB_PUBLIC(int, acc_closedir) (acc_dir_p f)
{
    int r = -1;
    if (f->u_handle != -1)
        r = FindClose((HANDLE) f->u_handle);
    f->u_handle = -1;
    return r;
}


#elif (ACC_OS_DOS16 || ACC_OS_DOS32 || ACC_OS_WIN16)

ACCLIB_PUBLIC(int, acc_opendir) (acc_dir_p f, const char* path)
{
    char tmp[ACC_FN_PATH_MAX+1];
    int r;
    f->u_dirp = 0;
    if (__ACCLIB_FUNCNAME(acc_opendir_init)(f, path, tmp, sizeof(tmp)) != 0)
        return -1;
#if (ACC_CC_AZTECC || ACC_CC_PACIFICC)
    r = -1;
#elif (ACC_CC_BORLANDC || ACC_CC_TURBOC)
    r = findfirst(tmp, (struct ffblk*) f->u_dta, FA_HIDDEN|FA_SYSTEM|FA_RDONLY|FA_DIREC);
#else
    r = _dos_findfirst(tmp, _A_HIDDEN|_A_SYSTEM|_A_RDONLY|_A_SUBDIR, (struct find_t*) f->u_dta);
#endif
    if (r != 0) f->f_name[0] = 0;
    if (!f->f_name[0]) return -1;
    f->u_dirp = 1;
    return 0;
}

ACCLIB_PUBLIC(int, acc_readdir) (acc_dir_p f)
{
    int r;
    f->f_name[0] = 0;
    if (!f->u_dirp)
        return -1;
#if (ACC_CC_AZTECC || ACC_CC_PACIFICC)
    r = -1;
#elif (ACC_CC_BORLANDC || ACC_CC_TURBOC)
    r = findnext((struct ffblk*) f->u_dta);
#else
    r = _dos_findnext((struct find_t*) f->u_dta);
#endif
    if (r != 0) f->f_name[0] = 0;
    if (!f->f_name[0]) return -1;
    return 0;
}

ACCLIB_PUBLIC(int, acc_closedir) (acc_dir_p f)
{
    ACC_COMPILE_TIME_ASSERT(sizeof(*f) == 44);
    f->f_name[0] = 0;
    f->u_dirp = 0;
    return 0;
}


#elif (ACC_OS_TOS)

ACCLIB_PUBLIC(int, acc_opendir) (acc_dir_p f, const char* path)
{
    char tmp[ACC_FN_PATH_MAX+1];
    int r;
    DTA* olddta;
    f->u_dirp = 0;
    if (__ACCLIB_FUNCNAME(acc_opendir_init)(f, path, tmp, sizeof(tmp)) != 0)
        return -1;
    olddta = Fgetdta();
    Fsetdta((DTA*) f->u_dta);
    r = Fsfirst(tmp, FA_HIDDEN|FA_SYSTEM|FA_READONLY|FA_SUBDIR);
    Fsetdta(olddta);
    if (r != 0) f->f_name[0] = 0;
    if (!f->f_name[0]) return -1;
    f->u_dirp = 1;
    return 0;
}

ACCLIB_PUBLIC(int, acc_readdir) (acc_dir_p f)
{
    int r;
    DTA* olddta;
    f->f_name[0] = 0;
    if (!f->u_dirp)
        return -1;
    olddta = Fgetdta();
    Fsetdta((DTA*) f->u_dta);
    r = Fsnext();
    Fsetdta(olddta);
    if (r != 0) f->f_name[0] = 0;
    if (!f->f_name[0]) return -1;
    return 0;
}

ACCLIB_PUBLIC(int, acc_closedir) (acc_dir_p f)
{
    ACC_COMPILE_TIME_ASSERT(sizeof(*f) == 44);
    f->f_name[0] = 0;
    f->u_dirp = 0;
    return 0;
}


#else

ACCLIB_PUBLIC(int, acc_opendir) (acc_dir_p f, const char* path)
{
    ACC_UNUSED(path);
    f->f_name[0] = 0;
    return -3;
}

ACCLIB_PUBLIC(int, acc_readdir) (acc_dir_p f)
{
    f->f_name[0] = 0;
    return -1;
}

ACCLIB_PUBLIC(int, acc_closedir) (acc_dir_p f)
{
    f->u_dirp = 0;
    return -1;
}

#endif


/*
vi:ts=4:et
*/
