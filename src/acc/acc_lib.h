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


#ifndef __ACC_LIB_H_INCLUDED
#define __ACC_LIB_H_INCLUDED


/*************************************************************************
// wrap <dirent.h>
**************************************************************************/

struct ACC_DIR { void* dirp; };     /* opaque type */
typedef struct ACC_DIR ACC_DIR;
struct acc_dirent
{
    char d_name[255+1];
};

int acc_opendir(ACC_DIR* dirp, const char* name);
int acc_readdir(ACC_DIR* dirp, struct acc_dirent* d);
int acc_closedir(ACC_DIR* dirp);


/*************************************************************************
// wrap <fnmatch.h>
**************************************************************************/


/*************************************************************************
// wrap <getopt.h>
**************************************************************************/


/*************************************************************************
// wrap <string.h>
**************************************************************************/


/*************************************************************************
// wrap misc
**************************************************************************/

#if defined(__MINGW32__)
#  define acc_alloca(x)     __builtin_alloca((x))
#elif (HAVE_ALLOCA)
#  define acc_alloca(x)     alloca((x))
#endif

long acc_get_osfhandle(int fd);
int acc_isatty(int fd);
int acc_mkdir(const char* name, unsigned mode);
int acc_response(int* argc, char*** argv);
int acc_set_binmode(int fd, int binary);

#endif /* already included */


/*
vi:ts=4:et
*/
