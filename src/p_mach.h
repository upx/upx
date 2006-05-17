/* p_mach.h --

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


#ifndef __UPX_P_MACHO_H
#define __UPX_P_MACHO_H

/*************************************************************************
// Mach  Mach Object executable
**************************************************************************/

struct Mach_header {
    BE32 magic;
        enum {
            MH_MAGIC = 0xfeedface
        };
    BE32 cputype;
        enum {
            CPU_TYPE_POWERPC = 18
        };
    BE32 cpysubtype;
    BE32 filetype;
        enum {
            MH_EXECUTE = 2
        };
    BE32 ncmds;
    BE32 sizeofcmds;
    BE32 flags;
        enum {
            MH_NOUNDEFS = 1
        };
}
__attribute_packed;

struct Mach_segment_command {
    BE32 cmd;
        enum {
            LC_SEGMENT       = 0x1,
            LC_THREAD        = 0x4,
            LC_UNIXTHREAD    = 0x5,
            LC_LOAD_DYLINKER = 0xe
        };
    BE32 cmdsize;
    char segname[16];
    BE32 vmaddr;
    BE32 vmsize;
    BE32 fileoff;
    BE32 filesize;
    BE32 maxprot;
        enum {
            VM_PROT_READ = 1,
            VM_PROT_WRITE = 2,
            VM_PROT_EXECUTE = 4
        };
    BE32 initprot;
    BE32 nsects;
    BE32 flags;
}
__attribute_packed;

struct Mach_section {
    char sectname[16];
    char segname[16];
    BE32 addr;   /* memory address */
    BE32 size;   /* size in bytes */
    BE32 offset; /* file offset */
    BE32 align;  /* power of 2 */
    BE32 reloff; /* file offset of relocation entries */
    BE32 nreloc; /* number of relocation entries */
    BE32 flags;  /* section type and attributes */
        enum {
            S_REGULAR = 0,
            S_ZEROFILL,
            S_CSTRING_LITERALS,
            S_4BYTE_LITERALS,
            S_8BYTE_LITERALS,
            S_LITERAL_POINTERS,
            S_NON_LAZY_SYMBOL_POINTERS,
            S_LAZY_SYMBOL_POINTERS,
            S_SYMBOL_STUBS,
            S_MOD_INIT_FUNC_POINTERS,
            S_MOD_TERM_FNC_POINTERS,
            S_COALESCED
        };
        enum {
            S_ATTR_PURE_INSTRUCTIONS = 0x80000000,
            S_ATTR_NO_TOC            = 0x40000000,
            S_ATTR_STRIP_STATIC_SYMS = 0x20000000,
            S_ATTR_SOME_INSTRUCTIONS = 0x00000400,
            S_ATTR_EXT_RELOC         = 0x00000200,
            S_ATTR_LOC_RELOC         = 0x00000100
        };
    BE32 reserved1;
    BE32 reserved2;

}
__attribute_packed;

struct Mach_ppc_thread_state {
    BE32 srr0;      /* Instruction address register (PC; entry addr) */
    BE32 srr1;      /* Machine state register (supervisor) */
    BE32  r0, r1, r2, r3, r4, r5, r6, r7;
    BE32  r8, r9,r10,r11,r12,r13,r14,r15;
    BE32 r16,r17,r18,r19,r20,r21,r22,r23;
    BE32 r24,r25,r26,r27,r28,r29,r30,r31;

    BE32 cr;        /* Condition register */
    BE32 xer;       /* User's integer exception register */
    BE32 lr;        /* Link register */
    BE32 ctr;       /* Count register */
    BE32 mq;        /* MQ register (601 only) */

    BE32 vrsave;    /* Vector Save Register */
};

struct Mach_thread_command {
    BE32 cmd;            /* LC_THREAD or  LC_UNIXTHREAD */
    BE32 cmdsize;        /* total size of this command */
    BE32 flavor;
        enum {
            PPC_THREAD_STATE = 1
        };
    BE32 count;          /* sizeof(following_thread_state)/4 */
        enum {
            PPC_THREAD_STATE_COUNT = sizeof(struct Mach_ppc_thread_state)/4
        };
    struct Mach_ppc_thread_state state;
}
__attribute_packed;


#include "p_unix.h"


class PackMachPPC32 : public PackUnixBe32
{
    typedef PackUnixBe32 super;
public:
    PackMachPPC32(InputFile *f);
    virtual ~PackMachPPC32();
    virtual int getVersion() const { return 13; }
    virtual int getFormat() const { return UPX_F_MACH_PPC32; }
    virtual const char *getName() const { return "Mach/ppc32"; }
    virtual const int *getCompressionMethods(int method, int level) const;
    virtual const int *getFilters() const;

    // called by the generic pack()
    virtual void pack1(OutputFile *, Filter &);  // generate executable header
    virtual void pack2(OutputFile *, Filter &);  // append compressed data
    virtual void pack3(OutputFile *, Filter &);  // append loader
    virtual void pack4(OutputFile *, Filter &);  // append PackHeader

    virtual void unpack(OutputFile *fo);

    virtual bool canPack();
    virtual unsigned find_SEGMENT_gap(unsigned const k);

protected:
    virtual int buildLoader(const Filter *ft);
    virtual void patchLoader();
    virtual void patchLoaderChecksum();
    virtual void updateLoader(OutputFile *);
    virtual int buildMachLoader(
        upx_byte const *const proto,
        unsigned        const szproto,
        upx_byte const *const fold,
        unsigned        const szfold,
        Filter const *ft );

    unsigned  n_segment;
    unsigned sz_segment;
    unsigned sz_mach_headers;
    Mach_segment_command *rawmseg;  // as input, with sections
    Mach_segment_command *msegcmd;  // LC_SEGMENT first, without sections
    Mach_header mhdri;

    Mach_header mhdro;
    Mach_segment_command segcmdo;
    Mach_thread_command threado;
    struct l_info linfo;
};


#endif /* already included */


/*
vi:ts=4:et
*/

