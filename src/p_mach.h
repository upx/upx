/* p_mach.h --

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


#ifndef __UPX_P_MACHO_H
#define __UPX_P_MACHO_H 1

#include "conf.h"

__packed_struct(Mach_fat_header)
    BE32 magic;
# if 0
        enum {  // note conflict with java bytecode PackLinuxI386
            FAT_MAGIC      = 0xcafebabe,
            FAT_MAGIC_SWAB = 0xbebafeca
        };
# else
        static const unsigned FAT_MAGIC      = 0xcafebabe;
        static const unsigned FAT_MAGIC_SWAB = 0xbebafeca;
# endif
    BE32 nfat_arch;  // Number of Mach_fat_arch which follow.
__packed_struct_end()

__packed_struct(Mach_fat_arch)
    BE32 cputype;
    BE32 cpusubtype;
    BE32 offset;
    BE32 size;
    BE32 align;  /* shift count; log base 2 */
__packed_struct_end()

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
__packed_struct(Mach_header)
    typedef typename TMachITypes::Word Word;

    Word magic;
    Word cputype;
    Word cpusubtype;
    Word filetype;
    Word ncmds;
    Word sizeofcmds;
    Word flags;
#define WANT_MACH_HEADER_ENUM 1
#include "p_mach_enum.h"
__packed_struct_end()

template <class TMachITypes>
__packed_struct(Mach_header64)
    // only difference is padding to 0 mod 8
    typedef typename TMachITypes::Word Word;

    Word magic;
    Word cputype;
    Word cpusubtype;
    Word filetype;
    Word ncmds;
    Word sizeofcmds;
    Word flags;
    Word reserved;  // pad to 0 mod 8
#define WANT_MACH_HEADER_ENUM 1
#include "p_mach_enum.h"
__packed_struct_end()

template <class TMachITypes>
__packed_struct(Mach_command)  // generic prefix
    typedef typename TMachITypes::Word Word;

    Word cmd;
    Word cmdsize;
    Word data[2];  // because cmdsize >= 16
#define WANT_MACH_SEGMENT_ENUM 1
#include "p_mach_enum.h"
__packed_struct_end()

template <class TMachITypes>
__packed_struct(Mach_segment_command)
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
__packed_struct_end()

template <class TMachITypes>
__packed_struct(Mach_section_command)
    typedef typename TMachITypes::Word Word;
    typedef typename TMachITypes::Addr Addr;
    typedef typename TMachITypes::Off  Off;

    char sectname[16];
    char segname[16];
    Addr addr;   /* memory address */
    Addr size;   /* size in bytes */
    Word offset; /* file offset */
    Word align;  /* power of 2 */
    Word reloff; /* file offset of relocation entries */
    Word nreloc; /* number of relocation entries */
    Word flags;  /* section type and attributes */
    Word reserved1;  /* for offset or index */
    Word reserved2;  /* for count or sizeof */
#define WANT_MACH_SECTION_ENUM 1
#include "p_mach_enum.h"
__packed_struct_end()

template <class TMachITypes>
__packed_struct(Mach_section_command_64)
    typedef typename TMachITypes::Word Word;
    typedef typename TMachITypes::Addr Addr;
    typedef typename TMachITypes::Off  Off;
    typedef typename TMachITypes::Word Off32;
    typedef typename TMachITypes::Xword Xword;

    char sectname[16];
    char segname[16];
    Addr addr;   /* memory address */
    Addr size;   /* size in bytes */
    Off32 offset; /* file offset */
    Word align;  /* power of 2 */
    Word reloff; /* file offset of relocation entries */
    Word nreloc; /* number of relocation entries */
    Word flags;  /* section type and attributes */
    Word reserved1;  /* for offset or index */
    Word reserved2;  /* for count or sizeof */
    Word reserved3;  /* NOT IN 32-bit VERSION!! */
#define WANT_MACH_SECTION_ENUM 1
#include "p_mach_enum.h"
__packed_struct_end()

template <class TMachITypes>
__packed_struct(Mach_symtab_command)
    typedef typename TMachITypes::Word Word;
    typedef typename TMachITypes::Off  Off;
    typedef typename TMachITypes::Word Off32;

    Word cmd;      /* LC_SYMTAB */
    Word cmdsize;  /* sizeof(struct Mach_symtab_command) */
    Off32 symoff;  /* symbol table offset */
    Word nsyms;    /* number of symbol table entries */
    Off32 stroff;  /* string table offset */
    Word strsize;  /* string table size in bytes */
__packed_struct_end()

template <class TMachITypes>
__packed_struct(Mach_dysymtab_command)
    typedef typename TMachITypes::Word Word;
    typedef typename TMachITypes::Off  Off;
    typedef typename TMachITypes::Word Off32;

    Word cmd;           /* LC_DYSYMTAB */
    Word cmdsize;       /* sizeof(struct Mach_dysymtab_command) */
    Word ilocalsym;     /* index to local symbols */
    Word nlocalsym;     /* number of local symbols */
    Word iextdefsym;    /* index to externally defined symbols */
    Word nextdefsym;    /* number of externally defined symbols */
    Word iundefsym;     /* index to undefined symbols */
    Word nundefsym;     /* number of undefined symbols */
    Off32 tocoff;       /* file offset to table of contents */
    Word ntoc;          /* number of entries in table of contents */
    Off32 modtaboff;    /* file offset to module table */
    Word nmodtab;       /* number of module table entries */
    Off32 extrefsymoff; /* offset to referenced symbol table */
    Word nextrefsymoff; /* number of referenced symbol table entries */
    Off32 indirectsymoff; /* file offset to the indirect symbol table */
    Word nindirectsyms; /* number of indirect symbol table entries */
    Off32 extreloff;    /* offset to external relocation entries */
    Word nextrel;       /* number of external relocation entries */
    Off32 locreloff;      /* offset to local relocation entries */
    Word nlocrel;       /* number of local relocation entries */
__packed_struct_end()

template <class TMachITypes>
__packed_struct(Mach_segsplit_info_command)
    typedef typename TMachITypes::Word Word;
    typedef typename TMachITypes::Off  Off;

    Word cmd;           /* LC_SEGMENT_SPLIT_INFO */
    Word cmdsize;       /* sizeof(struct Mach_segsplit_info_command) */
    Off dataoff;
    Word datasize;
__packed_struct_end()

template <class TMachITypes>
__packed_struct(Mach_routines_command)
    typedef typename TMachITypes::Word Word;
    typedef typename TMachITypes::Addr Addr;

    Word cmd;
    Word cmdsize;
    Addr init_address;
    Word init_module;
    Word reserved1;
    Word reserved2;
    Word reserved3;
    Word reserved4;
    Word reserved5;
    Word reserved6;
