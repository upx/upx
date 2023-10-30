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

#pragma once

/*************************************************************************
// general/pe handling
**************************************************************************/

class PeFile : public Packer {
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

    explicit PeFile(InputFile *f);
    virtual ~PeFile() noexcept;

    void readSectionHeaders(unsigned objs, unsigned sizeof_ih);
    unsigned readSections(unsigned objs, unsigned usize, unsigned ih_filealign,
                          unsigned ih_datasize);
    void checkHeaderValues(unsigned subsystem, unsigned mask, unsigned ih_entry,
                           unsigned ih_filealign);
    unsigned handleStripRelocs(upx_uint64_t ih_imagebase, upx_uint64_t default_imagebase,
                               LE16 &dllflags);

    virtual bool needForceOption() const = 0;
    virtual void callCompressWithFilters(Filter &, int filter_strategy, unsigned ih_codebase);
    virtual void defineSymbols(unsigned ncsection, unsigned upxsection, unsigned sizeof_oh,
                               unsigned isize_isplit, unsigned s1addr) = 0;
    virtual void addNewRelocations(Reloc &, unsigned) {}
    void callProcessStubRelocs(Reloc &rel, unsigned &ic);
    void callProcessResources(Resource &res, unsigned &ic);
    virtual unsigned getProcessImportParam(unsigned) { return 0; }
    virtual void setOhDataBase(const pe_section_t *osection) = 0;
    virtual void setOhHeaderSize(const pe_section_t *osection) = 0;

    template <typename LEXX, typename ht>
    void pack0(OutputFile *fo, ht &ih, ht &oh, unsigned subsystem_mask,
               upx_uint64_t default_imagebase, bool last_section_rsrc_only);

    template <typename ht, typename LEXX, typename ord_mask_t>
    void unpack0(OutputFile *fo, const ht &ih, ht &oh, ord_mask_t ord_mask, bool set_oft);

    // unpacker capabilities
    virtual bool canUnpackVersion(int version) const override {
        return (version >= 12 && version <= 13);
    }

    int canUnpack0(unsigned max_sections, unsigned objs, unsigned ih_entry, unsigned ih_size);

protected:
    static int checkMachine(unsigned cpu);
    virtual int readFileHeader();
    virtual bool testUnpackVersion(int version) const override;
    virtual void readPeHeader() = 0;

    unsigned pe_offset;

    template <typename LEXX, typename ord_mask_t>
    unsigned processImports0(ord_mask_t ord_mask);

    template <typename LEXX, typename ord_mask_t>
    void rebuildImports(SPAN_S(byte) & extra_info, ord_mask_t ord_mask, bool set_oft);
    virtual unsigned processImports() = 0;
    virtual void processImports2(unsigned, unsigned);
    MemBuffer mb_oimport;
    SPAN_0(byte) oimport = nullptr;
    unsigned soimport;
    byte *oimpdlls = nullptr;
    unsigned soimpdlls;
    ImportLinker *ilinker = nullptr;
    virtual const char *kernelDll() const { return "KERNEL32.DLL"; }
    void addKernelImport(const char *);
    virtual void addStubImports();
    upx_uint64_t ilinkerGetAddress(const char *, const char *) const;

    virtual void processRelocs() = 0;
    void rebuildRelocs(SPAN_S(byte) &, unsigned bits, unsigned flags, upx_uint64_t imagebase);
    MemBuffer mb_orelocs;
    SPAN_0(byte) orelocs = nullptr;
    unsigned sorelocs;
    byte *oxrelocs = nullptr;
    unsigned soxrelocs;

    void processExports(Export *);
    void processExports(Export *, unsigned);
    void rebuildExports();
    MemBuffer mb_oexport;
    SPAN_0(byte) oexport = nullptr;
    unsigned soexport;

    void processResources(Resource *);
    void processResources(Resource *, unsigned);
    void rebuildResources(SPAN_S(byte) &, unsigned);
    MemBuffer mb_oresources;
    SPAN_0(byte) oresources = nullptr;
    unsigned soresources;

