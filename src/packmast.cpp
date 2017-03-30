/* packmast.cpp --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2017 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2017 Laszlo Molnar
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
   <markus@oberhumer.com>               <ezerotven+github@gmail.com>
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
#include "p_lx_sh.h"
#include "p_lx_interp.h"
#include "p_sys.h"
#include "p_tos.h"
#include "p_wcle.h"
#include "p_tmt.h"
#include "p_w32pe.h"
#include "p_w64pep.h"
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
        memcpy(&this->local_options, o, sizeof(*o)); // struct copy
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



#define D(name) if (o->debug.debug_level) fprintf(stderr, "%s\n", #name)

Packer* PackMaster::visitAllPackers(visit_func_t func, InputFile *f, const options_t *o, void *user)
{
    Packer *p = NULL;

    // note: order of tries is important !

    //
    // .exe
    //
    if (!o->dos_exe.force_stub)
    {
        D(PackDjgpp2); if ((p = func(new PackDjgpp2(f), user)))
            return p;
        D(PackTmt); if ((p = func(new PackTmt(f), user)))
            return p;
        D(PackWcle); if ((p = func(new PackWcle(f), user)))
            return p;
        D(PackW64Pep); if ((p = func(new PackW64Pep(f), user)))
            return p;
        D(PackW32Pe); if ((p = func(new PackW32Pe(f), user)))
            return p;
    }
    D(PackArmPe); if ((p = func(new PackArmPe(f), user)))
        return p;
    D(PackExe); if ((p = func(new PackExe(f), user)))
        return p;

    //
    // atari
    //
    D(PackTos); if ((p = func(new PackTos(f), user)))
        return p;

    //
    // linux kernel
    //
    D(PackVmlinuxARMEL); if ((p = func(new PackVmlinuxARMEL(f), user)))
        return p;
    D(PackVmlinuxARMEB); if ((p = func(new PackVmlinuxARMEB(f), user)))
        return p;
    D(PackVmlinuxPPC32); if ((p = func(new PackVmlinuxPPC32(f), user)))
        return p;
    D(PackVmlinuxPPC64LE); if ((p = func(new PackVmlinuxPPC64LE(f), user)))
        return p;
    D(PackVmlinuxAMD64); if ((p = func(new PackVmlinuxAMD64(f), user)))
        return p;
    D(PackVmlinuxI386); if ((p = func(new PackVmlinuxI386(f), user)))
        return p;
    D(PackVmlinuzI386); if ((p = func(new PackVmlinuzI386(f), user)))
        return p;
    D(PackBvmlinuzI386); if ((p = func(new PackBvmlinuzI386(f), user)))
        return p;
    D(PackVmlinuzARMEL); if ((p = func(new PackVmlinuzARMEL(f), user)))
        return p;

    //
    // linux
    //
    if (!o->o_unix.force_execve)
    {
        if (o->o_unix.use_ptinterp) {
            D(PackLinuxElf32x86interp); if ((p = func(new PackLinuxElf32x86interp(f), user)))
                return p;
        }
        D(PackFreeBSDElf32x86); if ((p = func(new PackFreeBSDElf32x86(f), user)))
            return p;
        D(PackNetBSDElf32x86); if ((p = func(new PackNetBSDElf32x86(f), user)))
            return p;
        D(PackOpenBSDElf32x86); if ((p = func(new PackOpenBSDElf32x86(f), user)))
            return p;
        D(PackLinuxElf32x86); if ((p = func(new PackLinuxElf32x86(f), user)))
            return p;
        D(PackLinuxElf64amd); if ((p = func(new PackLinuxElf64amd(f), user)))
            return p;
        D(PackLinuxElf32armLe); if ((p = func(new PackLinuxElf32armLe(f), user)))
            return p;
        D(PackLinuxElf32armBe); if ((p = func(new PackLinuxElf32armBe(f), user)))
            return p;
        D(PackLinuxElf64arm); if ((p = func(new PackLinuxElf64arm(f), user)))
            return p;
        D(PackLinuxElf32ppc); if ((p = func(new PackLinuxElf32ppc(f), user)))
            return p;
        D(PackLinuxElf64ppcle); if ((p = func(new PackLinuxElf64ppcle(f), user)))
            return p;
        D(PackLinuxElf32mipsel); if ((p = func(new PackLinuxElf32mipsel(f), user)))
            return p;
        D(PackLinuxElf32mipseb); if ((p = func(new PackLinuxElf32mipseb(f), user)))
            return p;
        D(PackLinuxI386sh); if ((p = func(new PackLinuxI386sh(f), user)))
            return p;
    }
    D(PackBSDI386); if ((p = func(new PackBSDI386(f), user)))
        return p;
    D(PackMachFat); if ((p = func(new PackMachFat(f), user)))  // cafebabe conflict
        return p;
    D(PackLinuxI386); if ((p = func(new PackLinuxI386(f), user)))  // cafebabe conflict
        return p;

    //
    // psone
    //
    D(PackPs1); if ((p = func(new PackPs1(f), user)))
        return p;

    //
    // .sys and .com
    //
    D(PackSys); if ((p = func(new PackSys(f), user)))
        return p;
    D(PackCom); if ((p = func(new PackCom(f), user)))
        return p;

    // Mach (MacOS X PowerPC)
    D(PackMachPPC32); if ((p = func(new PackMachPPC32(f), user)))
        return p;
    D(PackMachPPC64LE); if ((p = func(new PackMachPPC64LE(f), user)))
        return p;
    D(PackMachI386); if ((p = func(new PackMachI386(f), user)))
        return p;
    D(PackMachAMD64); if ((p = func(new PackMachAMD64(f), user)))
        return p;
    D(PackMachARMEL); if ((p = func(new PackMachARMEL(f), user)))
        return p;

    // 2010-03-12  omit these because PackMachBase<T>::pack4dylib (p_mach.cpp)
    // does not understand what the Darwin (Apple Mac OS X) dynamic loader
    // assumes about .dylib file structure.
    // D(PackDylibI386); if ((p = func(new PackDylibI386(f), user)))
    //    return p;
    // D(PackDylibPPC32); if ((p = func(new PackDylibPPC32(f), user)))
    //    return p;
    // D(PackDylibAMD64); if ((p = func(new PackDylibAMD64(f), user)))
    //    return p;

    return NULL;
}


Packer *PackMaster::getPacker(InputFile *f)
{
    Packer *pp = visitAllPackers(try_pack, f, opt, f);
    if (!pp)
        throwUnknownExecutableFormat();
    pp->assertPacker();
    return pp;
}


Packer *PackMaster::getUnpacker(InputFile *f)
{
    Packer *pp = visitAllPackers(try_unpack, f, opt, f);
    if (!pp)
        throwNotPacked();
    pp->assertPacker();
    return pp;
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

/* vim:set ts=4 sw=4 et: */