#define WANT_MACH_SEGMENT_ENUM 1
#include "p_mach_enum.h"
__packed_struct_end()

template <class TMachITypes>
__packed_struct(Mach_routines_command_64)
    typedef typename TMachITypes::Word Word;
    typedef typename TMachITypes::Addr Addr;
    typedef typename TMachITypes::Xword Xword;

    Word cmd;
    Word cmdsize;
    Addr init_address;
    Xword init_module;
    Xword reserved1;
    Xword reserved2;
    Xword reserved3;
    Xword reserved4;
    Xword reserved5;
    Xword reserved6;
#define WANT_MACH_SEGMENT_ENUM 1
#include "p_mach_enum.h"
__packed_struct_end()

template <class TMachITypes>
__packed_struct(Mach_twolevel_hints_command)
    typedef typename TMachITypes::Word Word;
    typedef typename TMachITypes::Off  Off;

    Word cmd;
    Word cmdsize;
    Off offset;   /* offset to the hint table */
    Word nhints;  /* number of hints */
#define WANT_MACH_SEGMENT_ENUM 1
#include "p_mach_enum.h"
__packed_struct_end()

template <class TMachITypes>
__packed_struct(Mach_linkedit_data_command)
    typedef typename TMachITypes::Word Word;

    Word cmd;  // LC_CODE_SIGNATURE, LC_SEGMENT_SPLIT_INFO,
               // LC_FUNCTION_STARTS, LC_DATA_IN_CODE,
               // LC_DYLIB_CODE_SIGN_DRS, LC_LINKER_OPTIMIZATION_HINT
    Word cmdsize;
    Word dataoff;  // file offset of data in __LINKEDIT segment
    Word datasize;  // file size of data in __LINKEDIT segment
__packed_struct_end()

template <class TMachITypes>
__packed_struct(Mach_uuid_command)
    typedef typename TMachITypes::Word Word;

    Word cmd;  // LC_UUID
    Word cmdsize;  // 24
    unsigned char uuid[16];
__packed_struct_end()

template <class TMachITypes, class TMachThreadState>
__packed_struct(Mach_thread_command)
    typedef typename TMachITypes::Word Word;

    Word cmd;            /* LC_THREAD or  LC_UNIXTHREAD */
    Word cmdsize;        /* total size of this command */
    Word flavor;
    Word count;          /* sizeof(following_thread_state)/4 */
    TMachThreadState state;
#define WANT_MACH_THREAD_ENUM 1
#include "p_mach_enum.h"
__packed_struct_end()

template <class TMachITypes>
__packed_struct(Mach_main_command)
    typedef typename TMachITypes::Word Word;
    typedef typename TMachITypes::Xword Xword;

    Word cmd;  // LC_MAIN;  MH_EXECUTE only
    Word cmdsize;  // 24
    Xword entryoff;  // file offset of main() [expected in __TEXT]
    Xword stacksize;  // non-default initial stack size
__packed_struct_end()

template <class TMachITypes>
__packed_struct(Mach_source_version_command)
    typedef typename TMachITypes::Word Word;

    Word cmd;  // LC_SOURCE_VERSION
    Word cmdsize;  // 16
    Word version;
    Word __pad;  // to 0 mod 8
__packed_struct_end()

template <class TMachITypes>
__packed_struct(Mach_version_min_command)
    typedef typename TMachITypes::Word Word;

    Word cmd;  // LC_VERSION_MIN_MACOSX
    Word cmdsize;  // 16
    Word version;  // X.Y.Z ==> xxxx.yy.zz
    Word sdk;  // X.Y.Z ==> xxxx.yy.zz
__packed_struct_end()

template <class TMachITypes>
__packed_struct(Mach_dyld_info_only_command)
    typedef typename TMachITypes::Word Word;

    Word cmd;  // LC_DYLD_INFO_ONLY
    Word cmdsize;  // 48
    Word rebase_off;
    Word rebase_size;
    Word bind_off;
    Word bind_size;
    Word weak_bind_off;
    Word weak_bind_size;
    Word lazy_bind_off;
    Word lazy_bind_size;
    Word export_off;
    Word export_size;
__packed_struct_end()

template <class TMachITypes>
__packed_struct(Mach_load_dylinker_command)
    typedef typename TMachITypes::Word Word;

    Word cmd;
    Word cmdsize;
    Word name;
__packed_struct_end()

template <class TMachITypes>
__packed_struct(Mach_dylib)
    typedef typename TMachITypes::Word Word;

    Word name;         /* library's path name */
    Word timestamp;         /* library's build time stamp */
    Word current_version;       /* library's current version number */
    Word compatibility_version; /* library's compatibility vers number*/
__packed_struct_end()

template <class TMachITypes>
__packed_struct(Mach_load_dylib_command)
    typedef typename TMachITypes::Word Word;

    Word cmd;
    Word cmdsize;
    Mach_dylib<TMachITypes> dylib;
__packed_struct_end()

}  // namespace N_Mach

namespace N_Mach32 {

template <class TMachITypes>
__packed_struct(Mach_ppc_thread_state)
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
__packed_struct_end()

template <class TMachITypes>
__packed_struct(Mach_i386_thread_state)
    typedef typename TMachITypes::Word Word;

    Word eax, ebx, ecx, edx;
    Word edi, esi, ebp;
    Word esp, ss;
    Word eflags;
    Word eip, cs;
    Word ds, es, fs, gs;
__packed_struct_end()

template <class TMachITypes>
__packed_struct(Mach_ARM_thread_state)
    typedef typename TMachITypes::Word Word;

    Word r[13];  // r0-r12
    Word sp;  // r13
    Word lr;  // r14
    Word pc;  // r15
    Word cpsr;
__packed_struct_end()

} // namespace N_Mach32

namespace N_Mach64 {

template <class TMachITypes>
__packed_struct(Mach_AMD64_thread_state)
    typedef typename TMachITypes::Xword Xword;

    Xword rax, rbx, rcx, rdx;
    Xword rdi, rsi, rbp, rsp;
    Xword  r8,  r9, r10, r11;
    Xword r12, r13, r14, r15;
    Xword rip, rflags;
    Xword cs, fs, gs;
__packed_struct_end()

template <class TMachITypes>
__packed_struct(Mach_ppc_thread_state64)
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
__packed_struct_end()

template <class TMachITypes> __packed_struct(Mach_ARM64_thread_state)
    typedef typename TMachITypes::Xword Xword;
    typedef typename TMachITypes::Word Word;