    template <typename>
    struct tls_traits;
    template <typename LEXX>
    void processTls1(Interval *iv, typename tls_traits<LEXX>::cb_value_t imagebase,
                     unsigned imagesize); // pass 1
    template <typename LEXX>
    void processTls2(Reloc *rel, const Interval *iv, unsigned newaddr,
                     typename tls_traits<LEXX>::cb_value_t imagebase); // pass 2
    virtual void processTls(Interval *iv) = 0;
    virtual void processTls(Reloc *r, const Interval *iv, unsigned a) = 0;

    void rebuildTls();
    MemBuffer mb_otls;
    SPAN_0(byte) otls = nullptr;
    unsigned sotls;
    unsigned tlsindex;
    unsigned tlscb_ptr;
    unsigned tls_handler_offset = 0;
    bool use_tls_callbacks = false;

    void processLoadConf(Reloc *, const Interval *, unsigned);
    void processLoadConf(Interval *);
    MemBuffer mb_oloadconf;
    byte *oloadconf = nullptr;
    unsigned soloadconf;

    unsigned stripDebug(unsigned);

    unsigned icondir_offset;
    int icondir_count;

    bool importbyordinal = false;
    bool kernel32ordinal = false;
    unsigned rvamin;
    unsigned cimports; // rva of preprocessed imports
    unsigned crelocs;  // rva of preprocessed fixups
    int big_relocs;

    struct alignas(1) ddirs_t {
        LE32 vaddr;
        LE32 size;
    };
    ddirs_t *iddirs = nullptr;
    ddirs_t *oddirs = nullptr;

    LE32 &IDSIZE(unsigned x);
    LE32 &IDADDR(unsigned x);
    LE32 &ODSIZE(unsigned x);
    LE32 &ODADDR(unsigned x);
    const LE32 &IDSIZE(unsigned x) const;
    const LE32 &IDADDR(unsigned x) const;

    struct alignas(1) import_desc {
        LE32 oft; // orig first thunk
        byte _[8];
        LE32 dllname;
        LE32 iat; // import address table
    };

    struct alignas(1) pe_section_t {
        char name[8];
        LE32 vsize;
        LE32 vaddr;
        LE32 size;
        LE32 rawdataptr;
        byte _[12];
        LE32 flags;
    };

    MemBuffer mb_isection;
    SPAN_0(pe_section_t) isection = nullptr;
    bool isdll = false;
    bool isrtm = false;
    bool isefi = false;
    bool use_dep_hack = true;
    bool use_clear_dirty_stack = true;
    bool use_stub_relocs = true;

    static unsigned virta2objnum(unsigned, SPAN_0(pe_section_t), unsigned);
    unsigned tryremove(unsigned, unsigned);

    enum {
        IMAGE_FILE_MACHINE_UNKNOWN = 0,
        IMAGE_FILE_MACHINE_AMD64 = 0x8664,   // win64/pe (amd64)
        IMAGE_FILE_MACHINE_ARM = 0x01c0,     // wince/arm (Windows CE)
        IMAGE_FILE_MACHINE_ARMNT = 0x01c4,   // win32/arm
        IMAGE_FILE_MACHINE_ARM64 = 0xaa64,   // win64/arm64
        IMAGE_FILE_MACHINE_ARM64EC = 0xa641, // win64/arm64ec
        IMAGE_FILE_MACHINE_I386 = 0x014c,    // win32/pe (i386)
        IMAGE_FILE_MACHINE_IA64 = 0x200,
        IMAGE_FILE_MACHINE_LOONGARCH32 = 0x6232,
        IMAGE_FILE_MACHINE_LOONGARCH64 = 0x6264,
        IMAGE_FILE_MACHINE_RISCV32 = 0x5032,
        IMAGE_FILE_MACHINE_RISCV64 = 0x5064,
        IMAGE_FILE_MACHINE_RISCV128 = 0x5128,
        IMAGE_FILE_MACHINE_THUMB = 0x01c2, // wince/arm (Windows CE)
    };

