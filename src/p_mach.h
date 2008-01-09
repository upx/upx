/* p_mach.h --

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


#ifndef __UPX_P_MACHO_H
#define __UPX_P_MACHO_H


struct Mach_fat_header {
    BE32 magic;
        enum {  // note conflict with java bytecode PackLinuxI386
            FAT_MAGIC      = 0xcafebabe,
            FAT_MAGIC_SWAB = 0xbebafeca
        };
    BE32 nfat_arch;  // Number of Mach_fat_arch which follow.
}
__attribute_packed;

struct Mach_fat_arch {
    BE32 cputype;
    BE32 cpusubtype;
    BE32 offset;
    BE32 size;
    BE32 align;  /* shift count; log base 2 */
}
__attribute_packed;


/*************************************************************************
// Mach  Mach Object executable; all structures are target-endian
// 'otool' is the Mach analog of 'readelf' (convert executable file to ASCII).
**************************************************************************/

namespace N_Mach {

// integral types
template <class TWord, class TXword, class TAddr, class TOff>
struct MachITypes
{
    typedef TWord   Word;
    typedef TXword  Xword;
    typedef TAddr   Addr;
    typedef TOff    Off;
};

template <class TMachITypes>
struct Mach_header
{
    typedef typename TMachITypes::Word Word;

    Word magic;
    Word cputype;
    Word cpysubtype;
    Word filetype;
    Word ncmds;
    Word sizeofcmds;
    Word flags;
#define WANT_MACH_HEADER_ENUM 1
#include "p_mach_enum.h"
}
__attribute_packed;

template <class TMachITypes>
struct Mach_header64
{  // only difference is padding to 0 mod 8
    typedef typename TMachITypes::Word Word;

    Word magic;
    Word cputype;
    Word cpysubtype;
    Word filetype;
    Word ncmds;
    Word sizeofcmds;
    Word flags;
    Word reserved;  // pad to 0 mod 8
#define WANT_MACH_HEADER_ENUM 1
#include "p_mach_enum.h"
}
__attribute_packed;

template <class TMachITypes>
struct Mach_segment_command
{
    typedef typename TMachITypes::Word Word;
    typedef typename TMachITypes::Addr Addr;
    typedef typename TMachITypes::Off  Off;

    Word cmd;
    Word cmdsize;
    char segname[16];
    Addr vmaddr;
    Addr vmsize;
    Off  fileoff;
    Off  filesize;
    Word maxprot;
    Word initprot;
    Word nsects;
    Word flags;
#define WANT_MACH_SEGMENT_ENUM 1
#include "p_mach_enum.h"
}
__attribute_packed;

template <class TMachITypes>
struct Mach_section_command
{
    typedef typename TMachITypes::Word Word;
    typedef typename TMachITypes::Addr Addr;
    typedef typename TMachITypes::Off  Off;

    char sectname[16];
    char segname[16];
    Addr addr;   /* memory address */
    Addr size;   /* size in bytes */
    Word offset; /* file offset */  // FIXME: 64 bit?
    Word align;  /* power of 2 */
    Word reloff; /* file offset of relocation entries */
    Word nreloc; /* number of relocation entries */
    Word flags;  /* section type and attributes */
    Word reserved1;
    Word reserved2;
#define WANT_MACH_SECTION_ENUM 1
#include "p_mach_enum.h"
}
__attribute_packed;

template <class TMachITypes>
struct Mach_ppc_thread_state
{
    typedef typename TMachITypes::Addr Addr;

    Addr srr0;      /* Instruction address register (PC; entry addr) */
    Addr srr1;      /* Machine state register (supervisor) */
    Addr  r0, r1, r2, r3, r4, r5, r6, r7;
    Addr  r8, r9,r10,r11,r12,r13,r14,r15;
    Addr r16,r17,r18,r19,r20,r21,r22,r23;
    Addr r24,r25,r26,r27,r28,r29,r30,r31;

    Addr cr;        /* Condition register */  // FIXME: Word?
    Addr xer;       /* User's integer exception register */
    Addr lr;        /* Link register */
    Addr ctr;       /* Count register */
    Addr mq;        /* MQ register (601 only) */

    Addr vrsave;    /* Vector Save Register */
}
__attribute_packed;

template <class TMachITypes>
struct Mach_i386_thread_state
{
    typedef typename TMachITypes::Word Word;

