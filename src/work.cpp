/* work.cpp -- main driver

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2000 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2000 Laszlo Molnar

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

   Markus F.X.J. Oberhumer                   Laszlo Molnar
   markus.oberhumer@jk.uni-linz.ac.at        ml1050@cdata.tvnet.hu
 */


#include "conf.h"
#include "file.h"
#include "packmast.h"
#include "packer.h"
#include "ui.h"


#if defined(__DJGPP__)
#  define USE_FTIME
#elif defined(__MFX_WIN32) && defined(_MSC_VER)
#  define USE__FUTIME
#elif defined(HAVE_UTIME)
#  define USE_UTIME
#endif

#if !defined(SH_DENYRW)
#  define SH_DENYRW     (-1)
#endif
#if !defined(SH_DENYWR)
#  define SH_DENYWR     (-1)
#endif


/*************************************************************************
// process one file
**************************************************************************/

void do_one_file(const char *iname, char *oname)
{
    struct stat st;
    memset(&st, 0, sizeof(st));
#if defined(HAVE_LSTAT)
    int r = lstat(iname,&st);
#else
    int r = stat(iname,&st);
#endif
    bool need_chmod = true;

    if (r == -1)
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

    InputFile fi;
    fi.st = st;
    fi.sopen(iname, O_RDONLY | O_BINARY, SH_DENYWR);

#if defined(USE_FTIME)
    struct ftime fit;
    getftime(fi.getFd(),&fit);
#endif

    // open output file
    OutputFile fo;
    if (opt->cmd == CMD_COMPRESS || opt->cmd == CMD_DECOMPRESS)
    {
        if (opt->to_stdout)
        {
            if (!fo.openStdout(O_BINARY, opt->force ? true : false))
                throwIOException("data not written to a terminal; Use `-f' to force.");
        }
        else
        {
            char tname[PATH_MAX+1];
            if (opt->output_name)
                strcpy(tname,opt->output_name);
            else
                maketempname(tname,iname,".upx",1);
            if (opt->force >= 2)
            {
#if defined(HAVE_CHMOD)
                (void) ::chmod(tname, 0777);
#endif
                (void) ::unlink(tname);
            }
            int flags = O_CREAT | O_WRONLY | O_BINARY;
            if (opt->force)
                flags |= O_TRUNC;
            else
                flags |= O_EXCL;
            int shmode = SH_DENYWR;
#if defined(__MFX_DOS) || defined(__MFX_WIN32)
            // we can avoid the chmod() call below
            int omode = st.st_mode;
            fo.sopen(tname,flags,shmode,omode);
            need_chmod = false;
#else
            // cannot rely on open() because of umask
            //int omode = st.st_mode | 0600;
            int omode = 0600;
            fo.sopen(tname,flags,shmode,omode);
#endif
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
    if (oname[0] && fo.isOpen())
    {
#if defined(USE_FTIME)
        setftime(fo.getFd(),&fit);
#elif defined(USE__FUTIME)
        struct _utimbuf u;
        u.actime = st.st_atime;
        u.modtime = st.st_mtime;
        (void) _futime(fo.getFd(),&u);
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
            (void) ::chmod(iname, 0777);
#endif
            File::unlink(iname);
        }
        else
        {
            // make backup
            char bakname[PATH_MAX+1];
            makebakname(bakname,iname);
            if (file_exists(bakname))
                maketempname(bakname,iname,".000",1);
            File::rename(iname,bakname);
        }
        File::rename(oname,iname);
    }

    // copy file attributes
    if (oname[0])
    {
        oname[0] = 0;
        const char *name = opt->output_name ? opt->output_name : iname;
#if defined(USE_UTIME)
        // copy time stamp
        struct utimbuf u;
        u.actime = st.st_atime;
        u.modtime = st.st_mtime;
        (void) ::utime(name,&u);
#endif
#if defined(HAVE_CHMOD)
        // copy permissions
        if (need_chmod)
            (void) ::chmod(name, st.st_mode);
#endif
#if defined(HAVE_CHOWN)
        // copy the ownership
        (void) ::chown(name, st.st_uid, st.st_gid);
#endif
    }

    UiPacker::uiConfirmUpdate();
}


/*************************************************************************
// process all files from the commandline
**************************************************************************/

#if !defined(WITH_GUI)

void do_files(int i, int argc, char *argv[])
{
    if (opt->verbose >= 1)
    {
        show_head();
        UiPacker::uiHeader();
    }

    for ( ; i < argc; i++)
    {
#if defined(WITH_MSS)
        //MSS_ENABLE_LOG_OUTPUT;
        //MSS_ENTER_SCOPE;
#endif
        infoHeader();

        const char *iname = argv[i];
        char oname[PATH_MAX+1];
        oname[0] = 0;

        try {
            do_one_file(iname,oname);
        } catch (const Exception &e) {
            if (opt->verbose >= 2 || (opt->verbose >= 1 && !e.isWarning()))
                printErr(iname,&e);
            if (oname[0])
                (void) ::unlink(oname);
        } catch (const Error &e) {
            printErr(iname,&e);
            if (oname[0])
                (void) ::unlink(oname);
            e_exit(EXIT_ERROR);
            //throw;
        } catch (const exception &e) {
            printUnhandledException(iname,&e);
            if (oname[0])
                (void) ::unlink(oname);
            e_exit(EXIT_ERROR);
            //throw;
        } catch (const exception *e) {
            printUnhandledException(iname,e);
            if (oname[0])
                (void) ::unlink(oname);
            e_exit(EXIT_ERROR);
            //throw;
        } catch (...) {
            printUnhandledException(iname,NULL);
            if (oname[0])
                (void) ::unlink(oname);
            e_exit(EXIT_ERROR);
            //throw;
        }

#if defined(WITH_MSS)
        //MSS_LEAVE_SCOPE;
        MSS_CHECK_ALL_BLOCKS;
#endif
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

#endif /* !defined(WITH_GUI) */


/*
vi:ts=4:et
*/