    enum {
        PEDIR_EXPORT = 0,
        PEDIR_IMPORT = 1,
        PEDIR_RESOURCE = 2,
        PEDIR_EXCEPTION = 3, // Exception table
        PEDIR_SECURITY = 4,  // Certificate table (file pointer)
        PEDIR_BASERELOC = 5,
        PEDIR_DEBUG = 6,
        PEDIR_ARCHITECTURE = 7, // Architecture-specific data
        PEDIR_GLOBALPTR = 8,    // Global pointer
        PEDIR_TLS = 9,
        PEDIR_LOAD_CONFIG = 10, // Load Config Table
        PEDIR_BOUND_IMPORT = 11,
        PEDIR_IAT = 12,
        PEDIR_DELAY_IMPORT = 13,   // Delay Import Descriptor
        PEDIR_COM_DESCRIPTOR = 14, // Com+ Runtime Header
    };

    // section flags
    enum : unsigned {
        IMAGE_SCN_CNT_CODE = 0x00000020,
        IMAGE_SCN_CNT_INITIALIZED_DATA = 0x00000040,
        IMAGE_SCN_CNT_UNINITIALIZED_DATA = 0x00000080,
        IMAGE_SCN_LNK_OTHER = 0x00000100,
        IMAGE_SCN_LNK_INFO = 0x00000200,
        IMAGE_SCN_LNK_REMOVE = 0x00000800,
        IMAGE_SCN_LNK_COMDAT = 0x00001000,
        IMAGE_SCN_GPREL = 0x00008000,
        IMAGE_SCN_MEM_PURGEABLE = 0x00020000,
        IMAGE_SCN_MEM_16BIT = 0x00020000,
        IMAGE_SCN_MEM_LOCKED = 0x00040000,
        IMAGE_SCN_MEM_PRELOAD = 0x00080000,
        IMAGE_SCN_ALIGN_1BYTES = 0x00100000,
        IMAGE_SCN_ALIGN_2BYTES = 0x00200000,
        IMAGE_SCN_ALIGN_4BYTES = 0x00300000,
        IMAGE_SCN_ALIGN_8BYTES = 0x00400000,
        IMAGE_SCN_ALIGN_16BYTES = 0x00500000,
        IMAGE_SCN_ALIGN_32BYTES = 0x00600000,
        IMAGE_SCN_ALIGN_64BYTES = 0x00700000,
        IMAGE_SCN_ALIGN_128BYTES = 0x00800000,
        IMAGE_SCN_ALIGN_256BYTES = 0x00900000,
        IMAGE_SCN_ALIGN_512BYTES = 0x00A00000,
        IMAGE_SCN_ALIGN_1024BYTES = 0x00B00000,
        IMAGE_SCN_ALIGN_2048BYTES = 0x00C00000,
        IMAGE_SCN_ALIGN_4096BYTES = 0x00D00000,
        IMAGE_SCN_ALIGN_8192BYTES = 0x00E00000,
        IMAGE_SCN_LNK_NRELOC_OVFL = 0x01000000,
        IMAGE_SCN_MEM_DISCARDABLE = 0x02000000,
        IMAGE_SCN_MEM_NOT_CACHED = 0x04000000,
        IMAGE_SCN_MEM_NOT_PAGED = 0x08000000,
        IMAGE_SCN_MEM_SHARED = 0x10000000,
        IMAGE_SCN_MEM_EXECUTE = 0x20000000,
        IMAGE_SCN_MEM_READ = 0x40000000,
        IMAGE_SCN_MEM_WRITE = 0x80000000,
    };