    Xword x0,  x1,  x2,  x3;
    Xword x4,  x5,  x6,  x7;
    Xword x8,  x9,  x10, x11;
    Xword x12, x13, x14, x15;
    Xword x16, x17, x18, x19;
    Xword x20, x21, x22, x23;
    Xword x24, x25, x26, x27;
    Xword x28, fp,  lr,  sp;
    Xword pc;
    Word cpsr;
    Word pad;  // to (0 mod 8)
__packed_struct_end()

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
    typedef typename MachITypes::Addr Addr;

    // Mach types
    typedef N_Mach::Mach_header<MachITypes> Mach_header;
    typedef N_Mach::Mach_command<MachITypes> Mach_command;
    typedef N_Mach::Mach_segment_command<MachITypes> Mach_segment_command;
    typedef N_Mach::Mach_section_command<MachITypes> Mach_section_command;
    typedef N_Mach::Mach_symtab_command<MachITypes>  Mach_symtab_command;
    typedef N_Mach::Mach_dysymtab_command<MachITypes> Mach_dysymtab_command;
    typedef N_Mach::Mach_segsplit_info_command<MachITypes> Mach_segsplit_info_command;
    typedef N_Mach::Mach_routines_command<MachITypes> Mach_routines_command;
    typedef N_Mach::Mach_twolevel_hints_command<MachITypes> Mach_twolevel_hints_command;
    typedef N_Mach::Mach_linkedit_data_command<MachITypes> Mach_linkedit_data_command;
    typedef N_Mach::Mach_uuid_command<MachITypes> Mach_uuid_command;
    typedef N_Mach::Mach_load_dylib_command<MachITypes> Mach_load_dylib_command;
    typedef N_Mach::Mach_dylib<MachITypes> Mach_dylib;
    typedef N_Mach::Mach_load_dylinker_command<MachITypes> Mach_load_dylinker_command;
    typedef N_Mach::Mach_dyld_info_only_command<MachITypes> Mach_dyld_info_only_command;
    typedef N_Mach::Mach_version_min_command<MachITypes> Mach_version_min_command;
    typedef N_Mach::Mach_source_version_command<MachITypes> Mach_source_version_command;
    typedef N_Mach::Mach_main_command<MachITypes> Mach_main_command;

    typedef N_Mach32::Mach_ppc_thread_state<MachITypes> Mach_ppc_thread_state;
    typedef N_Mach32::Mach_i386_thread_state<MachITypes> Mach_i386_thread_state;
    typedef N_Mach32::Mach_ARM_thread_state<MachITypes> Mach_ARM_thread_state;

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
    typedef typename MachITypes::Addr Addr;

    // Mach types
    typedef N_Mach::Mach_header64<MachITypes> Mach_header;
    typedef N_Mach::Mach_command<MachITypes> Mach_command;
    typedef N_Mach::Mach_segment_command<MachITypes> Mach_segment_command;
    typedef N_Mach::Mach_section_command_64<MachITypes> Mach_section_command;
    typedef N_Mach::Mach_symtab_command<MachITypes>  Mach_symtab_command;
    typedef N_Mach::Mach_dysymtab_command<MachITypes> Mach_dysymtab_command;
    typedef N_Mach::Mach_segsplit_info_command<MachITypes> Mach_segsplit_info_command;
    typedef N_Mach::Mach_routines_command_64<MachITypes> Mach_routines_command;
    typedef N_Mach::Mach_twolevel_hints_command<MachITypes> Mach_twolevel_hints_command;
    typedef N_Mach::Mach_linkedit_data_command<MachITypes> Mach_linkedit_data_command;
    typedef N_Mach::Mach_uuid_command<MachITypes> Mach_uuid_command;
    typedef N_Mach::Mach_load_dylib_command<MachITypes> Mach_load_dylib_command;
    typedef N_Mach::Mach_dylib<MachITypes> Mach_dylib;
    typedef N_Mach::Mach_load_dylinker_command<MachITypes> Mach_load_dylinker_command;
    typedef N_Mach::Mach_dyld_info_only_command<MachITypes> Mach_dyld_info_only_command;
    typedef N_Mach::Mach_version_min_command<MachITypes> Mach_version_min_command;
    typedef N_Mach::Mach_source_version_command<MachITypes> Mach_source_version_command;
    typedef N_Mach::Mach_main_command<MachITypes> Mach_main_command;

    typedef N_Mach64::Mach_ppc_thread_state64<MachITypes> Mach_ppc_thread_state64;
    typedef N_Mach64::Mach_AMD64_thread_state<MachITypes> Mach_AMD64_thread_state;
    typedef N_Mach64::Mach_ARM64_thread_state<MachITypes> Mach_ARM64_thread_state;

