/* packmast.cpp --

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
#include "lefile.h"
#include "pefile.h"
#include "p_elf.h"

#include "p_com.h"
#include "p_djgpp2.h"
#include "p_exe.h"
#include "p_unix.h"
#include "p_lx_exc.h"
#include "p_lx_elf.h"
//#include "p_lx_sep.h"
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

PackMaster::PackMaster(InputFile *f, options_t *o) :
    fi(f), p(NULL)
{
    // replace global options with local options
    saved_opt = o;
    if (o)
    {
        this->local_options = *o;       // struct copy
        //memcpy(&this->local_options, o, sizeof(*o)); // struct copy
        opt = &this->local_options;
    }
}


PackMaster::~PackMaster()
{
    fi = NULL;
    delete p; p = NULL;
    // restore global options
    if (saved_opt)
        opt = saved_opt;
    saved_opt = NULL;
}


/*************************************************************************
//
**************************************************************************/

static Packer* try_pack(Packer *p, void *user)
{
    if (p == NULL)
        return NULL;
    InputFile *f = (InputFile *) user;
    p->assertPacker();
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
    delete p;
    return NULL;
}


static Packer* try_unpack(Packer *p, void *user)
{
    if (p == NULL)
        return NULL;
    InputFile *f = (InputFile *) user;
    p->assertPacker();
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

Packer* PackMaster::visitAllPackers(visit_func_t func, InputFile *f, const options_t *o, void *user)
{
    Packer *p = NULL;

    // note: order of tries is important !

    //
    // .exe
    //
    if (!o->dos_exe.force_stub)
    {
        if ((p = func(new PackDjgpp2(f), user)) != NULL)
            return p;
        if ((p = func(new PackTmt(f), user)) != NULL)
            return p;
        if ((p = func(new PackWcle(f), user)) != NULL)
            return p;
#if 0
        if ((p = func(new PackVxd(f), user)) != NULL)
            return p;
#endif
#if 0
        if ((p = func(new PackW16Ne(f), user)) != NULL)
            return p;
#endif
        if ((p = func(new PackW32Pe(f), user)) != NULL)
            return p;
    }
    if ((p = func(new PackArmPe(f), user)) != NULL)
        return p;
    if ((p = func(new PackExe(f), user)) != NULL)
        return p;

    //
    // atari
    //
    if ((p = func(new PackTos(f), user)) != NULL)
        return p;

    //
    // linux kernel
    //
    if ((p = func(new PackVmlinuxARMEL(f), user)) != NULL)
        return p;
    if ((p = func(new PackVmlinuxARMEB(f), user)) != NULL)
        return p;
    if ((p = func(new PackVmlinuxPPC32(f), user)) != NULL)
        return p;
    if ((p = func(new PackVmlinuxAMD64(f), user)) != NULL)
        return p;
    if ((p = func(new PackVmlinuxI386(f), user)) != NULL)
        return p;
    if ((p = func(new PackVmlinuzI386(f), user)) != NULL)
        return p;
    if ((p = func(new PackBvmlinuzI386(f), user)) != NULL)
        return p;
#if 0
    if ((p = func(new PackElks8086(f), user)) != NULL)
        return p;
#endif

    //
    // linux
    //
    if (!o->o_unix.force_execve)
    {
#if 0
        if (o->unix.script_name)
        {
            if ((p = func(new PackLinuxI386sep(f), user)) != NULL)
                return p;
        }
#endif
        if (o->o_unix.use_ptinterp) {
            if ((p = func(new PackLinuxElf32x86interp(f), user)) != NULL)
                return p;
        }
        if ((p = func(new PackFreeBSDElf32x86(f), user)) != NULL)
            return p;
        if ((p = func(new PackNetBSDElf32x86(f), user)) != NULL)
            return p;
        if ((p = func(new PackOpenBSDElf32x86(f), user)) != NULL)
            return p;
        if ((p = func(new PackLinuxElf32x86(f), user)) != NULL)
            return p;
        if ((p = func(new PackLinuxElf64amd(f), user)) != NULL)
            return p;
        if ((p = func(new PackLinuxElf32armLe(f), user)) != NULL)
            return p;
        if ((p = func(new PackLinuxElf32armBe(f), user)) != NULL)
            return p;
        if ((p = func(new PackLinuxElf32ppc(f), user)) != NULL)
            return p;
        if ((p = func(new PackLinuxElf32mipsel(f), user)) != NULL)
            return p;
        if ((p = func(new PackLinuxElf32mipseb(f), user)) != NULL)
            return p;
        if ((p = func(new PackLinuxI386sh(f), user)) != NULL)
            return p;
    }
    if ((p = func(new PackBSDI386(f), user)) != NULL)
        return p;
    if ((p = func(new PackMachFat(f), user)) != NULL)  // cafebabe conflict
        return p;
    if ((p = func(new PackLinuxI386(f), user)) != NULL)  // cafebabe conflict
        return p;

    //
    // psone
    //
    if ((p = func(new PackPs1(f), user)) != NULL)
        return p;

    //
    // .sys and .com
    //
    if ((p = func(new PackSys(f), user)) != NULL)
        return p;
    if ((p = func(new PackCom(f), user)) != NULL)
        return p;

    // Mach (MacOS X PowerPC)
    if ((p = func(new PackMachPPC32(f), user)) != NULL)
        return p;
    if ((p = func(new PackMachI386(f), user)) != NULL)
        return p;

    return NULL;
}


Packer *PackMaster::getPacker(InputFile *f)
{
    Packer *p = visitAllPackers(try_pack, f, opt, f);
    if (!p)
        throwUnknownExecutableFormat();
    p->assertPacker();
    return p;
}


Packer *PackMaster::getUnpacker(InputFile *f)
{
    Packer *p = visitAllPackers(try_unpack, f, opt, f);
    if (!p)
        throwNotPacked();
    p->assertPacker();
    return p;
}


/*************************************************************************
// delegation
**************************************************************************/

void PackMaster::pack(OutputFile *fo)
{
    p = getPacker(fi);
    fi = NULL;
    p->doPack(fo);
}


void PackMaster::unpack(OutputFile *fo)
{
    p = getUnpacker(fi);
    p->assertPacker();
    fi = NULL;
    p->doUnpack(fo);
}


void PackMaster::test()
{
    p = getUnpacker(fi);
    fi = NULL;
    p->doTest();
}


void PackMaster::list()
{
    p = getUnpacker(fi);
    fi = NULL;
    p->doList();
}


void PackMaster::fileInfo()
{
    p = visitAllPackers(try_unpack, fi, opt, fi);
    if (!p)
        p = visitAllPackers(try_pack, fi, opt, fi);
    if (!p)
        throwUnknownExecutableFormat(NULL, 1);    // make a warning here
    p->assertPacker();
    fi = NULL;
    p->doFileInfo();
}


/*
vi:ts=4:et:nowrap
*/

