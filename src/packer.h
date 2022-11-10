/* packer.h --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2022 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2022 Laszlo Molnar
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
#ifndef UPX_PACKER_H__
#define UPX_PACKER_H__ 1

#include "util/membuffer.h"

class InputFile;
class OutputFile;
class Packer;
class PackMaster;
class UiPacker;
class Filter;

/*************************************************************************
//
**************************************************************************/

// see stub/src/include/header.S
class PackHeader final {
    friend class Packer;

    // these are strictly private to friend Packer
    PackHeader();

    void putPackHeader(SPAN_S(upx_byte) p);
    bool fillPackHeader(SPAN_S(const upx_byte) b, int blen);

public:
    int getPackHeaderSize() const;

public:
    // fields stored in compressed file
    // enum { magic = UPX_MAGIC_LE32 };
    int version;
    int format; // executable format
    int method; // compresison method
    int level;  // compresison level 1..10
    unsigned u_len;
    unsigned c_len;
    unsigned u_adler;
    unsigned c_adler;
    unsigned u_file_size;
    int filter;
    int filter_cto;
    int n_mru; // FIXME: rename to filter_misc
    int header_checksum;

    // support fields for verifying decompression
    unsigned saved_u_adler;
    unsigned saved_c_adler;

    // info fields set by fillPackHeader()
    unsigned buf_offset;

    // info fields set by Packer::compress()
    upx_compress_result_t compress_result;
    // unsigned min_offset_found;
    unsigned max_offset_found;
    // unsigned min_match_found;
    unsigned max_match_found;
    // unsigned min_run_found;
    unsigned max_run_found;
    unsigned first_offset_found;
    // unsigned same_match_offsets_found;

    // info fields set by Packer::compressWithFilters()
    unsigned overlap_overhead;
};

bool ph_skipVerify(const PackHeader &ph);
void ph_decompress(PackHeader &ph, SPAN_P(const upx_byte) in, SPAN_P(upx_byte) out,
                   bool verify_checksum, Filter *ft);
bool ph_testOverlappingDecompression(const PackHeader &ph, SPAN_P(const upx_byte) buf,
                                     unsigned overlap_overhead);

/*************************************************************************
// abstract base class for packers
//
// FIXME: this class is way too fat and badly needs a decomposition
**************************************************************************/

class Packer {
    friend class UiPacker;

protected:
    Packer(InputFile *f);

public:
    virtual ~Packer();
    virtual void assertPacker() const;

    // getVersion() enables detecting forward incompatibility of unpack()
    // by old upx when newer upx changes the format of compressed output.
    virtual int getVersion() const = 0;
    // A unique integer ID for this executable format. See conf.h.
    virtual int getFormat() const = 0;
    virtual const char *getName() const = 0;
    virtual const char *getFullName(const options_t *) const = 0;
    virtual const int *getCompressionMethods(int method, int level) const = 0;
    virtual const int *getFilters() const = 0;

    // PackMaster entries
    void initPackHeader();
    void updatePackHeader();
    void doPack(OutputFile *fo);
    void doUnpack(OutputFile *fo);
    void doTest();
    void doList();
    void doFileInfo();

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

public:
    // canPack() should throw a cantPackException eplaining why it
    // cannot pack a recognized format.
    virtual bool canPack() = 0;
    // canUnpack() can return -1 meaning "format recognized, but file
    // is definitely not packed". See packmast.cpp try_unpack().
    virtual int canUnpack() = 0;

protected:
    // main compression drivers
    bool compress(SPAN_P(upx_byte) i_ptr, unsigned i_len, SPAN_P(upx_byte) o_ptr,
                  const upx_compress_config_t *cconf = nullptr);
    void decompress(SPAN_P(const upx_byte) in, SPAN_P(upx_byte) out, bool verify_checksum = true,
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
                             unsigned compress_obuf_off, upx_bytep const hdr_ptr, unsigned hdr_len,
                             bool inhibit_compression_check = false);
    // real compression driver
    void compressWithFilters(upx_bytep i_ptr, unsigned i_len, // written and restored by filters
                             upx_bytep o_ptr, upx_bytep f_ptr,
                             unsigned f_len, // subset of [*i_ptr, +i_len)
                             upx_bytep const hdr_ptr, unsigned hdr_len,
                             Filter *parm_ft, // updated
                             unsigned overlap_range, upx_compress_config_t const *cconf,
                             int filter_strategy, bool inhibit_compression_check = false);

