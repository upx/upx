/* p_w32pe.h --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2001 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2001 Laszlo Molnar
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
   markus@oberhumer.com      ml1050@cdata.tvnet.hu
 */


#ifndef __UPX_P_W32PE_H
#define __UPX_P_W32PE_H


class PackW32Pe_Interval;
class PackW32Pe_Reloc;
class PackW32Pe_Resource;
class PackW32Pe_Export;


/*************************************************************************
// w32/pe
**************************************************************************/

class PackW32Pe : public Packer
{
    typedef Packer super;

public:
    PackW32Pe(InputFile *f);
    ~PackW32Pe();
    virtual int getVersion() const { return 12; }
    virtual int getFormat() const { return UPX_F_WIN32_PE; }
    virtual const char *getName() const { return isrtm ? "rtm32/pe" : "win32/pe"; }
    virtual const int *getCompressionMethods(int method, int level) const;
    virtual const int *getFilters() const;

    virtual void pack(OutputFile *fo);
    virtual void unpack(OutputFile *fo);

    virtual bool canPack();
    virtual int canUnpack();

    // unpacker capabilities
    virtual bool canUnpackVersion(int version) const
        {  return (version == 12); }

protected:
    virtual int readFileHeader();
    virtual bool testUnpackVersion(int version) const;

    virtual int buildLoader(const Filter *ft);

    unsigned pe_offset;
    bool isrtm;

    unsigned processImports();
    void processImports(unsigned);
    void rebuildImports(upx_byte *&);
    upx_byte *oimport;
    unsigned soimport;
    upx_byte *oimpdlls;
    unsigned soimpdlls;

    void processRelocs();
    void processRelocs(PackW32Pe_Reloc *);
    void rebuildRelocs(upx_byte *&);
    upx_byte *orelocs;
    unsigned sorelocs;
    upx_byte *oxrelocs;
    unsigned soxrelocs;

    void processExports(PackW32Pe_Export *);
    void processExports(PackW32Pe_Export *,unsigned);
    void rebuildExports();
    upx_byte *oexport;
    unsigned soexport;

    void processResources(PackW32Pe_Resource *);
    void processResources(PackW32Pe_Resource *, unsigned);
    void rebuildResources(upx_byte *&);
    upx_byte *oresources;
    unsigned soresources;

    void processTls(PackW32Pe_Interval *);
    void processTls(PackW32Pe_Reloc *,const PackW32Pe_Interval *,unsigned);
    void rebuildTls();
    upx_byte *otls;
    unsigned sotls;

    unsigned stripDebug(unsigned);

    unsigned icondir_offset;
    int icondir_count;

    bool importbyordinal;
    bool kernel32ordinal;
    unsigned tlsindex;
    unsigned rvamin;
    unsigned cimports;              // rva of preprocessed imports
    unsigned crelocs;               // rva of preprocessed fixups
    int big_relocs;

    struct pe_header_t
    {
        // 0x0
        char    _[4];               // pemagic
        LE16    cpu;
        LE16    objects;
        char    __[12];             // timestamp + reserved
        LE16    opthdrsize;
        LE16    flags;
        // optional header
        char    ___[4];             // coffmagic + linkerversion
        LE32    codesize;
        // 0x20
        LE32    datasize;
        LE32    bsssize;
        LE32    entry;
        LE32    codebase;
        // 0x30
        LE32    database;
        // nt specific fields
        LE32    imagebase;
        LE32    objectalign;
        LE32    filealign;          // should set to 0x200 ?
        // 0x40
        char    ____[16];           // versions
        // 0x50
        LE32    imagesize;
        LE32    headersize;
        LE32    chksum;             // should set to 0
        LE16    subsystem;
        LE16    dllflags;
        // 0x60
        char    _____[20];          // stack + heap sizes
        // 0x74
        LE32    ddirsentries;       // usually 16

        struct ddirs_t
        {
            LE32    vaddr;
            LE32    size;
        }       ddirs[16];
    } ih, oh;

    struct pe_section_t
    {
        char    name[8];
        LE32    vsize;
        LE32    vaddr;
        LE32    size;
        LE32    rawdataptr;
        char    _[12];
        LE32    flags;
    } *isection;

    static unsigned virta2objnum (unsigned, pe_section_t *, unsigned);
    unsigned tryremove (unsigned, unsigned);

    enum {
        PEDIR_EXPORT    = 0,
        PEDIR_IMPORT    = 1,
        PEDIR_RESOURCE  = 2,
        PEDIR_EXCEPTION = 3,            // Exception table
        PEDIR_SEC       = 4,            // Certificate table (file pointer)
        PEDIR_RELOC     = 5,
        PEDIR_DEBUG     = 6,
        PEDIR_COPYRIGHT = 7,            // Architecture-specific data
        PEDIR_GLOBALPTR = 8,            // Global pointer
        PEDIR_TLS       = 9,
        PEDIR_LOADCONF  = 10,           // Load Config Table
        PEDIR_BOUNDIM   = 11,
        PEDIR_IAT       = 12,
        PEDIR_DELAYIMP  = 13,           // Delay Import Descriptor
        PEDIR_COMRT     = 14            // Com+ Runtime Header
    };

    enum {
        PEFL_CODE       = 0x20,
        PEFL_DATA       = 0x40,
        PEFL_BSS        = 0x80,
        PEFL_INFO       = 0x200,
        PEFL_EXTRELS    = 0x01000000,   // extended relocations
        PEFL_DISCARD    = 0x02000000,
        PEFL_NOCACHE    = 0x04000000,
        PEFL_NOPAGE     = 0x08000000,
        PEFL_SHARED     = 0x10000000,
        PEFL_EXEC       = 0x20000000,
        PEFL_READ       = 0x40000000,
        PEFL_WRITE      = 0x80000000
    };

    enum {
        RELOCS_STRIPPED = 0x0001,
        EXECUTABLE      = 0x0002,
        LNUM_STRIPPED   = 0x0004,
        LSYMS_STRIPPED  = 0x0008,
        AGGRESSIVE_TRIM = 0x0010,
        TWO_GIGS_AWARE  = 0x0020,
        FLITTLE_ENDIAN  = 0x0080,
        BITS_32_MACHINE = 0x0100,
        DEBUG_STRIPPED  = 0x0200,
        REMOVABLE_SWAP  = 0x0400,
        SYSTEM_PROGRAM  = 0x1000,
        DLL_FLAG        = 0x2000,
        FBIG_ENDIAN     = 0x8000
    };
};


#endif /* already included */


/*
vi:ts=4:et
*/
