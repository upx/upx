/* work.cpp -- main driver

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2008 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2008 Laszlo Molnar
   All Rights Reserved.

   UPX and the UCL library are free software; you can redistribute them
   and/or modify them under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.
   If not, write to the Free Software Foundation, Inc.,
   59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

   Markus F.X.J. Oberhumer              Laszlo Molnar
   <markus@oberhumer.com>               <ml1050@users.sourceforge.net>
 */


#include "conf.h"
#include "file.h"
#include "packmast.h"
#include "packer.h"
#include "ui.h"


#if defined(__DJGPP__)
#  define USE_FTIME 1
#elif (ACC_OS_WIN32 && ACC_CC_MWERKS) && defined(__MSL__)
#  include <utime.h>
#  define USE_UTIME 1
#elif ((ACC_OS_WIN32 || ACC_OS_WIN64) && (ACC_CC_INTELC || ACC_CC_MSC))
#  define USE__FUTIME 1
#elif defined(HAVE_UTIME)
#  define USE_UTIME 1
#endif

#if !defined(SH_DENYRW)
#  define SH_DENYRW     (-1)
#endif
#if !defined(SH_DENYWR)
#  define SH_DENYWR     (-1)
#endif

// ignore errors in some cases and silence __attribute__((__warn_unused_result__))
#define IGNORE_ERROR(var)        ACC_UNUSED(var)


/*************************************************************************
// process one file
**************************************************************************/

void do_one_file(const char *iname, char *oname)
{
    int r;
    struct stat st;
    memset(&st, 0, sizeof(st));
#if defined(HAVE_LSTAT)
    r = lstat(iname,&st);
#else
    r = stat(iname,&st);
#endif

    if (r != 0)
        throw FileNotFoundException(iname);
    if (!(S_ISREG(st.st_mode)))
        throwIOException("not a regular file -- skipped");
#if defined(__unix__)
    // no special bits may be set
    if ((st.st_mode & (S_ISUID | S_ISGID | S_ISVTX)) != 0)
        throwIOException("file has special permissions -- skipped");
#endif
    if (st.st_size <= 0)
        throwIOException("empty file -- skipped");
    if (st.st_size >= 1024*1024*1024)
        throwIOException("file is too large -- skipped");
    if ((st.st_mode & S_IWUSR) == 0)
    {
        bool skip = true;
        if (opt->output_name)
            skip = false;
        else if (opt->to_stdout)
            skip = false;
        else if (opt->backup)
            skip = false;
        if (skip)
            throwIOException("file is write protected -- skipped");
    }

    InputFile fi;
    fi.st = st;
    fi.sopen(iname, O_RDONLY | O_BINARY, SH_DENYWR);

#if defined(USE_FTIME)
    struct ftime fi_ftime;
    memset(&fi_ftime, 0, sizeof(fi_ftime));
    if (opt->preserve_timestamp)
    {
        if (getftime(fi.getFd(), &fi_ftime) != 0)
            throwIOException("cannot determine file timestamp");
    }
#endif

    // open output file
    OutputFile fo;
    if (opt->cmd == CMD_COMPRESS || opt->cmd == CMD_DECOMPRESS)
    {
        if (opt->to_stdout)
        {
            if (!fo.openStdout(1, opt->force ? true : false))
                throwIOException("data not written to a terminal; Use '-f' to force.");
        }
        else
        {
            char tname[ACC_FN_PATH_MAX+1];
            if (opt->output_name)
                strcpy(tname,opt->output_name);
            else
            {
                if (!maketempname(tname, sizeof(tname), iname, ".upx"))
                    throwIOException("could not create a temporary file name");
            }
            if (opt->force >= 2)
            {
#if defined(HAVE_CHMOD)
                r = chmod(tname, 0777);
                IGNORE_ERROR(r);
#endif
                r = unlink(tname);
                IGNORE_ERROR(r);
            }
            int flags = O_CREAT | O_WRONLY | O_BINARY;
            if (opt->force)
                flags |= O_TRUNC;
            else
                flags |= O_EXCL;
            int shmode = SH_DENYWR;
#if defined(__MINT__)
            flags |= O_TRUNC;
            shmode = O_DENYRW;
#endif
            // cannot rely on open() because of umask
            //int omode = st.st_mode | 0600;
            int omode = 0600;
            if (!opt->preserve_mode)
                omode = 0666;
            fo.sopen(tname,flags,shmode,omode);
            // open succeeded - now set oname[]
            strcpy(oname,tname);
        }
    }

    // handle command
    PackMaster pm(&fi, opt);
    if (opt->cmd == CMD_COMPRESS)
        pm.pack(&fo);
    else if (opt->cmd == CMD_DECOMPRESS)
        pm.unpack(&fo);
    else if (opt->cmd == CMD_TEST)
        pm.test();
    else if (opt->cmd == CMD_LIST)
        pm.list();
    else if (opt->cmd == CMD_FILEINFO)
        pm.fileInfo();
    else
        throwInternalError("invalid command");

    // copy time stamp
    if (opt->preserve_timestamp && oname[0] && fo.isOpen())
    {
#if defined(USE_FTIME)
        r = setftime(fo.getFd(), &fi_ftime);
        IGNORE_ERROR(r);
#elif defined(USE__FUTIME)
        struct _utimbuf u;
        u.actime = st.st_atime;
        u.modtime = st.st_mtime;
        r = _futime(fo.getFd(), &u);
        IGNORE_ERROR(r);
#endif
    }

    // close files
    fo.closex();
    fi.closex();

    // rename or delete files
    if (oname[0] && !opt->output_name)
    {
        // FIXME: .exe or .cof etc.
        if (!opt->backup)
        {
#if defined(HAVE_CHMOD)
            r = chmod(iname, 0777);
            IGNORE_ERROR(r);
#endif
            File::unlink(iname);
        }
        else
        {
            // make backup
            char bakname[ACC_FN_PATH_MAX+1];
            if (!makebakname(bakname, sizeof(bakname), iname))
                throwIOException("could not create a backup file name");
            File::rename(iname,bakname);
        }
        File::rename(oname,iname);
    }

    // copy file attributes
    if (oname[0])
    {
        oname[0] = 0;
        const char *name = opt->output_name ? opt->output_name : iname;
        UNUSED(name);
#if defined(USE_UTIME)
        // copy time stamp
        if (opt->preserve_timestamp)
        {
            struct utimbuf u;
            u.actime = st.st_atime;
            u.modtime = st.st_mtime;
            r = utime(name, &u);
            IGNORE_ERROR(r);
        }
#endif
#if defined(HAVE_CHMOD)
        // copy permissions
        if (opt->preserve_mode)
        {
            r = chmod(name, st.st_mode);
            IGNORE_ERROR(r);
        }
#endif
#if defined(HAVE_CHOWN)
        // copy the ownership
        if (opt->preserve_ownership)
        {
            r = chown(name, st.st_uid, st.st_gid);
            IGNORE_ERROR(r);
        }
#endif
    }

    UiPacker::uiConfirmUpdate();
}


