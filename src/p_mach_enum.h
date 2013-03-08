/* p_mach_enum.h --

   This file is part of the UPX executable compressor.

   Copyright (C) 2007-2013 John F. Reiser
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

   John F. Reiser
   <jreiser@users.sourceforge.net>
 */


/*************************************************************************
// Use the preprocessor to work around
//   - that the types embedding these enums have to be PODs, and
//     deriving from an empty base class (which is the usual C++ way
//     of "importing" enums) does not yield a POD any more
//   - that older compilers do not correctly perform EBCO
**************************************************************************/

#ifdef WANT_MACH_HEADER_ENUM  /*{*/
#undef WANT_MACH_HEADER_ENUM
    enum { // magic
        MH_MAGIC = 0xfeedface
    };
    enum { // cputype
        CPU_TYPE_I386      =          7,
        CPU_TYPE_X86_64    = 0x01000007,
        CPU_TYPE_ARM       =         12,
        CPU_TYPE_POWERPC   = 0x00000012,
        CPU_TYPE_POWERPC64 = 0x01000012
    };
    enum { // cpusubtype
        CPU_SUBTYPE_ARM_ALL = 0,
        CPU_SUBTYPE_ARM_V4T = 5,
        CPU_SUBTYPE_ARM_V6  = 6
    };
    enum { // filetype
        MH_EXECUTE = 2,
        MH_DYLIB   = 6,
        MH_BUNDLE  = 8      /* dynamically bound bundle file */
    };
    enum { // flags
        MH_NOUNDEFS = 1,
        MH_DYLDLINK = 4     /* code signing demands this */
    };
#endif  /*}*/

#ifdef WANT_MACH_SEGMENT_ENUM  /*{*/
#undef WANT_MACH_SEGMENT_ENUM
    enum { // cmd
        LC_SEGMENT       = 0x1,
        LC_SYMTAB        = 0x2,
        LC_THREAD        = 0x4,
        LC_UNIXTHREAD    = 0x5,
        LC_DYSYMTAB      = 0xb,
        LC_LOAD_DYLIB    = 0xc,
        LC_ID_DYLIB      = 0xd,
        LC_LOAD_DYLINKER = 0xe,
        LC_ROUTINES      = 0x11,
        LC_TWOLEVEL_HINTS= 0x16,
        LC_SEGMENT_64    = 0x19,
        LC_ROUTINES_64   = 0x1a,
        LC_UUID          = 0x1b,
        LC_RPATH         = 0x1c,
        LC_CODE_SIGNATURE = 0x1d,
        LC_SEGMENT_SPLIT_INFO = 0x1e,
        LC_REEXPORT_DYLIB = 0x1f,
        LC_LAZY_LOAD_DYLIB= 0X20,
        LC_ENCRYPTION_INFO= 0X21,
        LC_DYLD_INFO      = 0X22,  // compressed dyld information (10.6.x)
        LC_REQ_DYLD      = 0x80000000  // OR'ed ==> must not ignore
    };
    enum { // maxprot
        VM_PROT_READ = 1,
        VM_PROT_WRITE = 2,
        VM_PROT_EXECUTE = 4
    };
#endif  /*}*/

#ifdef WANT_MACH_SECTION_ENUM  /*{*/
#undef WANT_MACH_SECTION_ENUM
    enum { // section type  (low byte only)
        S_REGULAR = 0,
        S_ZEROFILL,
        S_CSTRING_LITERALS,
        S_4BYTE_LITERALS,
        S_8BYTE_LITERALS,
        S_LITERAL_POINTERS,
        S_NON_LAZY_SYMBOL_POINTERS,  // sectname __nl_symbol_ptr
        S_LAZY_SYMBOL_POINTERS,      // sectname __la_symbol_ptr
        S_SYMBOL_STUBS,
        S_MOD_INIT_FUNC_POINTERS,    // sectname __mod_init_func
        S_MOD_TERM_FUNC_POINTERS,
        S_COALESCED,
        S_GB_ZEROFILL,
        S_INTERPOSING,
        S_16BYTE_LITERALS,
        S_DTRACE_DOF
    };
    enum { // section flags (high 24 bits)
        S_ATTR_PURE_INSTRUCTIONS = 0x80000000,
        S_ATTR_NO_TOC            = 0x40000000,
        S_ATTR_STRIP_STATIC_SYMS = 0x20000000,
        S_ATTR_NO_DEAD_STRIP     = 0x10000000,
        S_ATTR_LIVE_SUPPORT      = 0x08000000,
        S_ATTR_SELF_MODIFYING_CODE = 0x04000000,
        S_ATTR_DEBUG             = 0x02000000,
        S_ATTR_SOME_INSTRUCTIONS = 0x00000400,
        S_ATTR_EXT_RELOC         = 0x00000200,
        S_ATTR_LOC_RELOC         = 0x00000100
    };
#endif  /*}*/

#ifdef WANT_MACH_THREAD_ENUM  /*{*/
#undef WANT_MACH_THREAD_ENUM
    enum { // thread flavor
        PPC_THREAD_STATE = 1,
        PPC_THREAD_STATE64 = 5,
        x86_THREAD_STATE32 = 1,
        x86_THREAD_STATE64 = 4,
        i386_OLD_THREAD_STATE = -1,
        ARM_THREAD_STATE = 1
    };
#endif  /*}*/

/*
vi:ts=4:et
*/

