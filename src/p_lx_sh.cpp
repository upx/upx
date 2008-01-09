/* p_lx_sh.cpp --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2008 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2008 Laszlo Molnar
   Copyright (C) 2000-2008 John F. Reiser
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

   John F. Reiser
   <jreiser@users.sourceforge.net>
 */


#include "conf.h"

#include "file.h"
#include "filter.h"
#include "packer.h"
#include "p_elf.h"
#include "p_unix.h"
#include "p_lx_exc.h"
#include "p_lx_sh.h"

#define PT_LOAD     Elf_LE32_Phdr::PT_LOAD


/*************************************************************************
//
**************************************************************************/

static const
#include "stub/i386-linux.elf.shell-entry.h"
static const
#include "stub/i386-linux.elf.shell-fold.h"


PackLinuxI386sh::PackLinuxI386sh(InputFile *f) :
    super(f), o_shname(0), l_shname(0)
{
}

PackLinuxI386sh::~PackLinuxI386sh()
{
}

static unsigned
umax(unsigned a, unsigned b)
{
    if (a <= b) {
        return b;
    }
    return a;
}

void
PackLinuxI386sh::buildLoader(Filter const *ft)
{
    unsigned const sz_fold = sizeof(stub_i386_linux_elf_shell_fold);
    MemBuffer buf(sz_fold);
    memcpy(buf, stub_i386_linux_elf_shell_fold, sz_fold);

    checkPatch(NULL, 0, 0, 0);  // reset
    patch_le32(buf,sz_fold,"UPX3",l_shname);
    patch_le32(buf,sz_fold,"UPX2",o_shname);

    // get fresh filter
    Filter fold_ft = *ft;
    fold_ft.init(ft->id, ft->addvalue);
    int preferred_ctos[2] = { ft->cto, -1 };
    fold_ft.preferred_ctos = preferred_ctos;

    // filter
    optimizeFilter(&fold_ft, buf, sz_fold);
    unsigned fold_hdrlen = sizeof(l_info) + sizeof(Elf32_Ehdr) +
        sizeof(Elf32_Phdr) * get_te32(&((Elf32_Ehdr const *)(void *)buf)->e_phnum);
    if (0 == get_le32(fold_hdrlen + buf)) {
        // inconsistent SIZEOF_HEADERS in *.lds (ld, binutils)
        fold_hdrlen = umax(0x80, fold_hdrlen);
    }
    bool success = fold_ft.filter(buf + fold_hdrlen, sz_fold - fold_hdrlen);
    UNUSED(success);

    buildLinuxLoader(
        stub_i386_linux_elf_shell_entry, sizeof(stub_i386_linux_elf_shell_entry),
        buf, sz_fold, ft );
}

void PackLinuxI386sh::patchLoader() { }


bool PackLinuxI386sh::getShellName(char *buf)
{
    exetype = -1;
    l_shname = (int) strcspn(buf, " \t\n\v\f\r");
    buf[l_shname] = 0;
    static char const *const shname[] = { // known shells that accept "-c" arg
        "ash", "bash", "bsh", "csh", "ksh", "pdksh", "sh", "tcsh", "zsh",
        "python",
        NULL
    };
    const char *bname = strrchr(buf, '/');
    if (bname == NULL)
        return false;
    for (int j = 0; NULL != shname[j]; ++j) {
        if (0 == strcmp(shname[j], bname + 1)) {
            bool const s = super::canPack();
            if (s) {
                opt->o_unix.blocksize = blocksize = file_size;
            }
            return s;
        }
    }
    return false;
}


bool PackLinuxI386sh::canPack()
{
#if defined(__linux__)  //{
    // only compress i386sh scripts when running under Linux
    char buf[512];

    fi->readx(buf, sizeof(buf));
    fi->seek(0, SEEK_SET);
    buf[sizeof(buf) - 1] = 0;
    if (!memcmp(buf, "#!/", 3)) {                       // #!/bin/sh
        o_shname = 2;
        return getShellName(&buf[o_shname]);
    }
    else if (!memcmp(buf, "#! /", 4)) {                 // #! /bin/sh
        o_shname = 3;
        return getShellName(&buf[o_shname]);
    }
#endif  //}
    return false;
}


void
PackLinuxI386sh::pack1(OutputFile *fo, Filter &)
{
    generateElfHdr(fo, stub_i386_linux_elf_shell_fold, 0x08048000);
}

void
PackLinuxI386sh::pack3(OutputFile *fo, Filter &ft)
{
    super::pack3(fo,ft);
    elfout.phdr[0].p_filesz = fo->getBytesWritten();
}

/*
vi:ts=4:et
*/