    enum {
        IMAGE_FILE_RELOCS_STRIPPED = 0x0001,
        IMAGE_FILE_EXECUTABLE_IMAGE = 0x0002,
        IMAGE_FILE_LINE_NUMS_STRIPPED = 0x0004,
        IMAGE_FILE_LOCAL_SYMS_STRIPPED = 0x0008,
        IMAGE_FILE_AGGRESSIVE_WS_TRIM = 0x0010,
        IMAGE_FILE_LARGE_ADDRESS_AWARE = 0x0020,
        IMAGE_FILE_BYTES_REVERSED_LO = 0x0080,
        IMAGE_FILE_32BIT_MACHINE = 0x0100,
        IMAGE_FILE_DEBUG_STRIPPED = 0x0200,
        IMAGE_FILE_REMOVABLE_RUN_FROM_SWAP = 0x0400,
        IMAGE_FILE_NET_RUN_FROM_SWAP = 0x0800,
        IMAGE_FILE_SYSTEM = 0x1000,
        IMAGE_FILE_DLL = 0x2000,
        IMAGE_FILE_UP_SYSTEM_ONLY = 0x4000,
        IMAGE_FILE_BYTES_REVERSE_HI = 0x8000,
    };

    enum {
        IMAGE_DLLCHARACTERISTICS_HIGH_ENTROPY_VA = 0x0020,
        IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE = 0x0040,
        IMAGE_DLLCHARACTERISTICS_FORCE_INTEGRITY = 0x0080,
        IMAGE_DLLCHARACTERISTICS_NX_COMPAT = 0x0100,
        IMAGE_DLLCHARACTERISTICS_NO_ISOLATION = 0x0200,
        IMAGE_DLLCHARACTERISTICS_NO_SEH = 0x0400,
        IMAGE_DLLCHARACTERISTICS_NO_BIND = 0x0800,
        IMAGE_DLLCHARACTERISTICS_APPCONTAINER = 0x1000,
        IMAGE_DLLCHARACTERISTICS_WDM_DRIVER = 0x2000,
        IMAGE_DLLCHARACTERISTICS_GUARD_CF = 0x4000,
        IMAGE_DLLCHARACTERISTICS_TERMINAL_SERVER_AWARE = 0x8000,
    };

    enum {
        IMAGE_SUBSYSTEM_UNKNOWN = 0,
        IMAGE_SUBSYSTEM_NATIVE = 1,
        IMAGE_SUBSYSTEM_WINDOWS_GUI = 2, // Graphical User Interface
        IMAGE_SUBSYSTEM_WINDOWS_CUI = 3, // Character User Interface
        IMAGE_SUBSYSTEM_WINDOWS_OS2_CUI = 5,
        IMAGE_SUBSYSTEM_WINDOWS_POSIX_CUI = 7,
        IMAGE_SUBSYSTEM_NATIVE_WINDOWS = 8,
        IMAGE_SUBSYSTEM_WINDOWS_CE_GUI = 9,
        IMAGE_SUBSYSTEM_EFI_APPLICATION = 10,
        IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER = 11,
        IMAGE_SUBSYSTEM_EFI_RUNTIME_DRIVER = 12,
        IMAGE_SUBSYSTEM_EFI_ROM = 13,
        IMAGE_SUBSYSTEM_XBOX = 14,
        IMAGE_SUBSYSTEM_WINDOWS_BOOT_APPLICATION = 16,
    };

    // predefined Resource Types
    enum {
        RT_CURSOR = 1,
        RT_BITMAP,
        RT_ICON,
        RT_MENU,
        RT_DIALOG,
        RT_STRING,
        RT_FONTDIR,
        RT_FONT,
        RT_ACCELERATOR,
        RT_RCDATA,
        RT_MESSAGETABLE,
        RT_GROUP_CURSOR,
        RT_GROUP_ICON = 14,
        RT_VERSION = 16,
        RT_DLGINCLUDE,
        RT_PLUGPLAY = 19,
        RT_VXD,
        RT_ANICURSOR,
        RT_ANIICON,
        RT_HTML,
        RT_MANIFEST,
        RT_LAST
    };

    class Interval final : private noncopyable {
        unsigned capacity = 0;
        void *base = nullptr;
    public:
        struct interval {
            unsigned start, len;
        };
        struct interval *ivarr = nullptr;
        unsigned ivnum = 0;

        explicit Interval(void *b);
        ~Interval() noexcept;

        void add(unsigned start, unsigned len);
        void add(const void *start, unsigned len);
        void add(const void *start, const void *end);
        void add(const Interval *iv);
        void flatten();

        void clear();
        void dump() const;

