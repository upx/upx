/* pefile.h --

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


#ifndef __UPX_PEFILE_H
#define __UPX_PEFILE_H 1

#include "util/membuffer.h"


/*************************************************************************
// general/pe handling
**************************************************************************/

class PeFile : public Packer
{
    typedef Packer super;
public:
    virtual int getVersion() const override { return 13; }
protected:
    class Interval;
    class Reloc;
    class Resource;
    class Export;
    class ImportLinker;
    struct pe_section_t;

    PeFile(InputFile *f);
    virtual ~PeFile();

    void readSectionHeaders(unsigned objs, unsigned sizeof_ih);
    unsigned readSections(unsigned objs, unsigned usize,
                          unsigned ih_filealign, unsigned ih_datasize);
    void checkHeaderValues(unsigned subsystem, unsigned mask,
                           unsigned ih_entry, unsigned ih_filealign);
    unsigned handleStripRelocs(upx_uint64_t ih_imagebase,
                               upx_uint64_t default_imagebase,
                               LE16 &dllflags);

    virtual bool handleForceOption() = 0;
    virtual void callCompressWithFilters(Filter &, int filter_strategy,
                                         unsigned ih_codebase);
    virtual void defineSymbols(unsigned ncsection, unsigned upxsection,
                               unsigned sizeof_oh, unsigned isize_isplit,
                               unsigned s1addr) = 0;
    virtual void addNewRelocations(Reloc &, unsigned) {}
    void callProcessRelocs(Reloc &rel, unsigned &ic);
    void callProcessResources(Resource &res, unsigned &ic);
    virtual unsigned getProcessImportParam(unsigned) { return 0; }
    virtual void setOhDataBase(const pe_section_t *osection) = 0;
    virtual void setOhHeaderSize(const pe_section_t *osection) = 0;

    template <typename LEXX, typename ht>
    void pack0(OutputFile *fo, ht &ih, ht &oh,
               unsigned subsystem_mask, upx_uint64_t default_imagebase,
               bool last_section_rsrc_only);

    template <typename ht, typename LEXX, typename ord_mask_t>
    void unpack0(OutputFile *fo, const ht &ih, ht &oh,
                 ord_mask_t ord_mask, bool set_oft);

    // unpacker capabilities
    virtual bool canUnpackVersion(int version) const override
        {  return (version >= 12 && version <= 13); }

    int canUnpack0(unsigned max_sections, LE16 &ih_objects,
                   LE32 &ih_entry, unsigned ihsize);

protected:
    virtual int readFileHeader();
    virtual bool testUnpackVersion(int version) const override;
    virtual void readPeHeader() = 0;

    unsigned pe_offset;

    template <typename LEXX, typename ord_mask_t>
    unsigned processImports0(ord_mask_t ord_mask);

    template <typename LEXX, typename ord_mask_t>
    void rebuildImports(SPAN_S(upx_byte) & extrainfo,
                        ord_mask_t ord_mask, bool set_oft);
    virtual unsigned processImports() = 0;
    virtual void processImports2(unsigned, unsigned);
    MemBuffer mb_oimport;
    SPAN_0(upx_byte) oimport = nullptr;
    unsigned soimport;
    upx_byte *oimpdlls;
    unsigned soimpdlls;
    ImportLinker *ilinker;
    virtual const char *kernelDll() const { return "KERNEL32.DLL"; }
    void addKernelImport(const char *);
    virtual void addStubImports();
    upx_uint64_t ilinkerGetAddress(const char *, const char *) const;

    virtual void processRelocs() = 0;
    void processRelocs(Reloc *);
    void rebuildRelocs(SPAN_S(upx_byte) &, unsigned bits,
                       unsigned flags, upx_uint64_t imagebase);
    MemBuffer mb_orelocs;
    SPAN_0(upx_byte) orelocs = nullptr;
    unsigned sorelocs;
    upx_byte *oxrelocs = nullptr;
    unsigned soxrelocs;