    static void compileTimeAssertions() {
        BeLePolicy::compileTimeAssertions();
        COMPILE_TIME_ASSERT(sizeof(Addr) == 8)
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
typedef MachClass_Host32::Mach_symtab_command Mach32_symtab_command;
typedef MachClass_Host32::Mach_dysymtab_command Mach32_dysymtab_command;
typedef MachClass_Host32::Mach_segsplit_info_command Mach32_segsplit_info_command;
typedef MachClass_Host32::Mach_routines_command Mach32_routines_command;
typedef MachClass_Host32::Mach_twolevel_hints_command Mach32_twolevel_hints_command;
typedef MachClass_Host32::Mach_linkedit_data_command Mach32_linkedit_data_command;
typedef MachClass_Host32::Mach_uuid_command Mach32_uuid_command;
typedef MachClass_Host32::Mach_main_command Mach32_main_command;
typedef MachClass_Host32::Mach_load_dylib_command Mach32_load_dylib_command;
typedef MachClass_Host32::Mach_dylib Mach32_dylib;
typedef MachClass_Host32::Mach_load_dylinker_command Mach32_load_dylinker_command;
typedef MachClass_Host32::Mach_dyld_info_only_command Mach32_dyld_info_only_command;
typedef MachClass_Host32::Mach_version_min_command Mach32_version_min_command;
typedef MachClass_Host32::Mach_source_version_command Mach32_source_version_command;

typedef MachClass_Host64::Mach_segment_command Mach64_segment_command;
typedef MachClass_Host64::Mach_section_command Mach64_section_command;
typedef MachClass_Host64::Mach_symtab_command Mach64_symtab_command;
typedef MachClass_Host64::Mach_dysymtab_command Mach64_dysymtab_command;
typedef MachClass_Host64::Mach_segsplit_info_command Mach64_segsplit_info_command;
typedef MachClass_Host64::Mach_routines_command Mach64_routines_command;
typedef MachClass_Host64::Mach_twolevel_hints_command Mach64_twolevel_hints_command;
typedef MachClass_Host64::Mach_linkedit_data_command Mach64_linkedit_data_command;
typedef MachClass_Host64::Mach_uuid_command Mach64_uuid_command;
typedef MachClass_Host64::Mach_main_command Mach64_main_command;
typedef MachClass_Host64::Mach_load_dylib_command Mach64_load_dylib_command;
typedef MachClass_Host64::Mach_dylib Mach64_dylib;
typedef MachClass_Host64::Mach_load_dylinker_command Mach64_load_dylinker_command;
typedef MachClass_Host64::Mach_dyld_info_only_command Mach64_dyld_info_only_command;
typedef MachClass_Host64::Mach_version_min_command Mach64_version_min_command;
typedef MachClass_Host64::Mach_source_version_command Mach64_source_version_command;

typedef MachClass_BE32::Mach_segment_command   MachBE32_segment_command;
typedef MachClass_BE32::Mach_section_command   MachBE32_section_command;
typedef MachClass_BE32::Mach_symtab_command   MachBE32_symtab_command;
typedef MachClass_BE32::Mach_dysymtab_command   MachBE32_dysymtab_command;
typedef MachClass_BE32::Mach_segsplit_info_command   MachBE32_segsplit_info_command;
typedef MachClass_BE32::Mach_routines_command   MachBE32_routines_command;
typedef MachClass_BE32::Mach_twolevel_hints_command   MachBE32_twolevel_hints_command;
typedef MachClass_BE32::Mach_linkedit_data_command   MachBE32_linkedit_data_command;
typedef MachClass_BE32::Mach_uuid_command   MachBE32_uuid_command;
typedef MachClass_BE32::Mach_main_command MachBE32_main_command;
typedef MachClass_BE32::Mach_load_dylib_command MachBE32_load_dylib_command;
typedef MachClass_BE32::Mach_dylib MachBE32_dylib;
typedef MachClass_BE32::Mach_load_dylinker_command MachBE32_load_dylinker_command;
typedef MachClass_BE32::Mach_dyld_info_only_command MachBE32_dyld_info_only_command;
typedef MachClass_BE32::Mach_version_min_command MachBE32_version_min_command;
typedef MachClass_BE32::Mach_source_version_command MachBE32_source_version_command;

typedef MachClass_BE64::Mach_segment_command   MachBE64_segment_command;
typedef MachClass_BE64::Mach_section_command   MachBE64_section_command;
typedef MachClass_BE64::Mach_symtab_command   MachBE64_symtab_command;
typedef MachClass_BE64::Mach_dysymtab_command   MachBE64_dysymtab_command;
typedef MachClass_BE64::Mach_segsplit_info_command   MachBE64_segsplit_info_command;
typedef MachClass_BE64::Mach_routines_command   MachBE64_routines_command;
typedef MachClass_BE64::Mach_twolevel_hints_command   MachBE64_twolevel_hints_command;
typedef MachClass_BE64::Mach_linkedit_data_command   MachBE64_linkedit_data_command;
typedef MachClass_BE64::Mach_uuid_command   MachBE64_uuid_command;
typedef MachClass_BE64::Mach_main_command MachBE64_main_command;
typedef MachClass_BE64::Mach_load_dylib_command MachBE64_load_dylib_command;
typedef MachClass_BE64::Mach_dylib MachBE64_dylib;
typedef MachClass_BE64::Mach_load_dylinker_command MachBE64_load_dylinker_command;
typedef MachClass_BE64::Mach_dyld_info_only_command MachBE64_dyld_info_only_command;
typedef MachClass_BE64::Mach_version_min_command MachBE64_version_min_command;
typedef MachClass_BE64::Mach_source_version_command MachBE64_source_version_command;

typedef MachClass_LE32::Mach_segment_command   MachLE32_segment_command;
typedef MachClass_LE32::Mach_section_command   MachLE32_section_command;
typedef MachClass_LE32::Mach_symtab_command   MachLE32_symtab_command;
typedef MachClass_LE32::Mach_dysymtab_command   MachLE32_dysymtab_command;
typedef MachClass_LE32::Mach_segsplit_info_command   MachLE32_segsplit_info_command;
typedef MachClass_LE32::Mach_routines_command   MachLE32_routines_command;
typedef MachClass_LE32::Mach_twolevel_hints_command   MachLE32_twolevel_hints_command;
typedef MachClass_LE32::Mach_linkedit_data_command   MachLE32_linkedit_data_command;
typedef MachClass_LE32::Mach_uuid_command   MachLE32_uuid_command;
typedef MachClass_LE32::Mach_main_command  MachLE32_main_command;
typedef MachClass_LE32::Mach_load_dylib_command  MachLE32_load_dylib_command;
typedef MachClass_LE32::Mach_dylib  MachLE32_dylib;
typedef MachClass_LE32::Mach_load_dylinker_command  MachLE32_load_dylinker_command;
typedef MachClass_LE32::Mach_dyld_info_only_command  MachLE32_dyld_info_only_command;
typedef MachClass_LE32::Mach_version_min_command  MachLE32_version_min_command;
typedef MachClass_LE32::Mach_source_version_command MachLE32_source_version_command;

typedef MachClass_LE64::Mach_segment_command   MachLE64_segment_command;
typedef MachClass_LE64::Mach_section_command   MachLE64_section_command;
typedef MachClass_LE64::Mach_symtab_command   MachLE64_symtab_command;
typedef MachClass_LE64::Mach_dysymtab_command   MachLE64_dysymtab_command;
typedef MachClass_LE64::Mach_segsplit_info_command   MachLE64_segsplit_info_command;
typedef MachClass_LE64::Mach_routines_command   MachLE64_routines_command;
typedef MachClass_LE64::Mach_twolevel_hints_command   MachLE64_twolevel_hints_command;
typedef MachClass_LE64::Mach_linkedit_data_command   MachLE64_linkedit_data_command;
typedef MachClass_LE64::Mach_uuid_command   MachLE64_uuid_command;
typedef MachClass_LE64::Mach_main_command MachLE64_main_command;
typedef MachClass_LE64::Mach_load_dylib_command MachLE64_load_dylib_command;
typedef MachClass_LE64::Mach_dylib MachLE64_dylib;
typedef MachClass_LE64::Mach_load_dylinker_command MachLE64_load_dylinker_command;
typedef MachClass_LE64::Mach_dyld_info_only_command MachLE64_dyld_info_only_command;
typedef MachClass_LE64::Mach_version_min_command MachLE64_version_min_command;
typedef MachClass_LE64::Mach_source_version_command MachLE64_source_version_command;

typedef MachClass_BE32::Mach_ppc_thread_state  Mach_ppc_thread_state;
typedef MachClass_BE64::Mach_ppc_thread_state64  Mach_ppc_thread_state64;
typedef MachClass_LE32::Mach_i386_thread_state Mach_i386_thread_state;
typedef MachClass_LE64::Mach_AMD64_thread_state  Mach_AMD64_thread_state;
typedef MachClass_LE64::Mach_ARM64_thread_state  Mach_ARM64_thread_state;
typedef MachClass_LE32::Mach_ARM_thread_state  Mach_ARM_thread_state;
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
    typedef typename MachClass::Mach_command Mach_command;
    typedef typename MachClass::Mach_segment_command Mach_segment_command;
    typedef typename MachClass::Mach_section_command Mach_section_command;
    typedef typename MachClass::Mach_symtab_command Mach_symtab_command;
    typedef typename MachClass::Mach_dysymtab_command Mach_dysymtab_command;
    typedef typename MachClass::Mach_segsplit_info_command Mach_segsplit_info_command;
    typedef typename MachClass::Mach_routines_command Mach_routines_command;
    typedef typename MachClass::Mach_twolevel_hints_command Mach_twolevel_hints_command;
    typedef typename MachClass::Mach_linkedit_data_command Mach_linkedit_data_command;
    typedef typename MachClass::Mach_uuid_command Mach_uuid_command;
    typedef typename MachClass::Mach_main_command Mach_main_command;
    typedef typename MachClass::Mach_load_dylib_command Mach_load_dylib_command;
    typedef typename MachClass::Mach_dylib Mach_dylib;
    typedef typename MachClass::Mach_load_dylinker_command Mach_load_dylinker_command;
    typedef typename MachClass::Mach_dyld_info_only_command Mach_dyld_info_only_command;
    typedef typename MachClass::Mach_version_min_command Mach_version_min_command;
    typedef typename MachClass::Mach_source_version_command Mach_source_version_command;

public:
    PackMachBase(InputFile *, unsigned cpuid, unsigned filetype,
        unsigned t_flavor, unsigned ts_word_cnt, unsigned tc_size,
        unsigned page_shift);
    virtual ~PackMachBase();
    virtual int getVersion() const override { return 13; }
    virtual const int *getCompressionMethods(int method, int level) const override;

    // called by the generic pack()
    virtual void pack1(OutputFile *, Filter &) override;  // generate executable header
    virtual int  pack2(OutputFile *, Filter &) override;  // append compressed data
    virtual off_t pack3(OutputFile *, Filter &) override /*= 0*/;  // append loader
    virtual void pack4(OutputFile *, Filter &) override /*= 0*/;  // append PackHeader

    virtual void pack4dylib(OutputFile *, Filter &, Addr init_address);

    virtual int  threado_size() const = 0;
    virtual void threado_setPC(upx_uint64_t pc) = 0;
    virtual void threado_rewrite(OutputFile *) = 0;
    virtual void threado_write(OutputFile *) = 0;
    virtual void pack1_setup_threado(OutputFile *const fo) = 0;
    virtual void unpack(OutputFile *fo) override;

    virtual bool canPack() override;
    virtual int canUnpack() override;
    virtual upx_uint64_t get_mod_init_func(Mach_segment_command const *segptr);
    virtual unsigned find_SEGMENT_gap(unsigned const k, unsigned pos_eof);

protected:
    virtual void patchLoader() override;
    virtual void patchLoaderChecksum() override;
    virtual void updateLoader(OutputFile *) override;
    virtual void buildLoader(const Filter *ft) override;
    virtual void buildMachLoader(
        upx_byte const *const proto,
        unsigned        const szproto,
        upx_byte const *const fold,
        unsigned        const szfold,
        Filter const *ft );
    virtual void defineSymbols(Filter const *);
    virtual void addStubEntrySections(Filter const *);

    static int __acc_cdecl_qsort compare_segment_command(void const *aa, void const *bb);

    virtual upx_uint64_t threadc_getPC(void /*MachThreadCommand*/ const *) = 0;

    upx_uint64_t entryVMA;
    upx_uint64_t my_page_size;
    upx_uint64_t my_page_mask;
    unsigned my_cputype;
    unsigned my_cpusubtype;
    unsigned my_filetype;
    unsigned my_thread_flavor;
    unsigned my_thread_state_word_count;
    unsigned my_thread_command_size;

    unsigned  n_segment;
    unsigned sz_segment;
    unsigned sz_mach_headers;
    unsigned sz_stub_entry;
    unsigned sz_stub_fold;
    unsigned sz_stub_main;
    upx_byte const *stub_entry;
    upx_byte const *stub_fold;
    upx_byte const *stub_main;

    MemBuffer rawmseg_buf;  // Mach_segment_command[];
    Mach_segment_command *rawmseg;  // as input, with sections

    MemBuffer msegcmd_buf;  // Mach_segment_command[];
    Mach_segment_command *msegcmd;  // LC_SEGMENT first, without sections

    unsigned o__mod_init_func;  // file offset to __DATA.__mod_init_func Mach_section_command
    upx_uint64_t prev_mod_init_func;
    upx_uint64_t pagezero_vmsize;
    upx_uint64_t vma_max;  // max over (.vmsize + .vmaddr)
    Mach_header mhdri;

    Mach_header mhdro;
    Mach_segment_command segZERO;
    Mach_segment_command segXHDR;  // location to put eXtra headers
    Mach_section_command secXHDR;
    Mach_segment_command segTEXT;
    Mach_section_command secTEXT;
    Mach_segment_command segLINK;
    Mach_linkedit_data_command linkitem;
    Mach_uuid_command cmdUUID;  // copied from input, then incremented
    Mach_source_version_command cmdSRCVER;  // copied from input
    Mach_version_min_command cmdVERMIN;  // copied from input

    __packed_struct(b_info)     // 12-byte header before each compressed block
        TE32 sz_unc;  // uncompressed_size
        TE32 sz_cpr;  //   compressed_size
        unsigned char b_method; // compression algorithm
        unsigned char b_ftid;   // filter id
        unsigned char b_cto8;   // filter parameter
        unsigned char b_segseq; // LC_SEGMENT ordinal
    __packed_struct_end()

    __packed_struct(l_info)     // 12-byte trailer in header for loader
        TE32 l_checksum;
        LE32 l_magic;
        TE16 l_lsize;
        unsigned char l_version;
        unsigned char l_format;
    __packed_struct_end()

    __packed_struct(p_info)     // 12-byte packed program header
        TE32 p_progid;
        TE32 p_filesize;
        TE32 p_blocksize;
    __packed_struct_end()

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
    PackMachPPC32(InputFile *f);

    virtual int getFormat() const override { return UPX_F_MACH_PPC32; }
    virtual const char *getName() const override { return "macho/ppc32"; }
    virtual const char *getFullName(const options_t *) const override { return "powerpc-darwin.macho"; }

protected:
    virtual const int *getFilters() const override;

    virtual void pack1_setup_threado(OutputFile *const fo) override;
    virtual Linker* newLinker() const override;
    virtual void addStubEntrySections(Filter const *) override;

    __packed_struct(Mach_thread_command)
        TE32 cmd;            /* LC_THREAD or  LC_UNIXTHREAD */
        TE32 cmdsize;        /* total size of this command */
        TE32 flavor;
        TE32 count;          /* sizeof(following_thread_state)/4 */
        Mach_ppc_thread_state state;
    #define WANT_MACH_THREAD_ENUM 1
    #include "p_mach_enum.h"
    __packed_struct_end()

    Mach_thread_command threado;
    int  threado_size() const override { return sizeof(threado); }
    void threado_setPC(upx_uint64_t pc) override {
        memset(&threado, 0, sizeof(threado));
        threado.cmd = Mach_segment_command::LC_UNIXTHREAD;
        threado.cmdsize = sizeof(threado);
        threado.flavor = my_thread_flavor;
        threado.count =  my_thread_state_word_count;
        threado.state.srr0 = pc;
    }
    void threado_rewrite(OutputFile *fo) override { fo->rewrite(&threado, sizeof(threado)); }
    void   threado_write(OutputFile *fo) override {   fo->write(&threado, sizeof(threado)); }

    upx_uint64_t threadc_getPC(void const *ptr) override {
        Mach_thread_command const *tc = (Mach_thread_command const *)ptr;
        if (tc->cmd!=Mach_segment_command::LC_UNIXTHREAD
        ||  tc->cmdsize!=sizeof(threado)
        ||  tc->flavor!=my_thread_flavor
        ||  tc->count!=my_thread_state_word_count) {
            return ~0ull;
        }
        return tc->state.srr0;
    }
};

class PackMachPPC64 : public PackMachBase<MachClass_BE64>
{
    typedef PackMachBase<MachClass_BE64> super;

public:
    PackMachPPC64(InputFile *f);