    private:
        static int __acc_cdecl_qsort compare(const void *p1, const void *p2);
    };

    class Reloc final : private noncopyable {
        // these are set in constructor
        byte *start = nullptr;
        unsigned start_size_in_bytes = 0;
        bool start_did_alloc = false;
        SPAN_0(byte) start_buf = nullptr;

        struct alignas(1) BaseReloc { // IMAGE_BASE_RELOCATION
            LE32 virtual_address;
            LE32 size_of_block;
            // LE16 rel1[COUNT]; // COUNT == (size_of_block - 8) / 2
        };
        struct RelocationBlock {
            SPAN_0(BaseReloc) rel = nullptr;
            SPAN_0(LE16) rel1 = nullptr;
            unsigned count = 0;
            void reset() noexcept;
        };
        RelocationBlock rb;                   // current RelocationBlock
        bool readFromRelocationBlock(byte *); // set rb

        unsigned counts[16] = {};

        void initSpans();
    public:
        explicit Reloc(byte *ptr, unsigned bytes);
        explicit Reloc(unsigned relocnum);
        ~Reloc() noexcept;
        //
        bool next(unsigned &result_pos, unsigned &result_type);
        const unsigned *getcounts() const { return counts; }
        //
        void add(unsigned pos, unsigned type);
        void finish(byte *(&result_ptr), unsigned &result_size); // => transfer ownership
    };

    class Resource final : private noncopyable {
        struct res_dir_entry;
        struct res_dir;
        struct res_data;
        struct upx_rnode;
        struct upx_rbranch;
        struct upx_rleaf;

        MemBuffer mb_start;
        const byte *start = nullptr;
        byte *newstart = nullptr;
        upx_rnode *root = nullptr;
        upx_rleaf *head = nullptr;
        upx_rleaf *current = nullptr;
        unsigned dsize = 0;
        unsigned ssize = 0;

        const byte *ibufstart = nullptr;
        const byte *ibufend = nullptr;

        void check(const res_dir *, unsigned);
        upx_rnode *convert(const void *, upx_rnode *, unsigned);
        void build(const upx_rnode *, unsigned &, unsigned &, unsigned);
        void clear(byte *, unsigned, Interval *);
        void dump(const upx_rnode *, unsigned) const;
        void destroy(upx_rnode *urd, unsigned level) noexcept;

        void ibufcheck(const void *m, unsigned size);

    public:
        explicit Resource(const byte *ibufstart, const byte *ibufen);
        explicit Resource(const byte *p, const byte *ibufstart, const byte *ibufend);
        ~Resource() noexcept;
        void init(const byte *);

        unsigned dirsize() const;
        bool next();

        unsigned itype() const;
        const byte *ntype() const;
        unsigned size() const;
        unsigned offs() const;
        unsigned &newoffs();

        byte *build();
        bool clear();

        void dump() const;
        unsigned iname() const;
        const byte *nname() const;
        /*
         unsigned ilang() const {return current->id;}
         const byte *nlang() const {return current->name;}
         */
    };

    class Export final : private noncopyable {
        struct alignas(1) export_dir_t {
            byte _[12]; // flags, timedate, version
            LE32 name;
            byte __[4]; // ordinal base
            LE32 functions;
            LE32 names;
            LE32 addrtable;
            LE32 nameptrtable;
            LE32 ordinaltable;
        };

        export_dir_t edir;
        char *ename;
        char *functionptrs;
        char *ordinals;
        char **names;

        char *base;
        unsigned size;
        Interval iv;

    public:
        explicit Export(char *_base);
        ~Export() noexcept;

        void convert(unsigned eoffs, unsigned esize);
        void build(char *base, unsigned newoffs);
        unsigned getsize() const { return size; }
    };
};

class PeFile32 : public PeFile {
    typedef PeFile super;
protected:
    explicit PeFile32(InputFile *f);
    virtual ~PeFile32() noexcept;

    void pack0(OutputFile *fo, unsigned subsystem_mask, upx_uint64_t default_imagebase,
               bool last_section_rsrc_only);
    virtual void unpack(OutputFile *fo) override;
    virtual tribool canUnpack() override;