    // util for verifying overlapping decompresion
    //   non-destructive test
    virtual bool testOverlappingDecompression(const upx_bytep buf, const upx_bytep tbuf,
                                              unsigned overlap_overhead) const;
    //   non-destructive find
    virtual unsigned findOverlapOverhead(const upx_bytep buf, const upx_bytep tbuf,
                                         unsigned range = 0, unsigned upper_limit = ~0u) const;
    //   destructive decompress + verify
    void verifyOverlappingDecompression(Filter *ft = nullptr);
    void verifyOverlappingDecompression(upx_bytep o_ptr, unsigned o_size, Filter *ft = nullptr);

    // packheader handling
    virtual int patchPackHeader(void *b, int blen);
    virtual bool getPackHeader(const void *b, int blen, bool allow_incompressible = false);
    virtual bool readPackHeader(int len, bool allow_incompressible = false);
    virtual void checkAlreadyPacked(const void *b, int blen);

    // loader core
    virtual void buildLoader(const Filter *ft) = 0;
    virtual Linker *newLinker() const = 0;
    virtual void relocateLoader();
    // loader util for linker
    virtual upx_byte *getLoader() const;
    virtual int getLoaderSize() const;
    virtual void initLoader(const void *pdata, int plen, int small = -1);
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
    virtual void optimizeFilter(Filter *, const upx_byte *, unsigned) const {}
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
    static unsigned optimizeReloc(SPAN_P(upx_byte) in, unsigned relocnum, SPAN_P(upx_byte) out,
                                  SPAN_P(upx_byte) image, unsigned headway, bool bswap, int *big,
                                  int bits);
    static unsigned unoptimizeReloc(SPAN_P(upx_byte) & in, SPAN_P(upx_byte) image, MemBuffer &out,
                                    bool bswap, int bits);

    static unsigned optimizeReloc32(SPAN_P(upx_byte) in, unsigned relocnum, SPAN_P(upx_byte) out,
                                    SPAN_P(upx_byte) image, unsigned headway, bool bswap, int *big);
    static unsigned unoptimizeReloc32(SPAN_P(upx_byte) & in, SPAN_P(upx_byte) image, MemBuffer &out,
                                      bool bswap);

    static unsigned optimizeReloc64(SPAN_P(upx_byte) in, unsigned relocnum, SPAN_P(upx_byte) out,
                                    SPAN_P(upx_byte) image, unsigned headway, bool bswap, int *big);
    static unsigned unoptimizeReloc64(SPAN_P(upx_byte) & in, SPAN_P(upx_byte) image, MemBuffer &out,
                                      bool bswap);

    // Target Endianness abstraction
    unsigned get_te16(const void *p) const { return bele->get16(p); }
    unsigned get_te32(const void *p) const { return bele->get32(p); }
    upx_uint64_t get_te64(const void *p) const { return bele->get64(p); }
    void set_te16(void *p, unsigned v) { bele->set16(p, v); }
    void set_te32(void *p, unsigned v) { bele->set32(p, v); }
    void set_te64(void *p, upx_uint64_t v) { bele->set64(p, v); }

protected:
    const N_BELE_RTP::AbstractPolicy *bele = nullptr; // target endianness
    InputFile *fi = nullptr;

    union {                        // unnamed union
        upx_int64_t file_size = 0; // will get set by constructor
        upx_uint64_t file_size_u;  // explicitly unsigned
    };

    PackHeader ph; // must be filled by canUnpack()
    int ph_format;
    int ph_version;

    // compression buffers
    MemBuffer ibuf;    // input
    MemBuffer obuf;    // output
    unsigned ibufgood; // high-water mark in ibuf (pefile.cpp)

    // UI handler
    UiPacker *uip = nullptr;

    // linker
    Linker *linker = nullptr;

private:
    // private to checkPatch()
    void *last_patch = nullptr;
    int last_patch_len;
    int last_patch_off;

private:
    // disable copy and assignment
    Packer(const Packer &) = delete;
    Packer &operator=(const Packer &) = delete;
};

int force_method(int method);     // (0x80ul<<24)|method
int forced_method(int method);    // (0x80ul<<24)|method ==> method
int is_forced_method(int method); // predicate

#endif /* already included */

/* vim:set ts=4 sw=4 et: */