    void processExports(Export *);
    void processExports(Export *,unsigned);
    void rebuildExports();
    MemBuffer mb_oexport;
    SPAN_0(upx_byte) oexport = nullptr;
    unsigned soexport;

    void processResources(Resource *);
    void processResources(Resource *, unsigned);
    void rebuildResources(SPAN_S(upx_byte) &, unsigned);
    MemBuffer mb_oresources;
    SPAN_0(upx_byte) oresources = nullptr;
    unsigned soresources;

    template <typename>
    struct tls_traits;
    template <typename LEXX>
    void processTls1(Interval *iv,
                     typename tls_traits<LEXX>::cb_value_t imagebase,
                     unsigned imagesize); // pass 1
    template <typename LEXX>
    void processTls2(Reloc *rel,const Interval *iv,unsigned newaddr,
                     typename tls_traits<LEXX>::cb_value_t imagebase); // pass 2
    virtual void processTls(Interval *iv) = 0;
    virtual void processTls(Reloc *r, const Interval *iv, unsigned a) = 0;

    void rebuildTls();
    MemBuffer mb_otls;
    SPAN_0(upx_byte) otls = nullptr;
    unsigned sotls;
    unsigned tlsindex;
    unsigned tlscb_ptr;
    unsigned tls_handler_offset;
    bool use_tls_callbacks;

    void processLoadConf(Reloc *, const Interval *, unsigned);
    void processLoadConf(Interval *);
    MemBuffer mb_oloadconf;
    upx_byte *oloadconf;
    unsigned soloadconf;

    unsigned stripDebug(unsigned);

    unsigned icondir_offset;
    int icondir_count;

    bool importbyordinal;
    bool kernel32ordinal;
    unsigned rvamin;
    unsigned cimports;              // rva of preprocessed imports
    unsigned crelocs;               // rva of preprocessed fixups
    int big_relocs;

    struct alignas(1) ddirs_t {
        LE32    vaddr;
        LE32    size;
    };
    ddirs_t *iddirs;
    ddirs_t *oddirs;

    struct alignas(1) import_desc {
        LE32  oft;      // orig first thunk
        char  _[8];
        LE32  dllname;
        LE32  iat;      // import address table
    };

    LE32 &IDSIZE(unsigned x);
    LE32 &IDADDR(unsigned x);
    LE32 &ODSIZE(unsigned x);
    LE32 &ODADDR(unsigned x);

    struct alignas(1) pe_section_t {
        char    name[8];
        LE32    vsize;
        LE32    vaddr;
        LE32    size;
        LE32    rawdataptr;
        char    _[12];
        LE32    flags;
    };

    MemBuffer mb_isection;
    pe_section_t *isection;
    bool isdll;
    bool isrtm;
    bool isefi;
    bool use_dep_hack;
    bool use_clear_dirty_stack;
    bool use_stub_relocs;


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

    //NEW: DLL characteristics definition for ASLR, ... - Stefan Widmann
    enum {
        IMAGE_DLLCHARACTERISTICS_HIGH_ENTROPY_VA       = 0x0020,
        IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE          = 0x0040,
        IMAGE_DLLCHARACTERISTICS_FORCE_INTEGRITY       = 0x0080,
        IMAGE_DLLCHARACTERISTICS_NX_COMPAT             = 0x0100,
        IMAGE_DLLCHARACTERISTICS_NO_ISOLATION          = 0x0200,
        IMAGE_DLLCHARACTERISTICS_NO_SEH                = 0x0400,
        IMAGE_DLLCHARACTERISTICS_NO_BIND               = 0x0800,
        IMAGE_DLLCHARACTERISTICS_WDM_DRIVER            = 0x2000,
        IMAGE_DLLCHARACTERISTICS_CONTROL_FLOW_GUARD    = 0x4000,
        IMAGE_DLLCHARACTERISTICS_TERMINAL_SERVER_AWARE = 0x8000
    };

