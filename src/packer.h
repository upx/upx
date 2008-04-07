/* packer.h --

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


#ifndef __UPX_PACKER_H
#define __UPX_PACKER_H

#include "mem.h"

class InputFile;
class OutputFile;
class Packer;
class PackMaster;
class UiPacker;
class Filter;


/*************************************************************************
//
**************************************************************************/

// see stub/header.ash
class PackHeader
{
    friend class Packer;

private:
    // these are strictly private to Packer and not accessible in subclasses:
    PackHeader();

    void putPackHeader(upx_bytep p);
    bool fillPackHeader(const upx_bytep b, int blen);

public:
    int getPackHeaderSize() const;

public:
    // fields stored in compressed file
    //enum { magic = UPX_MAGIC_LE32 };
    int version;
    int format;                 // executable format
    int method;                 // compresison method
    int level;                  // compresison level 1..10
    unsigned u_len;
    unsigned c_len;
    unsigned u_adler;
    unsigned c_adler;
    off_t u_file_size;
    int filter;
    int filter_cto;
    int n_mru;                  // FIXME: rename to filter_misc
    int header_checksum;

    // support fields for verifying decompression
    unsigned saved_u_adler;
    unsigned saved_c_adler;

    // info fields set by fillPackHeader()
    long buf_offset;

    // info fields set by Packer::compress()
    upx_compress_result_t compress_result;
    //unsigned min_offset_found;
    unsigned max_offset_found;
    //unsigned min_match_found;
    unsigned max_match_found;
    //unsigned min_run_found;
    unsigned max_run_found;
    unsigned first_offset_found;
    //unsigned same_match_offsets_found;

    // info fields set by Packer::compressWithFilters()
    unsigned overlap_overhead;
};


bool ph_skipVerify(const PackHeader &ph);
void ph_decompress(PackHeader &ph, const upx_bytep in, upx_bytep out,
                   bool verify_checksum, Filter *ft);
bool ph_testOverlappingDecompression(const PackHeader &ph, const upx_bytep buf,
                                     unsigned overlap_overhead);


/*************************************************************************
// abstract base class for packers
//
// FIXME: this class is way too fat and badly needs a decomposition
**************************************************************************/

class Packer
{
    //friend class PackMaster;
    friend class UiPacker;
protected:
    Packer(InputFile *f);
public:
    virtual ~Packer();
    virtual void assertPacker() const;

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
    virtual bool canUnpackVersion(int version) const
        { return (version >= 8); }
    virtual bool canUnpackFormat(int format) const
        { return (format == getFormat()); }

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
    // canUnpack() can return -1 meaning "format recognized, but file
    // is definitely not packed" (see packmast.cpp).
    virtual bool canPack() = 0;
    virtual int canUnpack() = 0;
    virtual int canTest() { return canUnpack(); }
    virtual int canList() { return canUnpack(); }

protected:
    // main compression drivers
    virtual bool compress(upx_bytep i_ptr, unsigned i_len, upx_bytep o_ptr,
                          const upx_compress_config_t *cconf = NULL);
    virtual void decompress(const upx_bytep in, upx_bytep out,
                            bool verify_checksum = true, Filter *ft = NULL);
    virtual bool checkDefaultCompressionRatio(unsigned u_len, unsigned c_len) const;
    virtual bool checkCompressionRatio(unsigned u_len, unsigned c_len) const;
    virtual bool checkFinalCompressionRatio(const OutputFile *fo) const;

    // high-level compression drivers
    void compressWithFilters(Filter *ft,
                             const unsigned overlap_range,
                             const upx_compress_config_t *cconf,
                             int filter_strategy = 0);
    void compressWithFilters(Filter *ft,
                             const unsigned overlap_range,
                             const upx_compress_config_t *cconf,
                             int filter_strategy,
                             unsigned filter_buf_off,
                             unsigned compress_ibuf_off,
                             unsigned compress_obuf_off,
                             const upx_bytep hdr_ptr, unsigned hdr_len);
    // real compression driver
    void compressWithFilters(upx_bytep i_ptr, unsigned i_len,
                             upx_bytep o_ptr,
                             upx_bytep f_ptr, unsigned f_len,
                             const upx_bytep hdr_ptr, unsigned hdr_len,
                             Filter *ft,
                             const unsigned overlap_range,
                             const upx_compress_config_t *cconf,
                             int filter_strategy);

    // util for verifying overlapping decompresion
    //   non-destructive test
    virtual bool testOverlappingDecompression(const upx_bytep buf,
                                              const upx_bytep tbuf,
                                              unsigned overlap_overhead) const;
    //   non-destructive find
    virtual unsigned findOverlapOverhead(const upx_bytep buf,
                                         const upx_bytep tbuf,
                                         unsigned range = 0,
                                         unsigned upper_limit = ~0u) const;
    //   destructive decompress + verify
    void verifyOverlappingDecompression(Filter *ft = NULL);
    void verifyOverlappingDecompression(upx_bytep o_ptr, unsigned o_size, Filter *ft = NULL);

