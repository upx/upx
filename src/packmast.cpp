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

#include "headers.h"
#include <memory> // std::unique_ptr
#include "conf.h"
#include "file.h"
#include "packmast.h"
#include "packer.h"

#include "lefile.h"
#include "pefile.h"
#include "p_elf.h"
#include "p_unix.h"

#include "p_com.h"
#include "p_djgpp2.h"
#include "p_exe.h"
#include "p_lx_elf.h"
#include "p_lx_exc.h"
#include "p_lx_interp.h"
#include "p_lx_sh.h"
#include "p_mach.h"
#include "p_ps1.h"
#include "p_sys.h"
#include "p_tmt.h"
#include "p_tos.h"
#include "p_vmlinx.h"
#include "p_vmlinz.h"
#include "p_w32pe_i386.h"
#include "p_w64pe_amd64.h"
#include "p_w64pe_arm64.h"
#include "p_wcle.h"
#include "p_wince_arm.h"

/*************************************************************************
//
**************************************************************************/

PackMaster::PackMaster(InputFile *f, Options *o) noexcept : fi(f) {
    // replace global options with local options
    if (o != nullptr) {
#if WITH_THREADS
        // TODO later: check for possible "noexcept" violation here
        std::lock_guard<std::mutex> lock(opt_lock_mutex);
#endif
        saved_opt = o;
        memcpy(&this->local_options, o, sizeof(*o)); // struct copy
        opt = &this->local_options;
    }
}

PackMaster::~PackMaster() noexcept {
    upx::owner_delete(packer);
    // restore global options
    if (saved_opt != nullptr) {
#if WITH_THREADS
        // TODO later: check for possible "noexcept" violation here
        std::lock_guard<std::mutex> lock(opt_lock_mutex);
#endif
        opt = saved_opt;
        saved_opt = nullptr;
    }
}

/*************************************************************************
//
**************************************************************************/

static noinline tribool try_can_pack(PackerBase *pb, void *user) may_throw {
    InputFile *f = (InputFile *) user;
    try {
        pb->initPackHeader();
        f->seek(0, SEEK_SET);
        tribool r = pb->canPack();
        if (r) {
            if (opt->cmd == CMD_COMPRESS)
                pb->updatePackHeader();
            f->seek(0, SEEK_SET);
            return true; // success
        }
        if (r.isThird()) // aka "-1"
            return r;    // canPack() says the format is recognized and we should fail early
    } catch (const IOException &) {
        // ignored
    }
    return false;
}

static noinline tribool try_can_unpack(PackerBase *pb, void *user) may_throw {
    InputFile *f = (InputFile *) user;
    try {
        pb->initPackHeader();
        f->seek(0, SEEK_SET);
        tribool r = pb->canUnpack();
        if (r) {
            f->seek(0, SEEK_SET);
            return true; // success
        }
        if (r.isThird()) // aka "-1"
            return r;    // canUnpack() says the format is recognized and we should fail early
    } catch (const IOException &) {
        // ignored
    }
    return false;
}

/*************************************************************************
//
**************************************************************************/

/*static*/
PackerBase *PackMaster::visitAllPackers(visit_func_t func, InputFile *f, const Options *o,
                                        void *user) may_throw {
#define D(Klass)                                                                                   \
    ACC_BLOCK_BEGIN                                                                                \
    COMPILE_TIME_ASSERT(std::is_nothrow_destructible_v<Klass>)                                     \
    auto pb = std::unique_ptr<PackerBase>(new Klass(f));                                           \
    if (o->debug.debug_level)                                                                      \
        fprintf(stderr, "visitAllPackers: (ver=%d, fmt=%3d) %s\n", pb->getVersion(),               \
                pb->getFormat(), #Klass);                                                          \
    pb->assertPacker();                                                                            \
    tribool r = func(pb.get(), user);                                                              \
    if (r)                                                                                         \
        return pb.release(); /* success */                                                         \
    if (r.isThird())                                                                               \
        return nullptr; /* stop and fail early */                                                  \
    ACC_BLOCK_END

    // NOTE: order of tries is important !!!

    //
    // .exe
    //
    if (!o->dos_exe.force_stub) {
        // dos32
        D(PackDjgpp2);
        D(PackTmt);
        D(PackWcle);
        // Windows
        // D(PackW64PeArm64EC); // NOT YET IMPLEMENTED
        // D(PackW64PeArm64); // NOT YET IMPLEMENTED
        D(PackW64PeAmd64);
        D(PackW32PeI386);
        D(PackWinCeArm);
    }
    D(PackExe); // dos/exe

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

    // Mach (Darwin / macOS)
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

    //
    // misc
    //
    D(PackTos); // atari/tos
    D(PackPs1); // ps1/exe
    D(PackSys); // dos/sys
    D(PackCom); // dos/com

    return nullptr;
#undef D
}

/*static*/ PackerBase *PackMaster::getPacker(InputFile *f) {
    PackerBase *pb = visitAllPackers(try_can_pack, f, opt, f);
    if (!pb)
        throwUnknownExecutableFormat();
    return pb;
}

/*static*/ PackerBase *PackMaster::getUnpacker(InputFile *f) {
    PackerBase *pb = visitAllPackers(try_can_unpack, f, opt, f);
    if (!pb)
        throwNotPacked();
    return pb;
}

/*************************************************************************
// delegation from work.cpp
**************************************************************************/

void PackMaster::pack(OutputFile *fo) {
    assert(packer == nullptr);
    packer = getPacker(fi);
    packer->doPack(fo);
}

void PackMaster::unpack(OutputFile *fo) {
    assert(packer == nullptr);
    packer = getUnpacker(fi);
    packer->doUnpack(fo);
}

void PackMaster::test() {
    assert(packer == nullptr);
    packer = getUnpacker(fi);
    packer->doTest();
}

void PackMaster::list() {
    assert(packer == nullptr);
    packer = getUnpacker(fi);
    packer->doList();
}

void PackMaster::fileInfo() {
    assert(packer == nullptr);
    packer = visitAllPackers(try_can_unpack, fi, opt, fi);
    if (!packer)
        packer = visitAllPackers(try_can_pack, fi, opt, fi);
    if (!packer)
        throwUnknownExecutableFormat(nullptr, 1); // make a warning here
    packer->doFileInfo();
}

/* vim:set ts=4 sw=4 et: */
