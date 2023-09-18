/* packer.h --

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

#include "packhead.h"
#include "util/membuffer.h"

class InputFile;
class OutputFile;
class UiPacker;
class Filter;

/*************************************************************************
// purely abstract minimal base class for all packers
//
// clients: PackMaster, UiPacker
**************************************************************************/

class PackerBase {
    friend class UiPacker;
protected:
    explicit PackerBase(InputFile *f);
public:
    virtual ~PackerBase() noexcept {}
    // getVersion() enables detecting forward incompatibility of unpack()
    // by old upx when newer upx changes the format of compressed output.
    virtual int getVersion() const = 0;
    // A unique integer ID for this executable format; see UPX_F_xxx in conf.h.
    virtual int getFormat() const = 0;
    virtual const char *getName() const = 0;
    virtual const char *getFullName(const Options *) const = 0;
    virtual const int *getCompressionMethods(int method, int level) const = 0;
    virtual const int *getFilters() const = 0;

    // canPack() should throw a cantPackException explaining why it cannot pack
    // a recognized format.
    // canPack() can also return -1 to fail early; see class PackMaster
    virtual tribool canPack() = 0;
    // canUnpack() should throw a cantUnpackException explaining why it cannot pack
    // a recognized format.
    // canUnpack() can also return -1 to fail early; see class PackMaster
    virtual tribool canUnpack() = 0;

    // PackMaster entries
    virtual void assertPacker() const = 0;
    virtual void initPackHeader() = 0;
    virtual void updatePackHeader() = 0;
    virtual void doPack(OutputFile *fo) = 0;
    virtual void doUnpack(OutputFile *fo) = 0;
    virtual void doTest() = 0;
    virtual void doList() = 0;
    virtual void doFileInfo() = 0;

protected:
    InputFile *const fi;                // reference
    union {                             // unnamed union
        const upx_int64_t file_size;    // must get set by constructor
        const upx_uint64_t file_size_u; // (explicitly unsigned)
    };
    PackHeader ph; // must be filled by canUnpack(); also used by UiPacker
};

/*************************************************************************
// abstract default implementation class for packers
//
// Packer can be viewed as "PackerDefaultImplV1"; it is grown historically
// and still would benefit from a decomposition
**************************************************************************/

class Packer : public PackerBase {
protected:
    explicit Packer(InputFile *f);

public:
    virtual ~Packer() noexcept;

    // PackMaster entries
    virtual void assertPacker() const override;
    virtual void initPackHeader() final override;
    virtual void updatePackHeader() final override;
    virtual void doPack(OutputFile *fo) final override;
    virtual void doUnpack(OutputFile *fo) final override;
    virtual void doTest() final override;
    virtual void doList() final override;
    virtual void doFileInfo() final override;

    // unpacker capabilities
    virtual bool canUnpackVersion(int version) const { return (version >= 8); }
    virtual bool canUnpackFormat(int format) const { return (format == getFormat()); }

protected:
    // unpacker tests - these may throw exceptions
    virtual bool testUnpackVersion(int version) const;
    virtual bool testUnpackFormat(int format) const;

protected:
    // implementation
    virtual void pack(OutputFile *fo) = 0;
    virtual void unpack(OutputFile *fo) = 0;
    virtual void test();
    virtual void list();
    virtual void fileInfo();

protected:
    // main compression drivers
    bool compress(SPAN_P(byte) i_ptr, unsigned i_len, SPAN_P(byte) o_ptr,
                  const upx_compress_config_t *cconf = nullptr);
    void decompress(SPAN_P(const byte) in, SPAN_P(byte) out, bool verify_checksum = true,
                    Filter *ft = nullptr);
    virtual bool checkDefaultCompressionRatio(unsigned u_len, unsigned c_len) const;
    virtual bool checkCompressionRatio(unsigned u_len, unsigned c_len) const;
    virtual bool checkFinalCompressionRatio(const OutputFile *fo) const;