    virtual int getFormat() const override { return UPX_F_MACH_PPC64; }
    virtual const char *getName() const override { return "macho/ppc64"; }
    virtual const char *getFullName(const options_t *) const override { return "powerpc64-darwin.macho"; }

protected:
    virtual const int *getFilters() const override;

    virtual void pack1_setup_threado(OutputFile *const fo) override;
    virtual Linker* newLinker() const override;

    __packed_struct(Mach_thread_command)
        BE32 cmd;            /* LC_THREAD or  LC_UNIXTHREAD */
        BE32 cmdsize;        /* total size of this command */
        BE32 flavor;
        BE32 count;          /* sizeof(following_thread_state)/4 */
        Mach_ppc_thread_state64 state64;
    #define WANT_MACH_THREAD_ENUM 1
    #include "p_mach_enum.h"
    __packed_struct_end()

    Mach_thread_command threado;
    int threado_size() const override { return sizeof(threado); }
    void threado_setPC(upx_uint64_t pc) override {
        memset(&threado, 0, sizeof(threado));
        threado.cmd = Mach_segment_command::LC_UNIXTHREAD;
        threado.cmdsize = sizeof(threado);
        threado.flavor = my_thread_flavor;
        threado.count =  my_thread_state_word_count;
        threado.state64.srr0 = pc;
    }
    void threado_rewrite(OutputFile *fo) override { fo->rewrite(&threado, sizeof(threado)); }
    void   threado_write(OutputFile *fo) override {   fo->write(&threado, sizeof(threado)); }