/*************************************************************************
// process all files from the commandline
**************************************************************************/

static void unlink_ofile(char *oname)
{
    if (oname && oname[0])
    {
#if defined(HAVE_CHMOD)
        int r;
        r = chmod(oname, 0777);
        IGNORE_ERROR(r);
#endif
        if (unlink(oname) == 0)
            oname[0] = 0;
    }
}


void do_files(int i, int argc, char *argv[])
{
    if (opt->verbose >= 1)
    {
        show_head();
        UiPacker::uiHeader();
    }

    for ( ; i < argc; i++)
    {
        infoHeader();

        const char *iname = argv[i];
        char oname[ACC_FN_PATH_MAX+1];
        oname[0] = 0;

        try {
            do_one_file(iname,oname);
        } catch (const Exception &e) {
            unlink_ofile(oname);
            if (opt->verbose >= 1 || (opt->verbose >= 0 && !e.isWarning()))
                printErr(iname,&e);
            set_ec(e.isWarning() ? EXIT_WARN : EXIT_ERROR);
        } catch (const Error &e) {
            unlink_ofile(oname);
            printErr(iname,&e);
            e_exit(EXIT_ERROR);
        } catch (std::bad_alloc *e) {
            unlink_ofile(oname);
            printErr(iname,"out of memory");
            //delete e;
            e_exit(EXIT_ERROR);
        } catch (const std::bad_alloc &) {
            unlink_ofile(oname);
            printErr(iname,"out of memory");
            e_exit(EXIT_ERROR);
        } catch (std::exception *e) {
            unlink_ofile(oname);
            printUnhandledException(iname,e);
            //delete e;
            e_exit(EXIT_ERROR);
        } catch (const std::exception &e) {
            unlink_ofile(oname);
            printUnhandledException(iname,&e);
            e_exit(EXIT_ERROR);
        } catch (...) {
            unlink_ofile(oname);
            printUnhandledException(iname,NULL);
            e_exit(EXIT_ERROR);
        }
    }

    if (opt->cmd == CMD_COMPRESS)
        UiPacker::uiPackTotal();
    else if (opt->cmd == CMD_DECOMPRESS)
        UiPacker::uiUnpackTotal();
    else if (opt->cmd == CMD_LIST)
        UiPacker::uiListTotal();
    else if (opt->cmd == CMD_TEST)
        UiPacker::uiTestTotal();
    else if (opt->cmd == CMD_FILEINFO)
        UiPacker::uiFileInfoTotal();
}


/*
vi:ts=4:et
*/