    Word eax, ebx, ecx, edx;
    Word edi, esi, ebp;
    Word esp, ss;
    Word eflags;
    Word eip, cs;
    Word ds, es, fs, gs;
}
__attribute_packed;

template <class TMachITypes>
struct Mach_i386_new_thread_state
{
    typedef typename TMachITypes::Word Word;

    Word gs, fs, es, ds;
    Word edi, esi, ebp, esp;
    Word ebx, edx, ecx, eax;
    Word eip, cs, efl;
    Word uesp, ss;
}
__attribute_packed;

}  // namespace N_Mach

namespace N_Mach32 {

} // namespace N_Mach32

namespace N_Mach64 {

template <class TMachITypes>
struct Mach_ppc_thread_state64
{
    typedef typename TMachITypes::Word Word;
    typedef typename TMachITypes::Xword Xword;

    Xword srr0;    /* Instruction address register (PC; entry addr) */
    Xword srr1;    /* Machine state register (supervisor) */
    Xword  r0, r1, r2, r3, r4, r5, r6, r7;
    Xword  r8, r9,r10,r11,r12,r13,r14,r15;
    Xword r16,r17,r18,r19,r20,r21,r22,r23;
    Xword r24,r25,r26,r27,r28,r29,r30,r31;

    Word cr;        /* Condition register */  // FIXME: Xword?
    Xword xer;      /* User's integer exception register */
    Xword lr;       /* Link register */
    Xword ctr;      /* Count register */

    Word vrsave;    /* Vector Save Register */
}
__attribute_packed;

}  // namespace N_Mach64

namespace N_Mach {

template <class TP>
struct MachClass_32
{
    typedef TP BeLePolicy;

    // integral types (target endianness)
    typedef typename TP::U16 TE16;
    typedef typename TP::U32 TE32;
    typedef typename TP::U64 TE64;
    typedef N_Mach::MachITypes<TE32, TE64, TE32, TE32> MachITypes;
#if (ACC_CC_BORLANDC)
    typedef TE32 Addr;
#else
    typedef typename MachITypes::Addr Addr;
#endif

    // Mach types
    typedef N_Mach::Mach_header<MachITypes> Mach_header;
    typedef N_Mach::Mach_segment_command<MachITypes> Mach_segment_command;
    typedef N_Mach::Mach_section_command<MachITypes> Mach_section_command;
    typedef N_Mach::Mach_ppc_thread_state<MachITypes> Mach_ppc_thread_state;
    typedef N_Mach::Mach_i386_thread_state<MachITypes> Mach_i386_thread_state;

    static void compileTimeAssertions() {
        BeLePolicy::compileTimeAssertions();
        COMPILE_TIME_ASSERT(sizeof(Addr) == 4)
    }
};

template <class TP>
struct MachClass_64
{
    typedef TP BeLePolicy;

    // integral types (target endianness)
    typedef typename TP::U16 TE16;
    typedef typename TP::U32 TE32;
    typedef typename TP::U64 TE64;
    typedef N_Mach::MachITypes<TE32, TE64, TE64, TE64> MachITypes;

    // Mach types
    typedef N_Mach::Mach_header64<MachITypes> Mach_header;
    typedef N_Mach::Mach_segment_command<MachITypes> Mach_segment_command;
    typedef N_Mach::Mach_section_command<MachITypes> Mach_section_command;

    static void compileTimeAssertions() {
        BeLePolicy::compileTimeAssertions();
    }
};

}  // namespace N_Mach

typedef N_Mach::MachClass_32<N_BELE_CTP::HostPolicy> MachClass_Host32;
typedef N_Mach::MachClass_64<N_BELE_CTP::HostPolicy> MachClass_Host64;
typedef N_Mach::MachClass_32<N_BELE_CTP::BEPolicy>   MachClass_BE32;
typedef N_Mach::MachClass_64<N_BELE_CTP::BEPolicy>   MachClass_BE64;
typedef N_Mach::MachClass_32<N_BELE_CTP::LEPolicy>   MachClass_LE32;
typedef N_Mach::MachClass_64<N_BELE_CTP::LEPolicy>   MachClass_LE64;

// shortcuts
typedef MachClass_Host32::Mach_segment_command Mach32_segment_command;
typedef MachClass_Host32::Mach_section_command Mach32_section_command;

typedef MachClass_Host64::Mach_segment_command Mach64_segment_command;
typedef MachClass_Host64::Mach_section_command Mach64_section_command;

typedef MachClass_BE32::Mach_segment_command   MachBE32_segment_command;
typedef MachClass_BE32::Mach_section_command   MachBE32_section_command;