    upx_uint64_t threadc_getPC(void const *ptr)  override{
        Mach_thread_command const *tc = (Mach_thread_command const *)ptr;
        if (tc->cmd!=Mach_segment_command::LC_UNIXTHREAD
        ||  tc->cmdsize!=sizeof(threado)
        ||  tc->flavor!=my_thread_flavor
        ||  tc->count!=my_thread_state_word_count) {
            return ~0ull;
        }
        return tc->state64.srr0;
    }
};

class PackDylibPPC32 : public PackMachPPC32
{
    typedef PackMachPPC32 super;

public:
    PackDylibPPC32(InputFile *f);

    virtual int getFormat() const override { return UPX_F_DYLIB_PPC32; }
    virtual const char *getName() const override { return "dylib/ppc32"; }
    virtual const char *getFullName(const options_t *) const override { return "powerpc-darwin.dylib"; }
protected:
    virtual off_t pack3(OutputFile *, Filter &) override;  // append loader
    virtual void pack4(OutputFile *, Filter &) override;  // append PackHeader
};

class PackDylibPPC64 : public PackMachPPC64
{
    typedef PackMachPPC64 super;

public:
    PackDylibPPC64(InputFile *f);

    virtual int getFormat() const override { return UPX_F_DYLIB_PPC64; }
    virtual const char *getName() const override { return "dylib/ppc64"; }
    virtual const char *getFullName(const options_t *) const override { return "powerpc64-darwin.dylib"; }
protected:
    virtual off_t pack3(OutputFile *, Filter &) override;  // append loader
    virtual void pack4(OutputFile *, Filter &) override;  // append PackHeader
};

class PackMachI386 : public PackMachBase<MachClass_LE32>
{
    typedef PackMachBase<MachClass_LE32> super;

public:
    PackMachI386(InputFile *f);

    virtual int getFormat() const override { return UPX_F_MACH_i386; }
    virtual const char *getName() const override { return "macho/i386"; }
    virtual const char *getFullName(const options_t *) const override { return "i386-darwin.macho"; }
protected:
    virtual const int *getFilters() const override;

    virtual void pack1_setup_threado(OutputFile *const fo) override;
    virtual Linker* newLinker() const override;
    virtual void addStubEntrySections(Filter const *) override;

    __packed_struct(Mach_thread_command)
        LE32 cmd;            /* LC_THREAD or  LC_UNIXTHREAD */
        LE32 cmdsize;        /* total size of this command */
        LE32 flavor;
        LE32 count;          /* sizeof(following_thread_state)/4 */
        Mach_i386_thread_state state;
    #define WANT_MACH_THREAD_ENUM 1
    #include "p_mach_enum.h"
    __packed_struct_end()

