/* packmast.cpp --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2004 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2004 Laszlo Molnar
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

   Markus F.X.J. Oberhumer   Laszlo Molnar
   markus@oberhumer.com      ml1050@users.sourceforge.net
 */


#include "conf.h"
#include "file.h"
#include "packmast.h"
#include "packer.h"
#include "lefile.h"
#include "p_elf.h"

#include "p_com.h"
#include "p_djgpp2.h"
#include "p_exe.h"
#include "p_unix.h"
#include "p_lx_exc.h"
#include "p_lx_elf.h"
#include "p_lx_sep.h"
#include "p_lx_sh.h"
#include "p_lx_interp.h"
#include "p_sys.h"
#include "p_tos.h"
#include "p_wcle.h"
#include "p_tmt.h"
#include "p_vxd.h"
#include "p_w16ne.h"
#include "p_w32pe.h"
#include "p_vmlinz.h"
#include "p_vmlinx.h"
#include "p_ps1.h"
#include "p_mach.h"
#include "p_armpe.h"


/*************************************************************************
//
**************************************************************************/

PackMaster::PackMaster(InputFile *f, struct options_t *o) :
    fi(f), p(NULL)
{
    // replace options with local options
    saved_opt = o;
    if (o)
    {
        this->local_options = *o;       // struct copy
        opt = &this->local_options;
    }
}


PackMaster::~PackMaster()
{
    fi = NULL;
    delete p; p = NULL;
    // restore options
    if (saved_opt)
        opt = saved_opt;
    saved_opt = NULL;
}


/*************************************************************************
//
**************************************************************************/

typedef Packer* (*try_function)(Packer *p, InputFile *f);

static Packer* try_pack(Packer *p, InputFile *f)
{
    if (p == NULL)
        return NULL;
#if !defined(UNUPX)
    try {
        p->initPackHeader();
        f->seek(0,SEEK_SET);
        if (p->canPack())
        {
            if (opt->cmd == CMD_COMPRESS)
                p->updatePackHeader();
            f->seek(0,SEEK_SET);
            return p;
        }
    } catch (const IOException&) {
    } catch (...) {
        delete p;
        throw;
    }
#endif /* UNUPX */
    delete p;
    return NULL;
}


static Packer* try_unpack(Packer *p, InputFile *f)
{
    if (p == NULL)
        return NULL;
    try {
        p->initPackHeader();
        f->seek(0,SEEK_SET);
        int r = p->canUnpack();
        if (r > 0)
        {
            f->seek(0,SEEK_SET);
            return p;
        }
        if (r < 0)
        {
            // FIXME - could stop testing all other unpackers at this time
            // see canUnpack() in packer.h
        }
    } catch (const IOException&) {
    } catch (...) {
        delete p;
        throw;
    }
    delete p;
    return NULL;
}


/*************************************************************************
//
**************************************************************************/

static Packer* try_packers(InputFile *f, try_function func)
{
    Packer *p = NULL;

    // note: order of tries is important !

    //
    // .exe
    //
    if (!opt->dos_exe.force_stub)
    {
        if ((p = func(new PackDjgpp2(f),f)) != NULL)
            return p;
        if ((p = func(new PackTmt(f),f)) != NULL)
            return p;
        if ((p = func(new PackWcle(f),f)) != NULL)
            return p;
#if 0
        if ((p = func(new PackVxd(f),f)) != NULL)
            return p;
#endif
        if ((p = func(new PackW16Ne(f),f)) != NULL)
            return p;
        if ((p = func(new PackArmPe(f),f)) != NULL)
            return p;
        if ((p = func(new PackW32Pe(f),f)) != NULL)
            return p;
    }
    if ((p = func(new PackExe(f),f)) != NULL)
        return p;

    //
    // atari
    //
    if ((p = func(new PackTos(f),f)) != NULL)
        return p;

    //
    // linux kernel
    //
    if ((p = func(new PackVmlinuxI386(f),f)) != NULL)
        return p;
    if ((p = func(new PackVmlinuzI386(f),f)) != NULL)
        return p;
    if ((p = func(new PackBvmlinuzI386(f),f)) != NULL)
        return p;
#if 0
    if ((p = func(new PackElks8086(f),f)) != NULL)
        return p;
#endif

    //
    // linux
    //
    if (!opt->o_unix.force_execve)
    {
#if 0
        if (opt->unix.script_name)
        {
            if ((p = func(new PackLinuxI386sep(f),f)) != NULL)
                return p;
        }
#endif
        if (opt->o_unix.use_ptinterp) {
            if ((p = func(new PackLinuxElf32x86interp(f),f)) != NULL)
                return p;
        }
        if ((p = func(new PackLinuxElf64amd(f),f)) != NULL)
            return p;
        if ((p = func(new PackLinuxElf32ppc(f),f)) != NULL)
            return p;
        if ((p = func(new PackLinuxElf32x86(f),f)) != NULL)
            return p;
        if ((p = func(new PackLinuxI386sh(f),f)) != NULL)
            return p;
    }
    if ((p = func(new PackLinuxI386(f),f)) != NULL)
        return p;

    //
    // psone
    //
    if ((p = func(new PackPs1(f),f)) != NULL)
        return p;

    //
    // .sys and .com
    //
    if ((p = func(new PackSys(f),f)) != NULL)
        return p;
    if ((p = func(new PackCom(f),f)) != NULL)
        return p;

    // Mach (MacOS X PowerPC)
    if ((p = func(new PackMachPPC32(f), f)) != NULL)
        return p;

    return NULL;
}


static Packer *getPacker(InputFile *f)
{
    Packer *p = try_packers(f, try_pack);
    if (!p)
        throwUnknownExecutableFormat();
    return p;
}


static Packer *getUnpacker(InputFile *f)
{
    Packer *p = try_packers(f, try_unpack);
    if (!p)
        throwNotPacked();
    return p;
}


static void assertPacker(const Packer *p)
{
    assert(p->getFormat() > 0);
    assert(p->getFormat() <= 255);
    assert(p->getVersion() >= 11);
    assert(p->getVersion() <= 14);
    assert(strlen(p->getName()) <= 13);
}


/*************************************************************************
// delegation
**************************************************************************/

void PackMaster::pack(OutputFile *fo)
{
    p = getPacker(fi);
    assertPacker(p);
    fi = NULL;
    p->doPack(fo);
}


void PackMaster::unpack(OutputFile *fo)
{
    p = getUnpacker(fi);
    assertPacker(p);
    fi = NULL;
    p->doUnpack(fo);
}


void PackMaster::test()
{
    p = getUnpacker(fi);
    assertPacker(p);
    fi = NULL;
    p->doTest();
}


void PackMaster::list()
{
    p = getUnpacker(fi);
    assertPacker(p);
    fi = NULL;
    p->doList();
}


void PackMaster::fileInfo()
{
    p = try_packers(fi, try_unpack);
    if (!p)
        p = try_packers(fi, try_pack);
    if (!p)
        throwUnknownExecutableFormat(NULL, 1);    // make a warning here
    assertPacker(p);
    fi = NULL;
    p->doFileInfo();
}


/*
vi:ts=4:et:nowrap
*/

