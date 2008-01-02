/* packer_c.cpp -- Packer compression handling

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
#include "packer.h"
#include "linker.h"
//#include "filter.h"


/*************************************************************************
// compression method util
**************************************************************************/

bool Packer::isValidCompressionMethod(int method)
{
    if (M_IS_LZMA(method)) {
#if !defined(WITH_LZMA)
        assert(0 && "Internal error - LZMA not compiled in");
#else
        return true;
#endif
    }
    return (method >= M_NRV2B_LE32 && method <= M_LZMA);
}


const int *Packer::getDefaultCompressionMethods_8(int method, int level, int small) const
{
#define M_LZMA_003      (M_LZMA | 0x00300)
#define M_LZMA_407      (M_LZMA | 0x40700)
    static const int m_all[]   = { M_NRV2B_8, M_NRV2D_8, M_NRV2E_8, M_LZMA, M_ULTRA_BRUTE, M_LZMA_003, M_LZMA_407, M_END };
    //static const int m_cl1b[]  = { M_CL1B_8, M_END };
    static const int m_lzma[]  = { M_LZMA, M_END };
    static const int m_nrv2b[] = { M_NRV2B_8, M_END };
    static const int m_nrv2d[] = { M_NRV2D_8, M_END };
    static const int m_nrv2e[] = { M_NRV2E_8, M_END };

    if (method == M_ALL)    return m_all;
    //if (M_IS_CL1B(method))  return m_cl1b;
    if (M_IS_LZMA(method))  return m_lzma;
    if (M_IS_NRV2B(method)) return m_nrv2b;
    if (M_IS_NRV2D(method)) return m_nrv2d;
    if (M_IS_NRV2E(method)) return m_nrv2e;
    if (small < 0)
        small = file_size <= 512*1024;
    if (level == 1 || small)
        return m_nrv2b;
    return m_nrv2e;
}


const int *Packer::getDefaultCompressionMethods_le32(int method, int level, int small) const
{
    static const int m_all[]   = { M_NRV2B_LE32, M_NRV2D_LE32, M_NRV2E_LE32, M_LZMA, M_ULTRA_BRUTE, M_LZMA_003, M_LZMA_407, M_END };
    //static const int m_cl1b[]  = { M_CL1B_LE32, M_END };
    static const int m_lzma[]  = { M_LZMA, M_END };
    static const int m_nrv2b[] = { M_NRV2B_LE32, M_END };
    static const int m_nrv2d[] = { M_NRV2D_LE32, M_END };
    static const int m_nrv2e[] = { M_NRV2E_LE32, M_END };

    if (method == M_ALL)    return m_all;
    //if (M_IS_CL1B(method))  return m_cl1b;
    if (M_IS_LZMA(method))  return m_lzma;
    if (M_IS_NRV2B(method)) return m_nrv2b;
    if (M_IS_NRV2D(method)) return m_nrv2d;
    if (M_IS_NRV2E(method)) return m_nrv2e;
    if (small < 0)
        small = file_size <= 512*1024;
    if (level == 1 || small)
        return m_nrv2b;
    return m_nrv2e;
}


/*************************************************************************
// loader util
**************************************************************************/

