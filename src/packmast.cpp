/* packmast.cpp --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2023 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2023 Laszlo Molnar
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

PackMaster::PackMaster(InputFile *f, options_t *o) : fi(f), p(nullptr) {
    // replace global options with local options
    saved_opt = o;
    if (o) {
        memcpy(&this->local_options, o, sizeof(*o)); // struct copy
        opt = &this->local_options;
    }
}

PackMaster::~PackMaster() {
    fi = nullptr;
    delete p;
    p = nullptr;
    // restore global options
    if (saved_opt)
        opt = saved_opt;
    saved_opt = nullptr;
}

/*************************************************************************
//
**************************************************************************/

static Packer *try_pack(Packer *p, void *user) {
    if (p == nullptr)
        return nullptr;
    InputFile *f = (InputFile *) user;
    p->assertPacker();
    try {
        p->initPackHeader();
        f->seek(0, SEEK_SET);
        if (p->canPack()) {
            if (opt->cmd == CMD_COMPRESS)
                p->updatePackHeader();
            f->seek(0, SEEK_SET);
            return p;
        }
    } catch (const IOException &) {
    } catch (...) {
        delete p;
        throw;
    }
    delete p;
    return nullptr;
}

static Packer *try_unpack(Packer *p, void *user) {
    if (p == nullptr)
        return nullptr;
    InputFile *f = (InputFile *) user;
    p->assertPacker();
    try {
        p->initPackHeader();
        f->seek(0, SEEK_SET);
        int r = p->canUnpack();
        if (r > 0) {
            f->seek(0, SEEK_SET);
            return p;
        }
        if (r < 0) {
            // FIXME - could stop testing all other unpackers at this time
            // see canUnpack() in packer.h
        }
    } catch (const IOException &) {
    } catch (...) {
        delete p;
        throw;
    }
    delete p;
    return nullptr;
}

/*************************************************************************
//
**************************************************************************/

Packer *PackMaster::visitAllPackers(visit_func_t func, InputFile *f, const options_t *o,
                                    void *user) {
    Packer *p = nullptr;

#define D(Klass)                                                                                   \
    ACC_BLOCK_BEGIN                                                                                \
    Klass *const kp = new Klass(f);                                                                \
    if (o->debug.debug_level)                                                                      \
        fprintf(stderr, "visitAllPackers: (ver=%d, fmt=%3d) %s\n", kp->getVersion(),               \
                kp->getFormat(), #Klass);                                                          \
    if ((p = func(kp, user)) != nullptr)                                                           \
        return p;                                                                                  \
    ACC_BLOCK_END

    // note: order of tries is important !

    //
    // .exe
    //
    if (!o->dos_exe.force_stub) {
        D(PackDjgpp2);
        D(PackTmt);
        D(PackWcle);
        D(PackW64Pep);
        D(PackW32Pe);
    }
    D(PackArmPe);
    D(PackExe);

    //
    // atari
    //
    D(PackTos);

    //
    // linux kernel
    //
    D(PackVmlinuxARMEL);
    D(PackVmlinuxARMEB);
    D(PackVmlinuxPPC32);
    D(PackVmlinuxPPC64LE);
    D(PackVmlinuxAMD64);
    D(PackVmlinuxI386);
    D(PackVmlinuzI386);
    D(PackBvmlinuzI386);
    D(PackVmlinuzARMEL);

    //
    // linux
    //
    if (!o->o_unix.force_execve) {
        if (o->o_unix.use_ptinterp) {
            D(PackLinuxElf32x86interp);
        }
        D(PackFreeBSDElf32x86);
        D(PackNetBSDElf32x86);
        D(PackOpenBSDElf32x86);
        D(PackLinuxElf32x86);
        D(PackLinuxElf64amd);
        D(PackLinuxElf32armLe);
        D(PackLinuxElf32armBe);
        D(PackLinuxElf64arm);
        D(PackLinuxElf32ppc);
        D(PackLinuxElf64ppc);
        D(PackLinuxElf64ppcle);
        D(PackLinuxElf32mipsel);
        D(PackLinuxElf32mipseb);
        D(PackLinuxI386sh);
    }
    D(PackBSDI386);
    D(PackMachFat);   // cafebabe conflict
    D(PackLinuxI386); // cafebabe conflict

    //
    // psone
    //
    D(PackPs1);

    //
    // .sys and .com
    //
    D(PackSys);
    D(PackCom);

    // Mach (macOS)
    D(PackDylibAMD64);
    D(PackMachPPC32); // TODO: this works with upx 3.91..3.94 but got broken in 3.95; FIXME
    D(PackMachI386);
    D(PackMachAMD64);
    D(PackMachARMEL);
    D(PackMachARM64EL);

    // 2010-03-12  omit these because PackMachBase<T>::pack4dylib (p_mach.cpp)
    // does not understand what the Darwin (Apple Mac OS X) dynamic loader
    // assumes about .dylib file structure.
    //   D(PackDylibI386);
    //   D(PackDylibPPC32);

    return nullptr;
#undef D
}

Packer *PackMaster::getPacker(InputFile *f) {
    Packer *pp = visitAllPackers(try_pack, f, opt, f);
    if (!pp)
        throwUnknownExecutableFormat();
    pp->assertPacker();
    return pp;
}

Packer *PackMaster::getUnpacker(InputFile *f) {
    Packer *pp = visitAllPackers(try_unpack, f, opt, f);
    if (!pp)
        throwNotPacked();
    pp->assertPacker();
    return pp;
}

/*************************************************************************
// delegation
**************************************************************************/

void PackMaster::pack(OutputFile *fo) {
    p = getPacker(fi);
    fi = nullptr;
    p->doPack(fo);
}

void PackMaster::unpack(OutputFile *fo) {
    p = getUnpacker(fi);
    p->assertPacker();
    fi = nullptr;
    p->doUnpack(fo);
}

void PackMaster::test() {
    p = getUnpacker(fi);
    fi = nullptr;
    p->doTest();
}

void PackMaster::list() {
    p = getUnpacker(fi);
    fi = nullptr;
    p->doList();
}

void PackMaster::fileInfo() {
    p = visitAllPackers(try_unpack, fi, opt, fi);
    if (!p)
        p = visitAllPackers(try_pack, fi, opt, fi);
    if (!p)
        throwUnknownExecutableFormat(nullptr, 1); // make a warning here
    p->assertPacker();
    fi = nullptr;
    p->doFileInfo();
}

/* vim:set ts=4 sw=4 et: */