typedef MachClass_BE64::Mach_segment_command   MachBE64_segment_command;
typedef MachClass_BE64::Mach_section_command   MachBE64_section_command;

typedef MachClass_LE32::Mach_segment_command   MachLE32_segment_command;
typedef MachClass_LE32::Mach_section_command   MachLE32_section_command;

typedef MachClass_LE64::Mach_segment_command   MachLE64_segment_command;
typedef MachClass_LE64::Mach_section_command   MachLE64_section_command;

typedef MachClass_BE32::Mach_ppc_thread_state  Mach_ppc_thread_state;
typedef MachClass_LE32::Mach_i386_thread_state Mach_i386_thread_state;
#include "p_unix.h"


template <class TMachClass>
class PackMachBase : public PackUnix
{
    typedef PackUnix super;
protected:
    typedef TMachClass MachClass;
    typedef typename MachClass::BeLePolicy BeLePolicy;
    typedef typename MachClass::MachITypes MachITypes;
    // integral types (target endianness)
    typedef typename MachClass::TE16  TE16;
    typedef typename MachClass::TE32  TE32;
    typedef typename MachClass::TE64  TE64;
    typedef typename MachClass::Addr  Addr;
    // Mach types
    typedef typename MachClass::Mach_header Mach_header;
    typedef typename MachClass::Mach_segment_command Mach_segment_command;
    typedef typename MachClass::Mach_section_command Mach_section_command;

public:
    PackMachBase(InputFile *, unsigned cpuid, unsigned t_flavor, unsigned ts_word_cnt, unsigned tc_size);
    virtual ~PackMachBase();
    virtual int getVersion() const { return 13; }
    virtual const int *getCompressionMethods(int method, int level) const;

    // called by the generic pack()
    virtual void pack1(OutputFile *, Filter &);  // generate executable header
    virtual void pack2(OutputFile *, Filter &);  // append compressed data
    virtual void pack3(OutputFile *, Filter &) = 0;  // append loader
    virtual void pack4(OutputFile *, Filter &) = 0;  // append PackHeader

    virtual void pack1_setup_threado(OutputFile *const fo) = 0;
    virtual void unpack(OutputFile *fo);

    virtual bool canPack();
    virtual unsigned find_SEGMENT_gap(unsigned const k);

protected:
    virtual void patchLoader();
    virtual void patchLoaderChecksum();
    virtual void updateLoader(OutputFile *);
    virtual void buildMachLoader(
        upx_byte const *const proto,
        unsigned        const szproto,
        upx_byte const *const fold,
        unsigned        const szfold,
        Filter const *ft );
    virtual void defineSymbols(Filter const *);
    virtual void addStubEntrySections(Filter const *);

    static int __acc_cdecl_qsort compare_segment_command(void const *aa, void const *bb);

    unsigned my_cputype;
    unsigned my_thread_flavor;
    unsigned my_thread_state_word_count;
    unsigned my_thread_command_size;

    unsigned  n_segment;
    unsigned sz_segment;
    unsigned sz_mach_headers;
    Mach_segment_command *rawmseg;  // as input, with sections
    Mach_segment_command *msegcmd;  // LC_SEGMENT first, without sections
    Mach_header mhdri;

    Mach_header mhdro;
    Mach_segment_command segcmdo;

    struct b_info { // 12-byte header before each compressed block
        TE32 sz_unc;  // uncompressed_size
        TE32 sz_cpr;  //   compressed_size
        unsigned char b_method;  // compression algorithm
        unsigned char b_ftid;  // filter id
        unsigned char b_cto8;  // filter parameter
        unsigned char b_unused;
    }
    __attribute_packed;
    struct l_info { // 12-byte trailer in header for loader
        TE32 l_checksum;
        LE32 l_magic;
        TE16 l_lsize;
        unsigned char l_version;
        unsigned char l_format;
    }
    __attribute_packed;

    struct p_info { // 12-byte packed program header
        TE32 p_progid;
        TE32 p_filesize;
        TE32 p_blocksize;
    }
    __attribute_packed;

    struct l_info linfo;

    static void compileTimeAssertions() {
        MachClass::compileTimeAssertions();
        COMPILE_TIME_ASSERT(sizeof(b_info) == 12)
        COMPILE_TIME_ASSERT(sizeof(l_info) == 12)
        COMPILE_TIME_ASSERT(sizeof(p_info) == 12)
        COMPILE_TIME_ASSERT_ALIGNED1(b_info)
        COMPILE_TIME_ASSERT_ALIGNED1(l_info)
        COMPILE_TIME_ASSERT_ALIGNED1(p_info)
    }
};