const char *Packer::getDecompressorSections() const
{
    static const char nrv2b_le32_small[] =
        "N2BSMA10,N2BDEC10,N2BSMA20,N2BDEC20,N2BSMA30,"
        "N2BDEC30,N2BSMA40,N2BSMA50,N2BDEC50,N2BSMA60,"
        "N2BDEC60";
    static const char nrv2b_le32_fast[] =
        "N2BFAS10,+80CXXXX,N2BFAS11,N2BDEC10,N2BFAS20,"
        "N2BDEC20,N2BFAS30,N2BDEC30,N2BFAS40,N2BFAS50,"
        "N2BDEC50,N2BFAS60,+40CXXXX,N2BFAS61,N2BDEC60";
    static const char nrv2d_le32_small[] =
        "N2DSMA10,N2DDEC10,N2DSMA20,N2DDEC20,N2DSMA30,"
        "N2DDEC30,N2DSMA40,N2DSMA50,N2DDEC50,N2DSMA60,"
        "N2DDEC60";
    static const char nrv2d_le32_fast[] =
        "N2DFAS10,+80CXXXX,N2DFAS11,N2DDEC10,N2DFAS20,"
        "N2DDEC20,N2DFAS30,N2DDEC30,N2DFAS40,N2DFAS50,"
        "N2DDEC50,N2DFAS60,+40CXXXX,N2DFAS61,N2DDEC60";
    static const char nrv2e_le32_small[] =
        "N2ESMA10,N2EDEC10,N2ESMA20,N2EDEC20,N2ESMA30,"
        "N2EDEC30,N2ESMA40,N2ESMA50,N2EDEC50,N2ESMA60,"
        "N2EDEC60";
    static const char nrv2e_le32_fast[] =
        "N2EFAS10,+80CXXXX,N2EFAS11,N2EDEC10,N2EFAS20,"
        "N2EDEC20,N2EFAS30,N2EDEC30,N2EFAS40,N2EFAS50,"
        "N2EDEC50,N2EFAS60,+40CXXXX,N2EFAS61,N2EDEC60";
#if 0
    static const char cl1b_le32_small[] =
        "CL1ENTER,CL1SMA10,CL1RLOAD,"
        "CL1WID01,CL1SMA1B,"
        "CL1WID02,CL1SMA1B,"
        "CL1WID03,CL1SMA1B,"
        "CL1WID04,CL1SMA1B,"
        "CL1WID05,CL1SMA1B,"
        "CL1WID06,CL1SMA1B,"
        "CL1WID07,CL1SMA1B,"
        "CL1WID08,CL1SMA1B,"
        "CL1WID09,CL1SMA1B,"
        "CL1WID10,"
        "CL1START,"
        "CL1TOP00,CL1SMA1B,"
        "CL1TOP01,CL1SMA1B,"
        "CL1TOP02,CL1SMA1B,"
        "CL1TOP03,CL1SMA1B,"
        "CL1TOP04,CL1SMA1B,"
        "CL1TOP05,CL1SMA1B,"
        "CL1TOP06,CL1SMA1B,"
        "CL1TOP07,CL1SMA1B,"
        "CL1OFF01,CL1SMA1B,"
        "CL1OFF02,CL1SMA1B,"
        "CL1OFF03,CL1SMA1B,"
        "CL1OFF04,"
        "CL1LEN00,CL1SMA1B,"
        "CL1LEN01,CL1SMA1B,"
        "CL1LEN02,"
        "CL1COPY0";
    static const char cl1b_le32_fast[] =
        "CL1ENTER,"          "CL1RLOAD,"
        "CL1WID01,CL1FAS1B,"
        "CL1WID02,CL1FAS1B,"
        "CL1WID03,CL1FAS1B,"
        "CL1WID04,CL1FAS1B,"
        "CL1WID05,CL1FAS1B,"
        "CL1WID06,CL1FAS1B,"
        "CL1WID07,CL1FAS1B,"
        "CL1WID08,CL1FAS1B,"
        "CL1WID09,CL1FAS1B,"
        "CL1WID10,"
        "CL1START,"
        "CL1TOP00,CL1FAS1B,"
        "CL1TOP01,CL1FAS1B,"
        "CL1TOP02,CL1FAS1B,"
        "CL1TOP03,CL1FAS1B,"
        "CL1TOP04,CL1FAS1B,"
        "CL1TOP05,CL1FAS1B,"
        "CL1TOP06,CL1FAS1B,"
        "CL1TOP07,CL1FAS1B,"
        "CL1OFF01,CL1FAS1B,"
        "CL1OFF02,CL1FAS1B,"
        "CL1OFF03,CL1FAS1B,"
        "CL1OFF04,"
        "CL1LEN00,CL1FAS1B,"
        "CL1LEN01,CL1FAS1B,"
        "CL1LEN02,"
        "CL1COPY0";
#endif
    static const char lzma_small[] =
        "LZMA_DEC00,LZMA_DEC10,LZMA_DEC30";
    static const char lzma_fast[] =
        "LZMA_DEC00,+80C,LZMA_DEC20,LZMA_DEC30";
    static const char lzma_elf_small[] =
        "LZMA_ELF00,LZMA_DEC10,LZMA_DEC30";
    static const char lzma_elf_fast[] =
        "LZMA_ELF00,+80C,LZMA_DEC20,LZMA_DEC30";

    if (ph.method == M_NRV2B_LE32)
        return opt->small ? nrv2b_le32_small : nrv2b_le32_fast;
    if (ph.method == M_NRV2D_LE32)
        return opt->small ? nrv2d_le32_small : nrv2d_le32_fast;
    if (ph.method == M_NRV2E_LE32)
        return opt->small ? nrv2e_le32_small : nrv2e_le32_fast;
//    if (ph.method == M_CL1B_LE32)
//        return opt->small ? cl1b_le32_small  : cl1b_le32_fast;
    if (M_IS_LZMA(ph.method)) {
        if (UPX_F_LINUX_ELF_i386   ==ph.format
        ||  UPX_F_LINUX_ELFI_i386  ==ph.format
        ||  UPX_F_LINUX_ELF64_AMD  ==ph.format
        ||  UPX_F_LINUX_ELF32_ARMEL==ph.format
        ||  UPX_F_LINUX_ELFPPC32   ==ph.format
        ||  UPX_F_LINUX_ELF32_ARMEB==ph.format
        ||  UPX_F_BSD_ELF_i386     ==ph.format
        ||  UPX_F_VMLINUX_ARMEL    ==ph.format
        ||  UPX_F_VMLINUX_ARMEB    ==ph.format
        ||  UPX_F_VMLINUX_PPC32    ==ph.format
        ||  UPX_F_MACH_PPC32       ==ph.format
        ||  UPX_F_MACH_i386        ==ph.format
    ) {
            return opt->small ? lzma_elf_small  : lzma_elf_fast;
        }
        return opt->small ? lzma_small  : lzma_fast;
    }
    throwInternalError("bad decompressor");
    return NULL;
}