    enum {
        IMAGE_SUBSYSTEM_UNKNOWN = 0,
        IMAGE_SUBSYSTEM_NATIVE  = 1,
        IMAGE_SUBSYSTEM_WINDOWS_GUI  = 2,  // Grapical
        IMAGE_SUBSYSTEM_WINDOWS_CUI  = 3,  // Character-mode
        IMAGE_SUBSYSTEM_WINDOWS_OS2_CUI  = 5,
        IMAGE_SUBSYSTEM_WINDOWS_POSIX_CUI  = 7,
        IMAGE_SUBSYSTEM_WINDOWS_CE_GUI  = 9,
        IMAGE_SUBSYSTEM_EFI_APPLICATION  = 10,
        IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER  = 11,
        IMAGE_SUBSYSTEM_EFI_RUNTIME_DRIVER  = 12,
        IMAGE_SUBSYSTEM_EFI_ROM  = 13,
        IMAGE_SUBSYSTEM_XBOX  = 14,
        IMAGE_SUBSYSTEM_WINDOWS_BOOT_APPLICATION  = 16
    };

    // predefined resource types
    enum {
        RT_CURSOR = 1, RT_BITMAP, RT_ICON, RT_MENU, RT_DIALOG, RT_STRING,
        RT_FONTDIR, RT_FONT, RT_ACCELERATOR, RT_RCDATA, RT_MESSAGETABLE,
        RT_GROUP_CURSOR, RT_GROUP_ICON = 14, RT_VERSION = 16, RT_DLGINCLUDE,
        RT_PLUGPLAY = 19, RT_VXD, RT_ANICURSOR, RT_ANIICON, RT_HTML,
        RT_MANIFEST, RT_LAST
    };

    class Interval : private noncopyable
    {
        unsigned capacity;
        void *base;
    public:
        struct interval
        {
            unsigned start, len;
        } *ivarr;

        unsigned ivnum;

        Interval(void *b);
        ~Interval();

        void add(unsigned start,unsigned len);
        void add(const void *start,unsigned len);
        void add(const void *start,const void *end);
        void add(const Interval *iv);
        void flatten();

        void clear();
        void dump() const;

    private:
        static int __acc_cdecl_qsort compare(const void *p1,const void *p2);
    };

    class Reloc : private noncopyable
    {
        upx_byte *start;
        unsigned size;

        void newRelocPos(void *p);

        struct reloc;
        reloc *rel;
        LE16 *rel1;
        unsigned counts[16];

    public:
        Reloc(upx_byte *,unsigned);
        Reloc(unsigned rnum);
        //
        bool next(unsigned &pos,unsigned &type);
        const unsigned *getcounts() const { return counts; }
        //
        void add(unsigned pos,unsigned type);
        void finish(upx_byte* &p,unsigned &size);
    };

    class Resource : private noncopyable
    {
        struct res_dir_entry;
        struct res_dir;
        struct res_data;
        struct upx_rnode;
        struct upx_rbranch;
        struct upx_rleaf;

        MemBuffer mb_start;
        const upx_byte *start;
        upx_byte   *newstart;
        upx_rnode  *root;
        upx_rleaf  *head;
        upx_rleaf  *current;
        unsigned   dsize;
        unsigned   ssize;

        const upx_byte *ibufstart;
        const upx_byte *ibufend;

        void check(const res_dir*,unsigned);
        upx_rnode *convert(const void *,upx_rnode *,unsigned);
        void build(const upx_rnode *,unsigned &,unsigned &,unsigned);
        void clear(upx_byte *,unsigned,Interval *);
        void dump(const upx_rnode *,unsigned) const;
        void destroy(upx_rnode *urd,unsigned level);

        void ibufcheck(const void *m, unsigned size);

    public:
        Resource(const upx_byte *ibufstart, const upx_byte *ibufen);
        Resource(const upx_byte *p, const upx_byte *ibufstart,
                 const upx_byte *ibufend);
        ~Resource();
        void init(const upx_byte *);