    // high-level compression drivers
    void compressWithFilters(Filter *ft, const unsigned overlap_range,
                             const upx_compress_config_t *cconf, int filter_strategy = 0,
                             bool inhibit_compression_check = false);
    void compressWithFilters(Filter *ft, const unsigned overlap_range,
                             const upx_compress_config_t *cconf, int filter_strategy,
                             unsigned filter_buf_off, unsigned compress_ibuf_off,
                             unsigned compress_obuf_off, byte *const hdr_ptr, unsigned hdr_len,
                             bool inhibit_compression_check = false);
    // real compression driver
    void compressWithFilters(byte *i_ptr, unsigned i_len, // written and restored by filters
                             byte *o_ptr, byte *f_ptr,
                             unsigned f_len, // subset of [*i_ptr, +i_len)
                             byte *const hdr_ptr, unsigned hdr_len,
                             Filter *parm_ft, // updated
                             unsigned overlap_range, upx_compress_config_t const *cconf,
                             int filter_strategy, bool inhibit_compression_check = false);

    // util for verifying overlapping decompression
    //   non-destructive test
    virtual bool testOverlappingDecompression(const byte *buf, const byte *tbuf,
                                              unsigned overlap_overhead) const;
    //   non-destructive find
    virtual unsigned findOverlapOverhead(const byte *buf, const byte *tbuf, unsigned range = 0,
                                         unsigned upper_limit = ~0u) const;
    //   destructive decompress + verify
    void verifyOverlappingDecompression(Filter *ft = nullptr);
    void verifyOverlappingDecompression(byte *o_ptr, unsigned o_size, Filter *ft = nullptr);

    // packheader handling
    virtual int patchPackHeader(void *b, int blen) final;
    virtual bool getPackHeader(const void *b, int blen, bool allow_incompressible = false) final;
    virtual bool readPackHeader(int len, bool allow_incompressible = false) final;
    virtual void checkAlreadyPacked(const void *b, int blen) final;

    // loader core
    virtual void buildLoader(const Filter *ft) = 0;
    virtual Linker *newLinker() const = 0;
    virtual void relocateLoader();
    // loader util for linker
    virtual byte *getLoader() const;
    virtual int getLoaderSize() const;
    virtual void initLoader(const void *pdata, int plen, int small = -1, int pextra = 0);
#define C const char *
    void addLoader(C);
    void addLoader(C, C);
    void addLoader(C, C, C);
    void addLoader(C, C, C, C);
    void addLoader(C, C, C, C, C);
    void addLoader(C, C, C, C, C, C);
    void addLoader(C, C, C, C, C, C, C);
    void addLoader(C, C, C, C, C, C, C, C);
    void addLoader(C, C, C, C, C, C, C, C, C);
    void addLoader(C, C, C, C, C, C, C, C, C, C);
#undef C
#if (ACC_CC_CLANG || ACC_CC_GNUC)
    void addLoaderVA(const char *s, ...) __attribute__((__sentinel__));
#else
    void addLoaderVA(const char *s, ...);
#endif
    virtual bool hasLoaderSection(const char *name) const;
    virtual int getLoaderSection(const char *name, int *slen = nullptr) const;
    virtual int getLoaderSectionStart(const char *name, int *slen = nullptr) const;

    // compression handling [see packer_c.cpp]
public:
    static bool isValidCompressionMethod(int method);

protected:
    const int *getDefaultCompressionMethods_8(int method, int level, int small = -1) const;
    const int *getDefaultCompressionMethods_le32(int method, int level, int small = -1) const;
    int prepareMethods(int *methods, int ph_method, const int *all_methods) const;
    virtual const char *getDecompressorSections() const;
    virtual unsigned getDecompressorWrkmemSize() const;
    virtual void defineDecompressorSymbols();

    // filter handling [see packer_f.cpp]
    virtual bool isValidFilter(int filter_id) const;
    virtual void optimizeFilter(Filter *, const byte *, unsigned) const {}
    virtual void addFilter32(int filter_id);
    virtual void defineFilterSymbols(const Filter *ft);

    // stub and overlay util
    static void handleStub(InputFile *fi, OutputFile *fo, unsigned size);
    virtual void checkOverlay(unsigned overlay);
    virtual void copyOverlay(OutputFile *fo, unsigned overlay, MemBuffer &buf, bool do_seek = true);

    // misc util
    virtual unsigned getRandomId() const;