    Mach_thread_command threado;
    int threado_size() const override { return sizeof(threado); }
    void threado_setPC(upx_uint64_t pc) override {
        memset(&threado, 0, sizeof(threado));
        threado.cmd = Mach_segment_command::LC_UNIXTHREAD;
        threado.cmdsize = sizeof(threado);
        threado.flavor = my_thread_flavor;
        threado.count =  my_thread_state_word_count;
        threado.state.eip = pc;
    }
    void threado_rewrite(OutputFile *fo) override { fo->rewrite(&threado, sizeof(threado)); }
    void   threado_write(OutputFile *fo) override {   fo->write(&threado, sizeof(threado)); }

    upx_uint64_t threadc_getPC(void const *ptr)  override{
        Mach_thread_command const *tc = (Mach_thread_command const *)ptr;
        if (tc->cmd!=Mach_segment_command::LC_UNIXTHREAD
        ||  tc->cmdsize!=sizeof(threado)
        ||  tc->flavor!=my_thread_flavor
        ||  tc->count!=my_thread_state_word_count) {
            return ~0ull;
        }
        return tc->state.eip;
    }
};

class PackDylibI386 : public PackMachI386
{
    typedef PackMachI386 super;

public:
    PackDylibI386(InputFile *f);

    virtual int getFormat() const override { return UPX_F_DYLIB_i386; }
    virtual const char *getName() const override { return "dylib/i386"; }
    virtual const char *getFullName(const options_t *) const override { return "i386-darwin.dylib"; }
protected:
    virtual off_t pack3(OutputFile *, Filter &) override;  // append loader
    virtual void pack4(OutputFile *, Filter &) override;  // append PackHeader
};

class PackMachAMD64 : public PackMachBase<MachClass_LE64>
{
    typedef PackMachBase<MachClass_LE64> super;

public:
    PackMachAMD64(InputFile *f);

    virtual int getFormat() const override { return UPX_F_MACH_AMD64; }
    virtual const char *getName() const override { return "macho/amd64"; }
    virtual const char *getFullName(const options_t *) const override { return "amd64-darwin.macho"; }
protected:
    virtual const int *getFilters() const override;

    virtual void pack1_setup_threado(OutputFile *const fo) override;
    virtual Linker* newLinker() const override;
    virtual void addStubEntrySections(Filter const *) override;

    __packed_struct(Mach_thread_command)
        typedef MachITypes::Word Word;
        Word cmd;            /* LC_THREAD or  LC_UNIXTHREAD */
        Word cmdsize;        /* total size of this command */
        Word flavor;
        Word count;          /* sizeof(following_thread_state)/4 */
        Mach_AMD64_thread_state state;
    #define WANT_MACH_THREAD_ENUM 1
    #include "p_mach_enum.h"
    __packed_struct_end()

    Mach_thread_command threado;
    int threado_size() const override { return sizeof(threado); }
    void threado_setPC(upx_uint64_t pc) override {
        memset(&threado, 0, sizeof(threado));
        threado.cmd = Mach_segment_command::LC_UNIXTHREAD;
        threado.cmdsize = sizeof(threado);
        threado.flavor = my_thread_flavor;
        threado.count =  my_thread_state_word_count;
        threado.state.rip = pc;
    }
    void threado_rewrite(OutputFile *fo) override { fo->rewrite(&threado, sizeof(threado)); }
    void   threado_write(OutputFile *fo) override {   fo->write(&threado, sizeof(threado)); }

    upx_uint64_t threadc_getPC(void const *ptr) override {
        Mach_thread_command const *tc = (Mach_thread_command const *)ptr;
        if (tc->cmd!=Mach_segment_command::LC_UNIXTHREAD
        ||  tc->cmdsize!=sizeof(threado)
        ||  tc->flavor!=my_thread_flavor
        ||  tc->count!=my_thread_state_word_count) {
            return ~0ull;
        }
        return tc->state.rip;
    }
};

class PackDylibAMD64 : public PackMachAMD64
{
    typedef PackMachAMD64 super;

public:
    PackDylibAMD64(InputFile *f);

    virtual int getFormat() const override { return UPX_F_DYLIB_AMD64; }
    virtual const char *getName() const override { return "dylib/amd64"; }
    virtual const char *getFullName(const options_t *) const override { return "amd64-darwin.dylib"; }
protected:
    virtual off_t pack3(OutputFile *, Filter &) override;  // append loader
    virtual void pack4(OutputFile *, Filter &) override;  // append PackHeader
};

class PackMachARMEL : public PackMachBase<MachClass_LE32>
{
    typedef PackMachBase<MachClass_LE32> super;

public:
    PackMachARMEL(InputFile *f);

    virtual int getFormat() const override { return UPX_F_MACH_ARMEL; }
    virtual const char *getName() const override { return "macho/arm"; }
    virtual const char *getFullName(const options_t *) const override { return "arm-darwin.macho"; }
protected:
    virtual const int *getCompressionMethods(int method, int level) const override;
    virtual const int *getFilters() const override;

    virtual void pack1_setup_threado(OutputFile *const fo) override;
    virtual Linker* newLinker() const override;
    virtual void addStubEntrySections(Filter const *) override;

    __packed_struct(Mach_thread_command)
        LE32 cmd;            /* LC_THREAD or  LC_UNIXTHREAD */
        LE32 cmdsize;        /* total size of this command */
        LE32 flavor;
        LE32 count;          /* sizeof(following_thread_state)/4 */
        Mach_ARM_thread_state state;
    #define WANT_MACH_THREAD_ENUM 1
    #include "p_mach_enum.h"
    __packed_struct_end()

    Mach_thread_command threado;
    int threado_size() const override { return sizeof(threado); }
    void threado_setPC(upx_uint64_t pc) override {
        memset(&threado, 0, sizeof(threado));
        threado.cmd = Mach_segment_command::LC_UNIXTHREAD;
        threado.cmdsize = sizeof(threado);
        threado.flavor = my_thread_flavor;
        threado.count =  my_thread_state_word_count;
        threado.state.pc = pc;
    }
    void threado_rewrite(OutputFile *fo) override { fo->rewrite(&threado, sizeof(threado)); }
    void   threado_write(OutputFile *fo) override { return   fo->write(&threado, sizeof(threado)); }

    upx_uint64_t threadc_getPC(void const *ptr) override {
        Mach_thread_command const *tc = (Mach_thread_command const *)ptr;
        if (tc->cmd!=Mach_segment_command::LC_UNIXTHREAD
        ||  tc->cmdsize!=sizeof(threado)
        ||  tc->flavor!=my_thread_flavor
        ||  tc->count!=my_thread_state_word_count) {
            return ~0ull;
        }
        return tc->state.pc;
    }
};

class PackMachARM64EL : public PackMachBase<MachClass_LE64>
{
    typedef PackMachBase<MachClass_LE64> super;

public:
    PackMachARM64EL(InputFile *f);