        unsigned dirsize() const;
        bool next();

        unsigned itype() const;
        const upx_byte *ntype() const;
        unsigned size() const;
        unsigned offs() const;
        unsigned &newoffs();

        upx_byte *build();
        bool clear();

        void dump() const;
        unsigned iname() const;
        const upx_byte *nname() const;
        /*
         unsigned ilang() const {return current->id;}
         const upx_byte *nlang() const {return current->name;}
         */
    };

    class Export : private noncopyable
    {
        struct alignas(1) export_dir_t {
            char  _[12]; // flags, timedate, version
            LE32  name;
            char  __[4]; // ordinal base
            LE32  functions;
            LE32  names;
            LE32  addrtable;
            LE32  nameptrtable;
            LE32  ordinaltable;
        };

        export_dir_t edir;
        char  *ename;
        char  *functionptrs;
        char  *ordinals;
        char  **names;

        char  *base;
        unsigned size;
        Interval iv;

    public:
        Export(char *_base);
        ~Export();

        void convert(unsigned eoffs,unsigned esize);
        void build(char *base,unsigned newoffs);
        unsigned getsize() const { return size; }
    };

};

class PeFile32 : public PeFile
{
    typedef PeFile super;
protected:
    PeFile32(InputFile *f);
    virtual ~PeFile32();
    void pack0(OutputFile *fo, unsigned subsystem_mask,
               upx_uint64_t default_imagebase, bool last_section_rsrc_only);
    virtual void unpack(OutputFile *fo) override;
    virtual int canUnpack() override;

    virtual void readPeHeader() override;

    virtual unsigned processImports() override;
    virtual void processRelocs() override;
    virtual void processTls(Interval *) override;
    virtual void processTls(Reloc *, const Interval *, unsigned) override;

    struct alignas(1) pe_header_t {
        // 0x0
        char    _[4];               // pemagic
        LE16    cpu;
        LE16    objects;
        char    __[12];             // timestamp + reserved
        LE16    opthdrsize;
        LE16    flags;
        // optional header
        LE16    coffmagic;          // NEW: Stefan Widmann
        char    ___[2];             // linkerversion
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

        ddirs_t ddirs[16];
    };

    pe_header_t ih, oh;
};

class PeFile64 : public PeFile
{
    typedef PeFile super;
protected:
    PeFile64(InputFile *f);
    virtual ~PeFile64();

    void pack0(OutputFile *fo, unsigned subsystem_mask,
               upx_uint64_t default_imagebase);

    virtual void unpack(OutputFile *fo) override;
    virtual int canUnpack() override;

    virtual void readPeHeader() override;

    virtual unsigned processImports() override;
    virtual void processRelocs() override;
    virtual void processTls(Interval *) override;
    virtual void processTls(Reloc *, const Interval *, unsigned) override;

    struct alignas(1) pe_header_t {
        // 0x0
        char    _[4];               // pemagic
        LE16    cpu;
        LE16    objects;            // number of sections
        char    __[12];             // timestamp + reserved
        LE16    opthdrsize;
        LE16    flags;              // characteristics
        // optional header
        LE16    coffmagic;          // NEW: Stefan Widmann
        char    ___[2];             // linkerversion
        LE32    codesize;
        // 0x20
        LE32    datasize;
        LE32    bsssize;
        LE32    entry;              // still a 32 bit RVA
        LE32    codebase;
        // 0x30
        //LE32    database;         // field does not exist in PE+!
        // nt specific fields
        LE64    imagebase;          // LE32 -> LE64 - Stefan Widmann standard is 0x0000000140000000
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
        char    _____[36];          // stack + heap sizes + loader flag
        // 0x84
        LE32    ddirsentries;       // usually 16

        ddirs_t ddirs[16];
    };

    pe_header_t ih, oh;
};

#endif /* already included */

/* vim:set ts=4 sw=4 et: */