    virtual void readPeHeader() override;

    virtual unsigned processImports() override;
    virtual void processRelocs() override;
    virtual void processTls(Interval *) override;
    virtual void processTls(Reloc *, const Interval *, unsigned) override;

    struct alignas(1) pe_header_t {
        // 0x00
        byte _[4]; // pemagic
        // 0x04 IMAGE_FILE_HEADER
        LE16 cpu;        // IMAGE_FILE_MACHINE_xxx
        LE16 objects;    // NumberOfSections
        byte __[12];     // timestamp + reserved
        LE16 opthdrsize; // SizeOfOptionalHeader
        LE16 flags;      // IMAGE_FILE_xxx Characteristics
        // 0x18 IMAGE_OPTIONAL_HEADER32
        LE16 coffmagic; // NEW: Stefan Widmann
        byte ___[2];    // linkerversion
        LE32 codesize;
        // 0x20
        LE32 datasize;
        LE32 bsssize;
        LE32 entry;
        LE32 codebase;
        // 0x30
        LE32 database;
        // nt specific fields
        LE32 imagebase;
        LE32 objectalign;
        LE32 filealign; // should set to 0x200 ?
        // 0x40
        byte ____[16]; // versions
        // 0x50
        LE32 imagesize;
        LE32 headersize;
        LE32 chksum;    // should set to 0
        LE16 subsystem; // IMAGE_SUBSYSTEM_xxx
        LE16 dllflags;  // IMAGE_DLLCHARACTERISTICS_xxx
        // 0x60
        byte _____[20]; // stack + heap sizes
        // 0x74
        LE32 ddirsentries; // usually 16
        // 0x78
        ddirs_t ddirs[16];
    };

    pe_header_t ih, oh;
};

class PeFile64 : public PeFile {
    typedef PeFile super;
protected:
    explicit PeFile64(InputFile *f);
    virtual ~PeFile64() noexcept;

    void pack0(OutputFile *fo, unsigned subsystem_mask, upx_uint64_t default_imagebase);

    virtual void unpack(OutputFile *fo) override;
    virtual tribool canUnpack() override;

    virtual void readPeHeader() override;

    virtual unsigned processImports() override;
    virtual void processRelocs() override;
    virtual void processTls(Interval *) override;
    virtual void processTls(Reloc *, const Interval *, unsigned) override;

    struct alignas(1) pe_header_t {
        // 0x00
        byte _[4]; // pemagic
        // 0x04 IMAGE_FILE_HEADER
        LE16 cpu;        // IMAGE_FILE_MACHINE_xxx
        LE16 objects;    // NumberOfSections
        byte __[12];     // timestamp + reserved
        LE16 opthdrsize; // SizeOfOptionalHeader
        LE16 flags;      // IMAGE_FILE_xxx Characteristics
        // 0x18 IMAGE_OPTIONAL_HEADER64
        LE16 coffmagic; // NEW: Stefan Widmann
        byte ___[2];    // linkerversion
        LE32 codesize;
        // 0x20
        LE32 datasize;
        LE32 bsssize;
        LE32 entry; // still a 32 bit RVA
        LE32 codebase;
        // 0x30
        // LE32    database;         // field does not exist in PE+!
        // nt specific fields
        LE64 imagebase; // LE32 -> LE64 - Stefan Widmann standard is 0x0000000140000000
        LE32 objectalign;
        LE32 filealign; // should set to 0x200 ?
        // 0x40
        byte ____[16]; // versions
        // 0x50
        LE32 imagesize;
        LE32 headersize;
        LE32 chksum;    // should set to 0
        LE16 subsystem; // IMAGE_SUBSYSTEM_xxx
        LE16 dllflags;  // IMAGE_DLLCHARACTERISTICS_xxx
        // 0x60
        byte _____[36]; // stack + heap sizes + loader flag
        // 0x84
        LE32 ddirsentries; // usually 16
        // 0x88
        ddirs_t ddirs[16];
    };

    pe_header_t ih, oh;
};

/* vim:set ts=4 sw=4 et: */