    virtual int getFormat() const override { return UPX_F_MACH_ARM64EL; }
    virtual const char *getName() const override { return "macho/arm64"; }
    virtual const char *getFullName(const options_t *) const override { return "arm64-darwin.macho"; }
protected:
    virtual const int *getFilters() const override;

    virtual void pack1_setup_threado(OutputFile *const fo) override;
    virtual Linker* newLinker() const override;
    virtual void addStubEntrySections(Filter const *) override;

    __packed_struct(Mach_thread_command)
        LE32 cmd;            /* LC_THREAD or  LC_UNIXTHREAD */
        LE32 cmdsize;        /* total size of this command */
        LE32 flavor;
        LE32 count;          /* sizeof(following_thread_state)/4 */
        Mach_ARM64_thread_state state;
    #define WANT_MACH_THREAD_ENUM 1
    #include "p_mach_enum.h"
    __packed_struct_end()

    Mach_thread_command threado;
    int threado_size() const override { return sizeof(threado); }
    void threado_setPC(upx_uint64_t pc) override {
        memset(&threado, 0, sizeof(threado));
        threado.cmd = Mach_segment_command::LC_UNIXTHREAD;
        threado.cmdsize = sizeof(threado);
        threado.flavor = my_thread_flavor;
        threado.count =  my_thread_state_word_count;
        threado.state.pc = pc;
    }
    void threado_rewrite(OutputFile *fo) override { fo->rewrite(&threado, sizeof(threado)); }
    void   threado_write(OutputFile *fo) override {   fo->write(&threado, sizeof(threado)); }

    upx_uint64_t threadc_getPC(void const *ptr) override {
        Mach_thread_command const *tc = (Mach_thread_command const *)ptr;
        if (tc->cmd!=Mach_segment_command::LC_UNIXTHREAD
        ||  tc->cmdsize!=sizeof(threado)
        ||  tc->flavor!=my_thread_flavor
        ||  tc->count!=my_thread_state_word_count) {
            return ~0ull;
        }
        return tc->state.pc;
    }
};

class PackMachFat : public Packer
{
    typedef Packer super;
public:
    PackMachFat(InputFile *f);
    virtual ~PackMachFat();

    virtual int getVersion() const override { return 13; }
    virtual int getFormat() const override { return UPX_F_MACH_FAT; }
    virtual const char *getName() const override { return "macho/fat"; }
    virtual const char *getFullName(const options_t *) const override { return "fat-darwin.macho"; }
    virtual const int *getCompressionMethods(int method, int level) const override;
    virtual const int *getFilters() const override;

protected:
    // implementation
    virtual unsigned check_fat_head();  // number of architectures
    virtual void pack(OutputFile *fo) override;
    virtual void unpack(OutputFile *fo) override;
    virtual void list() override;

public:
    virtual bool canPack() override;
    virtual int canUnpack() override;

protected:
    // loader core
    virtual void buildLoader(const Filter *ft) override;
    virtual Linker* newLinker() const override;

    enum { N_FAT_ARCH = 5 };
protected:
    __packed_struct(Fat_head)
        struct Mach_fat_header fat;
        struct Mach_fat_arch arch[N_FAT_ARCH];
    __packed_struct_end()

    Fat_head fat_head;

    // UI handler
    UiPacker *uip;

    // linker
    Linker *linker;
#define WANT_MACH_HEADER_ENUM 1
#include "p_mach_enum.h"
};

// Alignment and sizeof are independent of endianness,
// so all the above template classes just complicate.
// Besides, we use them only to check for valid Macho headers.
// (Fie on fuzzers!)

struct dyld_info_command {
    upx_uint32_t cmd;
    upx_uint32_t cmdsize;
    upx_uint32_t rebase_off;
    upx_uint32_t rebase_size;
    upx_uint32_t bind_off;
    upx_uint32_t bind_size;
    upx_uint32_t weak_bind_off;
    upx_uint32_t weak_bind_size;
    upx_uint32_t lazy_bind_off;
    upx_uint32_t lazy_bind_size;
    upx_uint32_t export_off;
    upx_uint32_t export_size;
};
union lc_str {
    upx_uint32_t offset;
};

struct dylib {
    union lc_str name;
    upx_uint32_t timestamp;
    upx_uint32_t current_version;
    upx_uint32_t compatibility_version;
};
struct dylib_command {
    upx_uint32_t cmd;
    upx_uint32_t cmdsize;
    struct dylib dylib;
};
struct dylinker_command {
    upx_uint32_t cmd;
    upx_uint32_t cmdsize;
    union lc_str name;
};
struct encryption_info_command {
    upx_uint32_t cmd;
    upx_uint32_t cmdsize;
    upx_uint32_t cryptoff;
    upx_uint32_t cryptsize;
    upx_uint32_t cryptid;
};
struct encryption_info_command_64 {
    upx_uint32_t cmd;
    upx_uint32_t cmdsize;
    upx_uint32_t cryptoff;
    upx_uint32_t cryptsize;
    upx_uint32_t cryptid;
    upx_uint32_t pad;
};
struct entry_point_command {
    upx_uint32_t cmd;
    upx_uint32_t cmdsize;
    upx_uint64_t entryoff;
    upx_uint64_t stacksize;
};
struct linkedit_data_command {
    upx_uint32_t cmd;
    upx_uint32_t cmdsize;
    upx_uint32_t dataoff;
    upx_uint32_t datasize;
};
struct rpath_command {
    upx_uint32_t cmd;
    upx_uint32_t cmdsize;
    union lc_str path;
};
struct routines_command {
    upx_uint32_t cmd;
    upx_uint32_t cmdsize;
    upx_uint32_t init_address;
    upx_uint32_t init_module;
    upx_uint32_t reserved1;
    upx_uint32_t reserved2;
    upx_uint32_t reserved3;
    upx_uint32_t reserved4;
    upx_uint32_t reserved5;
    upx_uint32_t reserved6;
};
struct routines_command_64 {
    upx_uint32_t cmd;
    upx_uint32_t cmdsize;
    upx_uint64_t init_address;
    upx_uint64_t init_module;
    upx_uint64_t reserved1;
    upx_uint64_t reserved2;
    upx_uint64_t reserved3;
    upx_uint64_t reserved4;
    upx_uint64_t reserved5;
    upx_uint64_t reserved6;
};
struct uuid_command {
    upx_uint32_t cmd;
    upx_uint32_t cmdsize;
    upx_uint8_t uuid[16];
};
struct version_min_command {
    upx_uint32_t cmd;
    upx_uint32_t cmdsize;
    upx_uint32_t version;
    upx_uint32_t sdk;
};

#endif /* already included */

/* vim:set ts=4 sw=4 et: */