    // packheader handling
    virtual int patchPackHeader(void *b, int blen);
    virtual bool getPackHeader(void *b, int blen, bool allow_incompressible=false);
    virtual bool readPackHeader(int len, bool allow_incompressible=false);
    virtual void checkAlreadyPacked(const void *b, int blen);

    // loader core
    virtual void buildLoader(const Filter *ft) = 0;
    virtual Linker* newLinker() const = 0;
    virtual void relocateLoader();
    // loader util for linker
    virtual upx_byte *getLoader() const;
    virtual int getLoaderSize() const;
    virtual void initLoader(const void *pdata, int plen, int small=-1);
#define C const char *
    void addLoader(C); void addLoader(C,C); void addLoader(C,C,C);
    void addLoader(C,C,C,C); void addLoader(C,C,C,C,C);
    void addLoader(C,C,C,C,C,C); void addLoader(C,C,C,C,C,C,C);
    void addLoader(C,C,C,C,C,C,C,C); void addLoader(C,C,C,C,C,C,C,C,C);
    void addLoader(C,C,C,C,C,C,C,C,C,C);
#undef C
#if 1 && (ACC_CC_GNUC >= 0x040100)
    void __acc_cdecl_va addLoaderVA(const char *s, ...) __attribute__((__sentinel__));
#else
    void __acc_cdecl_va addLoaderVA(const char *s, ...);
#endif
    virtual bool hasLoaderSection(const char *name) const;
    virtual int getLoaderSection(const char *name, int *slen=NULL) const;
    virtual int getLoaderSectionStart(const char *name, int *slen=NULL) const;

    // compression handling [see packer_c.cpp]
public:
    static bool isValidCompressionMethod(int method);
protected:
    const int *getDefaultCompressionMethods_8(int method, int level, int small=-1) const;
    const int *getDefaultCompressionMethods_le32(int method, int level, int small=-1) const;
    virtual const char *getDecompressorSections() const;
    virtual unsigned getDecompressorWrkmemSize() const;
    virtual void defineDecompressorSymbols();

    // filter handling [see packer_f.cpp]
    virtual bool isValidFilter(int filter_id) const;
    virtual void optimizeFilter(Filter *, const upx_byte *, unsigned) const { }
    virtual void addFilter32(int filter_id);
    virtual void defineFilterSymbols(const Filter *ft);

    // stub and overlay util
    static void handleStub(InputFile *fi, OutputFile *fo, long size);
    virtual void checkOverlay(unsigned overlay);
    virtual void copyOverlay(OutputFile *fo, unsigned overlay,
                             MemBuffer *buf, bool do_seek=true);

    // misc util
    virtual unsigned getRandomId() const;

    // patch util
    int patch_be16(void *b, int blen, unsigned old, unsigned new_);
    int patch_be16(void *b, int blen, const void * old, unsigned new_);
    int patch_be32(void *b, int blen, unsigned old, unsigned new_);
    int patch_be32(void *b, int blen, const void * old, unsigned new_);
    int patch_le16(void *b, int blen, unsigned old, unsigned new_);
    int patch_le16(void *b, int blen, const void * old, unsigned new_);
    int patch_le32(void *b, int blen, unsigned old, unsigned new_);
    int patch_le32(void *b, int blen, const void * old, unsigned new_);
    void checkPatch(void *b, int blen, int boff, int size);

    // relocation util
    static upx_byte *optimizeReloc32(upx_byte *in,unsigned relocnum,upx_byte *out,upx_byte *image,int bs,int *big);
    static unsigned unoptimizeReloc32(upx_byte **in,upx_byte *image,MemBuffer *out,int bs);

    // target endianness abstraction
    unsigned get_te16(const void *p)        const { return bele->get16(p); }
    unsigned get_te32(const void *p)        const { return bele->get32(p); }
    acc_uint64l_t get_te64(const void *p)   const { return bele->get64(p); }
    void set_te16(void *p, unsigned v)      const { bele->set16(p, v); }
    void set_te32(void *p, unsigned v)      const { bele->set32(p, v); }
    void set_te64(void *p, acc_uint64l_t v) const { bele->set64(p, v); }

protected:
    const N_BELE_RTP::AbstractPolicy *bele; // target endianness
    InputFile *fi;
    off_t file_size;        // will get set by constructor
    PackHeader ph;          // must be filled by canUnpack()
    int ph_format;
    int ph_version;

    // compression buffers
    MemBuffer ibuf;         // input
    MemBuffer obuf;         // output

    // UI handler
    UiPacker *uip;

    // linker
    Linker *linker;

private:
    // private to checkPatch()
    void *last_patch;
    int last_patch_len;
    int last_patch_off;

private:
    // disable copy and assignment
    Packer(const Packer &); // {}
    Packer& operator= (const Packer &); // { return *this; }
};


#endif /* already included */


/*
vi:ts=4:et
*/