    // patch util
    int patch_be16(void *b, int blen, unsigned old, unsigned new_);
    int patch_be16(void *b, int blen, const void *old, unsigned new_);
    int patch_be32(void *b, int blen, unsigned old, unsigned new_);
    int patch_be32(void *b, int blen, const void *old, unsigned new_);
    int patch_le16(void *b, int blen, unsigned old, unsigned new_);
    int patch_le16(void *b, int blen, const void *old, unsigned new_);
    int patch_le32(void *b, int blen, unsigned old, unsigned new_);
    int patch_le32(void *b, int blen, const void *old, unsigned new_);
    void checkPatch(void *b, int blen, int boff, int size);

    // relocation util
    static unsigned optimizeReloc(unsigned relocnum, SPAN_P(byte) relocs, SPAN_S(byte) out,
                                  SPAN_P(byte) image, unsigned image_size, int bits, bool bswap,
                                  int *big);
    static unsigned unoptimizeReloc(SPAN_S(const byte) & in, MemBuffer &out, SPAN_P(byte) image,
                                    unsigned image_size, int bits, bool bswap);

    // Target Endianness abstraction
#if 0
    // permissive version using "void *"
    inline unsigned get_te16(const void *p) const noexcept { return bele->get16(p); }
    inline unsigned get_te32(const void *p) const noexcept { return bele->get32(p); }
    inline upx_uint64_t get_te64(const void *p) const noexcept { return bele->get64(p); }
    inline void set_te16(void *p, unsigned v) noexcept { bele->set16(p, v); }
    inline void set_te32(void *p, unsigned v) noexcept { bele->set32(p, v); }
    inline void set_te64(void *p, upx_uint64_t v) noexcept { bele->set64(p, v); }
#else
    // try to detect TE16 vs TE32 vs TE64 size mismatches; note that byte is explicitly allowed
    template <class T>
    static inline constexpr bool is_te16_type = is_same_any_v<T, byte, upx_uint16_t, BE16, LE16>;
    template <class T>
    static inline constexpr bool is_te32_type = is_same_any_v<T, byte, upx_uint32_t, BE32, LE32>;
    template <class T>
    static inline constexpr bool is_te64_type = is_same_any_v<T, byte, upx_uint64_t, BE64, LE64>;
    template <class T>
    using enable_if_te16 = std::enable_if_t<is_te16_type<T>, T>;
    template <class T>
    using enable_if_te32 = std::enable_if_t<is_te32_type<T>, T>;
    template <class T>
    using enable_if_te64 = std::enable_if_t<is_te64_type<T>, T>;

    template <class T, class = enable_if_te16<T> >
    inline unsigned get_te16(const T *p) const noexcept {
        return bele->get16(p);
    }
    template <class T, class = enable_if_te32<T> >
    inline unsigned get_te32(const T *p) const noexcept {
        return bele->get32(p);
    }
    template <class T, class = enable_if_te64<T> >
    inline upx_uint64_t get_te64(const T *p) const noexcept {
        return bele->get64(p);
    }

    template <class T, class = enable_if_te16<T> >
    inline void set_te16(T *p, unsigned v) noexcept {
        bele->set16(p, v);
    }
    template <class T, class = enable_if_te32<T> >
    inline void set_te32(T *p, unsigned v) noexcept {
        bele->set32(p, v);
    }
    template <class T, class = enable_if_te64<T> >
    inline void set_te64(T *p, upx_uint64_t v) noexcept {
        bele->set64(p, v);
    }
#endif

protected:
    const N_BELE_RTP::AbstractPolicy *bele = nullptr; // target endianness

    // PackHeader
    int ph_format = -1;
    int ph_version = -1;

    // compression buffers
    MemBuffer ibuf;        // input
    MemBuffer obuf;        // output
    unsigned ibufgood = 0; // high-water mark in ibuf (pefile.cpp)

    // UI handler
    OwningPointer(UiPacker) uip = nullptr; // owner

    // linker
    OwningPointer(Linker) linker = nullptr; // owner

private:
    // private to checkPatch()
    void *last_patch = nullptr;
    int last_patch_len = 0;
    int last_patch_off = 0;

private:
    // disable copy and move
    Packer(const Packer &) DELETED_FUNCTION;
    Packer &operator=(const Packer &) DELETED_FUNCTION;
    Packer(Packer &&) noexcept DELETED_FUNCTION;
    Packer &operator=(Packer &&) noexcept DELETED_FUNCTION;
};

/* vim:set ts=4 sw=4 et: */