class PackMachPPC32 : public PackMachBase<MachClass_BE32>
{
    typedef PackMachBase<MachClass_BE32> super;

public:
    PackMachPPC32(InputFile *f) : super(f, Mach_header::CPU_TYPE_POWERPC,
        Mach_thread_command::PPC_THREAD_STATE,
        sizeof(Mach_ppc_thread_state)>>2, sizeof(threado)) { }

    virtual int getFormat() const { return UPX_F_MACH_PPC32; }
    virtual const char *getName() const { return "Mach/ppc32"; }
    virtual const char *getFullName(const options_t *) const { return "powerpc-darwin.macho"; }

protected:
    virtual const int *getFilters() const;

    virtual void pack1_setup_threado(OutputFile *const fo);
    virtual void pack3(OutputFile *, Filter &);  // append loader
    virtual void pack4(OutputFile *, Filter &);  // append PackHeader
    virtual Linker* newLinker() const;
    virtual void buildLoader(const Filter *ft);

    struct Mach_thread_command
    {
        BE32 cmd;            /* LC_THREAD or  LC_UNIXTHREAD */
        BE32 cmdsize;        /* total size of this command */
        BE32 flavor;
        BE32 count;          /* sizeof(following_thread_state)/4 */
        Mach_ppc_thread_state state;
    #define WANT_MACH_THREAD_ENUM
    #include "p_mach_enum.h"
    }
    __attribute_packed;
    Mach_thread_command threado;
};

class PackMachI386 : public PackMachBase<MachClass_LE32>
{
    typedef PackMachBase<MachClass_LE32> super;

public:
    PackMachI386(InputFile *f) : super(f, Mach_header::CPU_TYPE_I386,
        (unsigned)Mach_thread_command::i386_THREAD_STATE,
        sizeof(Mach_i386_thread_state)>>2, sizeof(threado)) { }

    virtual int getFormat() const { return UPX_F_MACH_i386; }
    virtual const char *getName() const { return "Mach/i386"; }
    virtual const char *getFullName(const options_t *) const { return "i386-darwin.macho"; }
protected:
    virtual const int *getFilters() const;

    virtual void pack1_setup_threado(OutputFile *const fo);
    virtual void pack3(OutputFile *, Filter &);  // append loader
    virtual void pack4(OutputFile *, Filter &);  // append PackHeader
    virtual Linker* newLinker() const;
    virtual void buildLoader(const Filter *ft);
    virtual void addStubEntrySections(Filter const *);

    struct Mach_thread_command
    {
        LE32 cmd;            /* LC_THREAD or  LC_UNIXTHREAD */
        LE32 cmdsize;        /* total size of this command */
        LE32 flavor;
        LE32 count;          /* sizeof(following_thread_state)/4 */
        Mach_i386_thread_state state;
    #define WANT_MACH_THREAD_ENUM
    #include "p_mach_enum.h"
    }
    __attribute_packed;
    Mach_thread_command threado;
};

class PackMachFat : public Packer
{
    typedef Packer super;
public:
    PackMachFat(InputFile *f);
    virtual ~PackMachFat();

    virtual int getVersion() const { return 13; }
    virtual int getFormat() const { return UPX_F_MACH_FAT; }
    virtual const char *getName() const { return "Mach/fat"; }
    virtual const char *getFullName(const options_t *) const { return "fat-darwin.macho"; }
    virtual const int *getCompressionMethods(int method, int level) const;
    virtual const int *getFilters() const;

protected:
    // implementation
    virtual void pack(OutputFile *fo);
    virtual void unpack(OutputFile *fo);
    virtual void list();

public:
    virtual bool canPack();
    virtual int canUnpack();

protected:
    // loader core
    virtual void buildLoader(const Filter *ft);
    virtual Linker* newLinker() const;

#if (ACC_CC_BORLANDC)
public:
#endif
    enum { N_FAT_ARCH = 5 };
protected:
    struct Fat_head {
        struct Mach_fat_header fat;
        struct Mach_fat_arch arch[N_FAT_ARCH];
    }
    __attribute_packed;
    Fat_head fat_head;

    // UI handler
    UiPacker *uip;

    // linker
    Linker *linker;
#define WANT_MACH_HEADER_ENUM
#include "p_mach_enum.h"
};

#endif /* already included */


/*
vi:ts=4:et
*/