unsigned Packer::getDecompressorWrkmemSize() const
{
    unsigned size = 0;
    if (M_IS_LZMA(ph.method))
    {
        const lzma_compress_result_t *res = &ph.compress_result.result_lzma;
        // FIXME - this is for i386 only
        size = 8 + 4 + ALIGN_UP(2 * res->num_probs, 4u);
        size = ALIGN_UP(size, 16u);
    }
    assert((int)size >= 0);
    return size;
}

void Packer::defineDecompressorSymbols()
{
    if (UPX_F_LINUX_ELF_i386   ==ph.format
    ||  UPX_F_LINUX_ELFI_i386  ==ph.format
    ||  UPX_F_LINUX_ELF64_AMD  ==ph.format
    ||  UPX_F_LINUX_ELF32_ARMEL==ph.format
    ||  UPX_F_LINUX_ELFPPC32   ==ph.format
    ||  UPX_F_LINUX_ELF32_ARMEB==ph.format
    ||  UPX_F_BSD_ELF_i386     ==ph.format
    ||  UPX_F_VMLINUX_ARMEL    ==ph.format
    ||  UPX_F_VMLINUX_ARMEB    ==ph.format
    ||  UPX_F_VMLINUX_PPC32    ==ph.format
    ||  UPX_F_MACH_PPC32       ==ph.format
    ||  UPX_F_MACH_i386        ==ph.format
    ) {
        // ELF calls the decompressor many times; the parameters change!
        return;
    }
    if (M_IS_LZMA(ph.method))
    {
        const lzma_compress_result_t *res = &ph.compress_result.result_lzma;
        acc_uint32e_t properties = // lc, lp, pb, dummy
            (res->lit_context_bits << 0) |
            (res->lit_pos_bits << 8) |
            (res->pos_bits << 16);
        if (linker->bele->isBE()) // big endian - bswap32
            acc_swab32s(&properties);

        linker->defineSymbol("lzma_properties", properties);
        // len - 2 because of properties
        linker->defineSymbol("lzma_c_len", ph.c_len - 2);
        linker->defineSymbol("lzma_u_len", ph.u_len);
        unsigned stack = getDecompressorWrkmemSize();
        linker->defineSymbol("lzma_stack_adjust", 0u - stack);

        if (ph.format == UPX_F_DOS_EXE)
        {
            linker->defineSymbol("lzma_properties_hi", properties >> 16);   // pb
            linker->defineSymbol("lzma_c_len_hi", (ph.c_len - 2) >> 16);
            linker->defineSymbol("lzma_u_len_hi", ph.u_len >> 16);
            linker->defineSymbol("lzma_u_len_segment", (ph.u_len & 0xf0000) >> 4);
        }
    }
}


/*
vi:ts=4:et:nowrap
*/

