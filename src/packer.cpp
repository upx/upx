/* packer.cpp --

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

#include "conf.h"
#include "file.h"
#include "packer.h"
#include "filter.h"
#include "linker.h"
#include "ui.h"

/*************************************************************************
//
**************************************************************************/

Packer::Packer(InputFile *f)
    : bele(nullptr), fi(f), file_size(-1), ph_format(-1), ph_version(-1), ibufgood(0), uip(nullptr),
      linker(nullptr), last_patch(nullptr), last_patch_len(0), last_patch_off(0) {
    file_size = 0;
    if (fi != nullptr)
        file_size = fi->st_size();
    mem_size_assert(1, file_size_u);
    uip = new UiPacker(this);
    mem_clear(&ph, sizeof(ph));
}

Packer::~Packer() {
    delete uip;
    uip = nullptr;
    delete linker;
    linker = nullptr;
}

// for PackMaster
void Packer::assertPacker() const {
    assert(getFormat() > 0);
    assert(getFormat() < 255);
    assert(getVersion() >= 11);
    assert(getVersion() <= 14);
    assert(strlen(getName()) <= 15);
    // info: 36 is the limit for show_all_packers() in help.cpp
    assert(strlen(getFullName(opt)) <= 32);
    assert(strlen(getFullName(nullptr)) <= 32);
    if (bele == nullptr)
        fprintf(stderr, "%s\n", getName());
    assert(bele != nullptr);
    if (getFormat() != UPX_F_MACH_FAT) // macho/fat is multiarch
    {
        const N_BELE_RTP::AbstractPolicy *format_bele;
        if (getFormat() < 128)
            format_bele = &N_BELE_RTP::le_policy;
        else
            format_bele = &N_BELE_RTP::be_policy;
        if (bele != format_bele)
            fprintf(stderr, "%s\n", getName());
        assert(bele == format_bele);
    }
#if 1
    Linker *l = newLinker();
    if (bele != l->bele)
        fprintf(stderr, "%s\n", getName());
    assert(bele == l->bele);
    delete l;
#endif
}

/*************************************************************************
// public entries called from class PackMaster
**************************************************************************/

void Packer::doPack(OutputFile *fo) {
    uip->uiPackStart(fo);
    pack(fo);
    uip->uiPackEnd(fo);
}

void Packer::doUnpack(OutputFile *fo) {
    uip->uiUnpackStart(fo);
    unpack(fo);
    uip->uiUnpackEnd(fo);
}

void Packer::doTest() {
    uip->uiTestStart();
    test();
    uip->uiTestEnd();
}

void Packer::doList() {
    uip->uiListStart();
    list();
    uip->uiListEnd();
}

void Packer::doFileInfo() {
    uip->uiFileInfoStart();
    fileInfo();
    uip->uiFileInfoEnd();
}

/*************************************************************************
// default actions
**************************************************************************/

void Packer::test() { unpack(nullptr); }

void Packer::list() { uip->uiList(); }

void Packer::fileInfo() {
    // FIXME: subclasses should list their sections here
    // We also should try to get a nice layout...
}

bool Packer::testUnpackVersion(int version) const {
    if (version != ph_version && ph_version != -1)
        throwCantUnpack("program has been modified; run a virus checker!");
    if (!canUnpackVersion(version))
        throwCantUnpack("I am not compatible with older versions of UPX");
    return true;
}

bool Packer::testUnpackFormat(int format) const {
    if (format != ph_format && ph_format != -1)
        throwCantUnpack("program has been modified; run a virus checker!");
    return canUnpackFormat(format);
}

bool ph_skipVerify(const PackHeader &ph) {
    if (M_IS_DEFLATE(ph.method))
        return false;
    if (M_IS_LZMA(ph.method))
        return false;
    if (ph.level > 1)
        return false;
    return true;
}

int force_method(int method) // mark as forced
{
    return (0x80ul << 24) | method;
}

int is_forced_method(int method) // predicate
{
    return -0x80 == (method >> 24);
}

int forced_method(int method) // extract the forced method
{
    if (is_forced_method(method))
        method &= ~(0x80ul << 24);
    assert(method > 0);
    return method;
}

/*************************************************************************
// compress - wrap call to low-level upx_compress()
**************************************************************************/

bool Packer::compress(SPAN_P(upx_byte) i_ptr, unsigned i_len, SPAN_P(upx_byte) o_ptr,
                      const upx_compress_config_t *cconf_parm) {
    ph.u_len = i_len;
    ph.c_len = 0;
    assert(ph.level >= 1);
    assert(ph.level <= 10);

    // Avoid too many progress bar updates. 64 is s->bar_len in ui.cpp.
    unsigned step = (ph.u_len < 64 * 1024) ? 0 : ph.u_len / 64;

    // save current checksums
    ph.saved_u_adler = ph.u_adler;
    ph.saved_c_adler = ph.c_adler;
    // update checksum of uncompressed data
    ph.u_adler = upx_adler32(raw_bytes(i_ptr, ph.u_len), ph.u_len, ph.u_adler);

    // set compression parameters
    upx_compress_config_t cconf;
    cconf.reset();
    if (cconf_parm)
        cconf = *cconf_parm;
    // cconf options
    int method = forced_method(ph.method);
    if (M_IS_NRV2B(method) || M_IS_NRV2D(method) || M_IS_NRV2E(method)) {
        if (opt->crp.crp_ucl.c_flags != -1)
            cconf.conf_ucl.c_flags = opt->crp.crp_ucl.c_flags;
        if (opt->crp.crp_ucl.p_level != -1)
            cconf.conf_ucl.p_level = opt->crp.crp_ucl.p_level;
        if (opt->crp.crp_ucl.h_level != -1)
            cconf.conf_ucl.h_level = opt->crp.crp_ucl.h_level;
        if (opt->crp.crp_ucl.max_offset != UINT_MAX &&
            opt->crp.crp_ucl.max_offset < cconf.conf_ucl.max_offset)
            cconf.conf_ucl.max_offset = opt->crp.crp_ucl.max_offset;
        if (opt->crp.crp_ucl.max_match != UINT_MAX &&
            opt->crp.crp_ucl.max_match < cconf.conf_ucl.max_match)
            cconf.conf_ucl.max_match = opt->crp.crp_ucl.max_match;
#if (WITH_NRV)
        if (ph.level >= 7 || (ph.level >= 4 && ph.u_len >= 512 * 1024))
            step = 0;
#endif
    }
    if (M_IS_LZMA(method)) {
        oassign(cconf.conf_lzma.pos_bits, opt->crp.crp_lzma.pos_bits);
        oassign(cconf.conf_lzma.lit_pos_bits, opt->crp.crp_lzma.lit_pos_bits);
        oassign(cconf.conf_lzma.lit_context_bits, opt->crp.crp_lzma.lit_context_bits);
        oassign(cconf.conf_lzma.dict_size, opt->crp.crp_lzma.dict_size);
        oassign(cconf.conf_lzma.num_fast_bytes, opt->crp.crp_lzma.num_fast_bytes);
    }
    if (M_IS_DEFLATE(method)) {
        oassign(cconf.conf_zlib.mem_level, opt->crp.crp_zlib.mem_level);
        oassign(cconf.conf_zlib.window_bits, opt->crp.crp_zlib.window_bits);
        oassign(cconf.conf_zlib.strategy, opt->crp.crp_zlib.strategy);
    }
    if (uip->ui_pass >= 0)
        uip->ui_pass++;
    uip->startCallback(ph.u_len, step, uip->ui_pass, uip->ui_total_passes);
    uip->firstCallback();

    // OutputFile::dump("data.raw", in, ph.u_len);

    // compress
    int r = upx_compress(raw_bytes(i_ptr, ph.u_len), ph.u_len, raw_bytes(o_ptr, 0), &ph.c_len,
                         uip->getCallback(), method, ph.level, &cconf, &ph.compress_result);

    // uip->finalCallback(ph.u_len, ph.c_len);
    uip->endCallback();

    if (r == UPX_E_OUT_OF_MEMORY)
        throwOutOfMemoryException();
    if (r != UPX_E_OK)
        throwInternalError("compression failed");

    if (M_IS_NRV2B(method) || M_IS_NRV2D(method) || M_IS_NRV2E(method)) {
        const ucl_uint *res = ph.compress_result.result_ucl.result;
        // ph.min_offset_found = res[0];
        ph.max_offset_found = res[1];
        // ph.min_match_found = res[2];
        ph.max_match_found = res[3];
        // ph.min_run_found = res[4];
        ph.max_run_found = res[5];
        ph.first_offset_found = res[6];
        // ph.same_match_offsets_found = res[7];
        if (cconf_parm) {
            assert(cconf.conf_ucl.max_offset == 0 ||
                   cconf.conf_ucl.max_offset >= ph.max_offset_found);
            assert(cconf.conf_ucl.max_match == 0 || cconf.conf_ucl.max_match >= ph.max_match_found);
        }
    }

    // printf("\nPacker::compress: %d/%d: %7d -> %7d\n", method, ph.level, ph.u_len, ph.c_len);
    if (!checkCompressionRatio(ph.u_len, ph.c_len))
        return false;
    // return in any case if not compressible
    if (ph.c_len >= ph.u_len)
        return false;

    // update checksum of compressed data
    ph.c_adler = upx_adler32(raw_bytes(o_ptr, ph.c_len), ph.c_len, ph.c_adler);
    // Decompress and verify. Skip this when using the fastest level.
    if (!ph_skipVerify(ph)) {
        // decompress
        unsigned new_len = ph.u_len;
        r = upx_decompress(raw_bytes(o_ptr, ph.c_len), ph.c_len, raw_bytes(i_ptr, ph.u_len),
                           &new_len, method, &ph.compress_result);
        if (r == UPX_E_OUT_OF_MEMORY)
            throwOutOfMemoryException();
        // printf("%d %d: %d %d %d\n", method, r, ph.c_len, ph.u_len, new_len);
        if (r != UPX_E_OK)
            throwInternalError("decompression failed");
        if (new_len != ph.u_len)
            throwInternalError("decompression failed (size error)");

        // verify decompression
        if (ph.u_adler != upx_adler32(raw_bytes(i_ptr, ph.u_len), ph.u_len, ph.saved_u_adler))
            throwInternalError("decompression failed (checksum error)");
    }
    return true;
}

#if 0
bool Packer::compress(upx_bytep in, upx_bytep out,
                      const upx_compress_config_t *cconf)
{
    return ph_compress(ph, in, out, cconf);
}
#endif

bool Packer::checkDefaultCompressionRatio(unsigned u_len, unsigned c_len) const {
    assert((int) u_len > 0);
    assert((int) c_len > 0);
    if (c_len >= u_len)
        return false;
    unsigned gain = u_len - c_len;

    if (gain < 512) // need at least 512 bytes gain
        return false;
#if 1
    if (gain >= 4096) // ok if we have 4096 bytes gain
        return true;
    if (gain >= u_len / 16) // ok if we have 6.25% gain
        return true;
    return false;
#else
    return true;
#endif
}

bool Packer::checkCompressionRatio(unsigned u_len, unsigned c_len) const {
    return checkDefaultCompressionRatio(u_len, c_len);
}

bool Packer::checkFinalCompressionRatio(const OutputFile *fo) const {
    const unsigned u_len = file_size;
    const unsigned c_len = fo->getBytesWritten();
    return checkDefaultCompressionRatio(u_len, c_len);
}

/*************************************************************************
// decompress
**************************************************************************/

void ph_decompress(PackHeader &ph, SPAN_P(const upx_byte) in, SPAN_P(upx_byte) out,
                   bool verify_checksum, Filter *ft) {
    unsigned adler;

    // verify checksum of compressed data
    if (verify_checksum) {
        adler = upx_adler32(raw_bytes(in, ph.c_len), ph.c_len, ph.saved_c_adler);
        if (adler != ph.c_adler)
            throwChecksumError();
    }

    // decompress
    if (ph.u_len < ph.c_len) {
        throwCantUnpack("header corrupted");
    }
    unsigned new_len = ph.u_len;
    int r = upx_decompress(raw_bytes(in, ph.c_len), ph.c_len, raw_bytes(out, ph.u_len), &new_len,
                           forced_method(ph.method), &ph.compress_result);
    if (r == UPX_E_OUT_OF_MEMORY)
        throwOutOfMemoryException();
    if (r != UPX_E_OK || new_len != ph.u_len)
        throwCompressedDataViolation();

    // verify checksum of decompressed data
    if (verify_checksum) {
        if (ft)
            ft->unfilter(raw_bytes(out, ph.u_len), ph.u_len);
        adler = upx_adler32(raw_bytes(out, ph.u_len), ph.u_len, ph.saved_u_adler);
        if (adler != ph.u_adler)
            throwChecksumError();
    }
}

void Packer::decompress(SPAN_P(const upx_byte) in, SPAN_P(upx_byte) out, bool verify_checksum,
                        Filter *ft) {
    ph_decompress(ph, in, out, verify_checksum, ft);
}

/*************************************************************************
// overlapping decompression
**************************************************************************/

static bool ph_testOverlappingDecompression(const PackHeader &ph, const upx_bytep buf,
                                            const upx_bytep tbuf, unsigned overlap_overhead) {
    if (ph.c_len >= ph.u_len)
        return false;

    assert((int) overlap_overhead >= 0);
    assert((int) (ph.u_len + overlap_overhead) >= 0);

    // Because upx_test_overlap() does not use the asm_fast decompressor
    // we must account for extra 3 bytes that asm_fast does use,
    // or else we may fail at runtime decompression.
    unsigned extra = 0;
    if (M_IS_NRV2B(ph.method) || M_IS_NRV2D(ph.method) || M_IS_NRV2E(ph.method))
        extra = 3;
    if (overlap_overhead <= 4 + extra) // don't waste time here
        return false;
    overlap_overhead -= extra;

    unsigned src_off = ph.u_len + overlap_overhead - ph.c_len;
    unsigned new_len = ph.u_len;
    int r = upx_test_overlap(buf - src_off, tbuf, src_off, ph.c_len, &new_len,
                             forced_method(ph.method), &ph.compress_result);
    if (r == UPX_E_OUT_OF_MEMORY)
        throwOutOfMemoryException();
    return (r == UPX_E_OK && new_len == ph.u_len);
}

bool Packer::testOverlappingDecompression(const upx_bytep buf, const upx_bytep tbuf,
                                          unsigned overlap_overhead) const {
    return ph_testOverlappingDecompression(ph, buf, tbuf, overlap_overhead);
}

void Packer::verifyOverlappingDecompression(Filter *ft) {
    assert(ph.c_len < ph.u_len);
    assert((int) ph.overlap_overhead > 0);
    // Idea:
    //   obuf[] was allocated with MemBuffer::allocForCompression(), and
    //   its contents are no longer needed, i.e. the compressed data
    //   must have been already written.
    //   We now can perform a real overlapping decompression and
    //   verify the checksum.
    //
    // Note:
    //   This verify is just because of complete paranoia that there
    //   could be a hidden bug in the upx_test_overlap implementation,
    //   and it should not be necessary at all.
    //
    // See also:
    //   Filter::verifyUnfilter()

    if (ph_skipVerify(ph))
        return;
    unsigned offset = (ph.u_len + ph.overlap_overhead) - ph.c_len;
    if (offset + ph.c_len > obuf.getSize())
        return;
    memmove(obuf + offset, obuf, ph.c_len);
    decompress(obuf + offset, obuf, true, ft);
    obuf.checkState();
}

void Packer::verifyOverlappingDecompression(upx_bytep o_ptr, unsigned o_size, Filter *ft) {
    assert(ph.c_len < ph.u_len);
    assert((int) ph.overlap_overhead > 0);
    if (ph_skipVerify(ph))
        return;
    unsigned offset = (ph.u_len + ph.overlap_overhead) - ph.c_len;
    if (offset + ph.c_len > o_size)
        return;
    memmove(o_ptr + offset, o_ptr, ph.c_len);
    decompress(o_ptr + offset, o_ptr, true, ft);
}

/*************************************************************************
// Find overhead for in-place decompression in a heuristic way
// (using a binary search). Return 0 on error.
//
// To speed up things:
//   - you can pass the range of an acceptable interval (so that
//     we can succeed early)
//   - you can enforce an upper_limit (so that we can fail early)
**************************************************************************/

unsigned Packer::findOverlapOverhead(const upx_bytep buf, const upx_bytep tbuf, unsigned range,
                                     unsigned upper_limit) const {
    assert((int) range >= 0);

    // prepare to deal with very pessimistic values
    unsigned low = 1;
    unsigned high = UPX_MIN(ph.u_len + 512, upper_limit);
    // but be optimistic for first try (speedup)
    unsigned m = UPX_MIN(16u, high);
    //
    unsigned overhead = 0;
    unsigned nr = 0; // statistics

    while (high >= low) {
        assert(m >= low);
        assert(m <= high);
        assert(m < overhead || overhead == 0);
        nr++;
        bool success = testOverlappingDecompression(buf, tbuf, m);
        // printf("testOverlapOverhead(%d): %d %d: %d -> %d\n", nr, low, high, m, (int)success);
        if (success) {
            overhead = m;
            // Succeed early if m lies in [low .. low+range-1], i.e. if
            // if the range of the current interval is <= range.
            //   if (m <= low + range - 1)
            //   if (m <  low + range)
            if (m - low < range) // avoid underflow
                break;
            high = m - 1;
        } else
            low = m + 1;
        ////m = (low + high) / 2;
        m = (low & high) + ((low ^ high) >> 1); // avoid overflow
    }

    // printf("findOverlapOverhead: %d (%d tries)\n", overhead, nr);
    if (overhead == 0)
        throwInternalError("this is an oo bug");

    UNUSED(nr);
    return overhead;
}

/*************************************************************************
// file i/o utils
**************************************************************************/

void Packer::handleStub(InputFile *fif, OutputFile *fo, unsigned size) {
    if (fo) {
        if (size > 0) {
            // copy stub from exe
            info("Copying original stub: %u bytes", size);
            ByteArray(stub, size);
            fif->seek(0, SEEK_SET);
            fif->readx(stub, size);
            fo->write(stub, size);
        } else {
            // no stub
        }
    }
}

void Packer::checkOverlay(unsigned overlay) {
    if ((int) overlay < 0 || overlay > file_size_u)
        throw OverlayException("invalid overlay size; file is possibly corrupt");
    if (overlay == 0)
        return;
    info("Found overlay: %d bytes", overlay);
    if (opt->overlay == opt->SKIP_OVERLAY)
        throw OverlayException("file has overlay -- skipped; try '--overlay=copy'");
}

void Packer::copyOverlay(OutputFile *fo, unsigned overlay, MemBuffer &buf, bool do_seek) {
    assert((int) overlay >= 0);
    assert(overlay < file_size_u);
    buf.checkState();
    if (!fo || overlay == 0)
        return;
    if (opt->overlay != opt->COPY_OVERLAY) {
        assert(opt->overlay == opt->STRIP_OVERLAY);
        infoWarning("stripping overlay: %d bytes", overlay);
        return;
    }
    info("Copying overlay: %d bytes", overlay);
    if (do_seek)
        fi->seek(-(upx_off_t) overlay, SEEK_END);

    // get buffer size, align to improve i/o speed
    unsigned buf_size = buf.getSize();
    if (buf_size > 65536)
        buf_size = ALIGN_DOWN(buf_size, 4096u);
    assert((int) buf_size > 0);

    do {
        unsigned len = overlay < buf_size ? overlay : buf_size;
        fi->readx(buf, len);
        fo->write(buf, len);
        overlay -= len;
    } while (overlay > 0);
    buf.checkState();
}

// Create a pseudo-unique program id.
unsigned Packer::getRandomId() const {
    if (opt->debug.disable_random_id)
        return 0x01020304;
    unsigned id = 0;
#if 0 && defined(__unix__)
    // Don't consume precious bytes from /dev/urandom.
    int fd = open("/dev/urandom", O_RDONLY | O_BINARY);
    if (fd < 0)
        fd = open("/dev/random", O_RDONLY | O_BINARY);
    if (fd >= 0)
    {
        if (read(fd, &id, 4) != 4)
            id = 0;
        close(fd);
    }
#endif
    while (id == 0) {
#if !(HAVE_GETTIMEOFDAY) || ((ACC_OS_DOS32) && defined(__DJGPP__))
        id ^= (unsigned) time(nullptr);
        id ^= ((unsigned) clock()) << 12;
#else
        struct timeval tv;
        gettimeofday(&tv, nullptr);
        id ^= (unsigned) tv.tv_sec;
        id ^= ((unsigned) tv.tv_usec) << 12; // shift into high-bits
#endif
#if (HAVE_GETPID)
        id ^= (unsigned) getpid();
#endif
        id ^= (unsigned) fi->st.st_ino;
        id ^= (unsigned) fi->st.st_atime;
        id ^= (unsigned) rand();
    }
    return id;
}

/*************************************************************************
// packheader util
**************************************************************************/

// this is called directly after the constructor from class PackMaster
void Packer::initPackHeader() {
    mem_clear(&ph, sizeof(ph));
    ph.version = getVersion();
    ph.format = getFormat();
    ph.method = M_NONE;
    ph.level = -1;
    ph.u_adler = ph.c_adler = ph.saved_u_adler = ph.saved_c_adler = upx_adler32(nullptr, 0);
    ph.buf_offset = 0;
    ph.u_file_size = file_size;
}

// this is called directly after canPack() from class PackMaster
void Packer::updatePackHeader() {
    assert(opt->cmd == CMD_COMPRESS);
    //
    const int *m = getCompressionMethods(opt->method, opt->level);
    ph.method = m[0];
    ph.level = opt->level;
    if (ph.level < 0)
        ph.level = file_size < 512 * 1024 ? 8 : 7;
    //
    assert(isValidCompressionMethod(ph.method));
    assert(1 <= ph.level && ph.level <= 10);
}

// FIXME: remove patchPackHeader() and fold into relocateLoader();
//   then make linker->relocate() private (friend Packer)
int Packer::patchPackHeader(void *b, int blen) {
    assert(isValidFilter(ph.filter));

    const int size = ph.getPackHeaderSize();
    if (linker->findSection("UPX1HEAD", false))
        assert(size == linker->getSectionSize("UPX1HEAD"));
    int boff = find_le32(b, blen, UPX_MAGIC_LE32);
    checkPatch(b, blen, boff, size);

    auto bb = (upx_byte *) b;
    ph.putPackHeader(SPAN_S_MAKE(upx_byte, bb + boff, blen, bb));

    return boff;
}

bool Packer::getPackHeader(const void *b, int blen, bool allow_incompressible) {
    auto bb = (const upx_byte *) b;
    if (!ph.fillPackHeader(SPAN_S_MAKE(const upx_byte, bb, blen), blen))
        return false;

    if (ph.version > getVersion())
        throwCantUnpack("need a newer version of UPX");
    // Some formats might be able to unpack old versions because
    // their implementation hasn't changed. Ask them.
    if (opt->cmd != CMD_FILEINFO)
        if (!testUnpackVersion(ph.version))
            return false;

    if (ph.c_len > ph.u_len || (ph.c_len == ph.u_len && !allow_incompressible) ||
        ph.c_len >= file_size_u || ph.version <= 0 || ph.version >= 0xff)
        throwCantUnpack("header corrupted");
    else if (ph.u_len > ph.u_file_size) {
#if 0
        // FIXME: does this check make sense w.r.t. overlays ???
        if (ph.format == UPX_F_WIN32_PE || ph.format == UPX_F_DOS_EXE)
            // may get longer
            ((void)0);
        else
            throwCantUnpack("header size corrupted");
#endif
    }
    if (!isValidCompressionMethod(ph.method))
        throwCantUnpack("unknown compression method (try a newer version of UPX)");

    // Some formats might be able to unpack "subformats". Ask them.
    if (!testUnpackFormat(ph.format))
        return false;

    return true;
}

bool Packer::readPackHeader(int len, bool allow_incompressible) {
    assert(len > 0);
    MemBuffer buf(len);
    len = fi->read(buf, len);
    if (len <= 0)
        return false;
    return getPackHeader(buf, len, allow_incompressible);
}

void Packer::checkAlreadyPacked(const void *b, int blen) {
    int boff = find_le32(b, blen, UPX_MAGIC_LE32);
    if (boff < 0)
        return;

    // FIXME: could add some more checks to verify that this
    //   is a real PackHeader, e.g.
    //
    // PackHeader tmp;
    // if (!tmp.fillPackHeader((unsigned char *)b + boff, blen - boff))
    //    return;
    //
    // This also would require that the buffer in 'b' holds
    // the full PackHeader, and not only the 4 magic bytes.

    throwAlreadyPacked();
}

/*************************************************************************
// patch util for loader
**************************************************************************/

void Packer::checkPatch(void *b, int blen, int boff, int size) {
    if (b == nullptr && blen == 0 && boff == 0 && size == 0) {
        // reset
        last_patch = nullptr;
        last_patch_len = 0;
        last_patch_off = 0;
        return;
    }
    if (b == nullptr || blen <= 0 || boff < 0 || size <= 0)
        throwBadLoader();
    if (boff + size <= 0 || boff + size > blen)
        throwBadLoader();
    // printf("checkPatch: %p %5d %5d %2d\n", b, blen, boff, size);
    if (b == last_patch) {
        if (boff + size > last_patch_off)
            throwInternalError("invalid patch order");
        // The next check is not strictly necessary, but the buffer
        // length should better not increase...
        if (blen > last_patch_len)
            throwInternalError("invalid patch order (length)");
    } else
        last_patch = b;
    last_patch_len = blen;
    last_patch_off = boff;
}

int Packer::patch_be16(void *b, int blen, unsigned old, unsigned new_) {
    int boff = find_be16(b, blen, old);
    checkPatch(b, blen, boff, 2);

    unsigned char *p = (unsigned char *) b + boff;
    set_be16(p, new_);

    return boff;
}

int Packer::patch_be16(void *b, int blen, const void *old, unsigned new_) {
    int boff = find(b, blen, old, 2);
    checkPatch(b, blen, boff, 2);

    unsigned char *p = (unsigned char *) b + boff;
    set_be16(p, new_);

    return boff;
}

int Packer::patch_be32(void *b, int blen, unsigned old, unsigned new_) {
    int boff = find_be32(b, blen, old);
    checkPatch(b, blen, boff, 4);

    unsigned char *p = (unsigned char *) b + boff;
    set_be32(p, new_);

    return boff;
}

int Packer::patch_be32(void *b, int blen, const void *old, unsigned new_) {
    int boff = find(b, blen, old, 4);
    checkPatch(b, blen, boff, 4);

    unsigned char *p = (unsigned char *) b + boff;
    set_be32(p, new_);

    return boff;
}

int Packer::patch_le16(void *b, int blen, unsigned old, unsigned new_) {
    int boff = find_le16(b, blen, old);
    checkPatch(b, blen, boff, 2);

    unsigned char *p = (unsigned char *) b + boff;
    set_le16(p, new_);

    return boff;
}

int Packer::patch_le16(void *b, int blen, const void *old, unsigned new_) {
    int boff = find(b, blen, old, 2);
    checkPatch(b, blen, boff, 2);

    unsigned char *p = (unsigned char *) b + boff;
    set_le16(p, new_);

    return boff;
}

int Packer::patch_le32(void *b, int blen, unsigned old, unsigned new_) {
    int boff = find_le32(b, blen, old);
    checkPatch(b, blen, boff, 4);

    unsigned char *p = (unsigned char *) b + boff;
    set_le32(p, new_);

    return boff;
}

int Packer::patch_le32(void *b, int blen, const void *old, unsigned new_) {
    int boff = find(b, blen, old, 4);
    checkPatch(b, blen, boff, 4);

    unsigned char *p = (unsigned char *) b + boff;
    set_le32(p, new_);

    return boff;
}

/*************************************************************************
// relocation util
**************************************************************************/

unsigned Packer::optimizeReloc(SPAN_P(upx_byte) in, unsigned relocnum, SPAN_P(upx_byte) out,
                               SPAN_P(upx_byte) image, unsigned headway, bool bswap, int *big,
                               int bits) {
    if (opt->exact)
        throwCantPackExact();

    *big = 0;
    if (relocnum == 0)
        return 0;
    qsort(raw_bytes(in, 4 * relocnum), relocnum, 4, le32_compare);

    unsigned jc, pc, oc;
    SPAN_P_VAR(upx_byte, fix, out);

    pc = (unsigned) -4;
    for (jc = 0; jc < relocnum; jc++) {
        oc = get_le32(in + jc * 4) - pc;
        if (oc == 0)
            continue;
        else if ((int) oc < 4)
            throwCantPack("overlapping fixups");
        else if (oc < 0xF0)
            *fix++ = (unsigned char) oc;
        else if (oc < 0x100000) {
            *fix++ = (unsigned char) (0xF0 + (oc >> 16));
            *fix++ = (unsigned char) oc;
            *fix++ = (unsigned char) (oc >> 8);
        } else {
            *big = 1;
            *fix++ = 0xf0;
            *fix++ = 0;
            *fix++ = 0;
            set_le32(fix, oc);
            fix += 4;
        }
        pc += oc;
        if (headway <= pc) {
            char msg[80];
            snprintf(msg, sizeof(msg), "bad reloc[%#x] = %#x", jc, oc);
            throwCantPack(msg);
        }
        if (bswap) {
            if (bits == 32)
                set_be32(image + pc, get_le32(image + pc));
            else if (bits == 64)
                set_be64(image + pc, get_le64(image + pc));
            else
                throwInternalError("optimizeReloc problem");
        }
    }
    *fix++ = 0;
    return ptr_udiff_bytes(fix, out);
}

unsigned Packer::optimizeReloc32(SPAN_P(upx_byte) in, unsigned relocnum, SPAN_P(upx_byte) out,
                                 SPAN_P(upx_byte) image, unsigned headway, bool bswap, int *big) {
    return optimizeReloc(in, relocnum, out, image, headway, bswap, big, 32);
}

unsigned Packer::optimizeReloc64(SPAN_P(upx_byte) in, unsigned relocnum, SPAN_P(upx_byte) out,
                                 SPAN_P(upx_byte) image, unsigned headway, bool bswap, int *big) {
    return optimizeReloc(in, relocnum, out, image, headway, bswap, big, 64);
}

unsigned Packer::unoptimizeReloc(SPAN_P(upx_byte) & in, SPAN_P(upx_byte) image, MemBuffer &out,
                                 bool bswap, int bits) {
    SPAN_P_VAR(upx_byte, p, in);
    unsigned relocn = 0;
    for (; *p; p++, relocn++)
        if (*p >= 0xF0) {
            if (*p == 0xF0 && get_le16(p + 1) == 0)
                p += 4;
            p += 2;
        }
    SPAN_P_VAR(upx_byte, const in_end, p);
    // fprintf(stderr,"relocnum=%x\n",relocn);
    out.alloc(4 * (relocn + 1)); // one extra entry
    SPAN_S_VAR(LE32, relocs, out);
    unsigned jc = (unsigned) -4;
    for (p = in; p < in_end; p++) {
        if (*p < 0xF0)
            jc += *p;
        else {
            unsigned dif = (*p & 0x0F) * 0x10000 + get_le16(p + 1);
            p += 2;
            if (dif == 0) {
                dif = get_le32(p + 1);
                p += 4;
            }
            jc += dif;
        }
        *relocs++ = jc; // FIXME: range check jc
        if (!relocn--) {
            break;
        }
        if (bswap && image != nullptr) {
            if (bits == 32) {
                set_be32(image + jc, get_le32(image + jc));
                if ((unsigned) ptr_diff_bytes(p, image + jc) < 4) {
                    // data must not overlap control
                    p = image + jc + (4 - 1); // -1: 'for' also increments
                }
            } else if (bits == 64) {
                set_be64(image + jc, get_le64(image + jc));
                if ((unsigned) ptr_diff_bytes(p, image + jc) < 8) {
                    // data must not overlap control
                    p = image + jc + (8 - 1); // -1: 'for' also increments
                }
            } else
                throwInternalError("unoptimizeReloc problem");
        }
    }
    in = p + 1;
    return ptr_udiff_bytes(relocs, out) / 4; // return number of relocs
}

unsigned Packer::unoptimizeReloc32(SPAN_P(upx_byte) & in, SPAN_P(upx_byte) image, MemBuffer &out,
                                   bool bswap) {
    return unoptimizeReloc(in, image, out, bswap, 32);
}

unsigned Packer::unoptimizeReloc64(SPAN_P(upx_byte) & in, SPAN_P(upx_byte) image, MemBuffer &out,
                                   bool bswap) {
    return unoptimizeReloc(in, image, out, bswap, 64);
}

/*************************************************************************
// loader util (interface to linker)
**************************************************************************/

static const char *getIdentstr(unsigned *size, int small) {
    // IMPORTANT: we do NOT change "http://upx.sf.net"
    static char identbig[] =
        "\n\0"
        "$Info: "
        "This file is packed with the UPX executable packer http://upx.sf.net $"
        "\n\0"
        "$Id: UPX " UPX_VERSION_STRING4 " Copyright (C) 1996-" UPX_VERSION_YEAR
        " the UPX Team. All Rights Reserved. $"
        "\n";
    static char identsmall[] =
        "\n"
        "$Id: UPX "
        "(C) 1996-" UPX_VERSION_YEAR " the UPX Team. All Rights Reserved. http://upx.sf.net $"
        "\n";
    static char identtiny[] = UPX_VERSION_STRING4;

    static int done;
    if (!done && (opt->debug.fake_stub_version[0] || opt->debug.fake_stub_year[0])) {
        struct strinfo_t {
            char *s;
            int size;
        };
        static const strinfo_t strlist[] = {{identbig, (int) sizeof(identbig)},
                                            {identsmall, (int) sizeof(identsmall)},
                                            {identtiny, (int) sizeof(identtiny)},
                                            {nullptr, 0}};
        const strinfo_t *iter;

        for (iter = strlist; iter->s; ++iter) {
            if (opt->debug.fake_stub_version[0])
                mem_replace(iter->s, iter->size, UPX_VERSION_STRING4, 4,
                            opt->debug.fake_stub_version);
            if (opt->debug.fake_stub_year[0])
                mem_replace(iter->s, iter->size, UPX_VERSION_YEAR, 4, opt->debug.fake_stub_year);
        }
        done = 1;
    }

    if (small < 0)
        small = opt->small;
    if (small >= 2) {
        *size = sizeof(identtiny);
        return identtiny;
    } else if (small >= 1) {
        *size = sizeof(identsmall);
        return identsmall;
    } else {
        *size = sizeof(identbig);
        return identbig;
    }
}

void Packer::initLoader(const void *pdata, int plen, int small, int pextra) {
    delete linker;
    linker = newLinker();
    assert(bele == linker->bele);
    linker->init(pdata, plen, pextra);

    unsigned size;
    char const *const ident = getIdentstr(&size, small);
    linker->addSection("IDENTSTR", ident, size, 0);
}

#define C const char *
#define N ACC_STATIC_CAST(void *, nullptr)
void Packer::addLoader(C a) { addLoaderVA(a, N); }
void Packer::addLoader(C a, C b) { addLoaderVA(a, b, N); }
void Packer::addLoader(C a, C b, C c) { addLoaderVA(a, b, c, N); }
void Packer::addLoader(C a, C b, C c, C d) { addLoaderVA(a, b, c, d, N); }
void Packer::addLoader(C a, C b, C c, C d, C e) { addLoaderVA(a, b, c, d, e, N); }
void Packer::addLoader(C a, C b, C c, C d, C e, C f) { addLoaderVA(a, b, c, d, e, f, N); }
void Packer::addLoader(C a, C b, C c, C d, C e, C f, C g) { addLoaderVA(a, b, c, d, e, f, g, N); }
void Packer::addLoader(C a, C b, C c, C d, C e, C f, C g, C h) {
    addLoaderVA(a, b, c, d, e, f, g, h, N);
}
void Packer::addLoader(C a, C b, C c, C d, C e, C f, C g, C h, C i) {
    addLoaderVA(a, b, c, d, e, f, g, h, i, N);
}
void Packer::addLoader(C a, C b, C c, C d, C e, C f, C g, C h, C i, C j) {
    addLoaderVA(a, b, c, d, e, f, g, h, i, j, N);
}
#undef C
#undef N

void Packer::addLoaderVA(const char *s, ...) {
    va_list ap;
    va_start(ap, s);
    linker->addLoader(s, ap);
    va_end(ap);
}

upx_byte *Packer::getLoader() const {
    int size = -1;
    upx_byte *oloader = linker->getLoader(&size);
    if (oloader == nullptr || size <= 0)
        throwBadLoader();
    return oloader;
}

int Packer::getLoaderSize() const {
    int size = -1;
    upx_byte *oloader = linker->getLoader(&size);
    if (oloader == nullptr || size <= 0)
        throwBadLoader();
    return size;
}

bool Packer::hasLoaderSection(const char *name) const {
    void *section = linker->findSection(name, false);
    return section != nullptr;
}

int Packer::getLoaderSection(const char *name, int *slen) const {
    int size = -1;
    int ostart = linker->getSection(name, &size);
    if (ostart < 0 || size <= 0)
        throwBadLoader();
    if (slen)
        *slen = size;
    return ostart;
}

// same, but the size of the section may be == 0
int Packer::getLoaderSectionStart(const char *name, int *slen) const {
    int size = -1;
    int ostart = linker->getSection(name, &size);
    if (ostart < 0 || size < 0)
        throwBadLoader();
    if (slen)
        *slen = size;
    return ostart;
}

void Packer::relocateLoader() {
    linker->relocate();

#if 0
    // "relocate" packheader
    if (linker->findSection("UPX1HEAD", false))
    {
        int lsize = -1;
        int loff = getLoaderSectionStart("UPX1HEAD", &lsize);
        assert(lsize == ph.getPackHeaderSize());
        unsigned char *p = getLoader() + loff;
        assert(get_le32(p) == UPX_MAGIC_LE32);
        //patchPackHeader(p, lsize);
        ph.putPackHeader(p);
    }
#endif
}

/*************************************************************************
//      void Packer::compressWithFilters():
// Try compression with several methods and filters, choose the best
/  or first working one. Needs buildLoader().
//
// Required inputs:
//   this->ph
//     ulen
//   parm_ft
//     clevel
//     addvalue
//     buf_len (optional)
//
// - updates this->ph
// - updates *ft
// - i_ptr[] is restored to the original unfiltered version
// - o_ptr[] contains the best compressed version
//
// filter_strategy:
//   n:  try the first N filters, use best one
//  -1:  try all filters, use first working one
//  -2:  try only the opt->filter filter
//  -3:  use no filter at all
//
// This has been prepared for generalization into class Packer so that
// opt->all_methods and/or opt->all_filters are available for all
// executable formats.
//
// It will replace the tryFilters() / compress() call sequence.
//
// 2006-02-15: hdr_buf and hdr_u_len are default empty input "header" array
// to fix a 2-pass problem with Elf headers.  As of today there can be
// only one decompression method per executable output file, and that method
// is the one that gives best compression for .text and loader.  However,
// the Elf headers precede .text in the output file, and are written first.
// "--brute" compression often compressed the Elf headers using nrv2b
// but the .text (and loader) with nrv2e.  This often resulted in SIGSEGV
// during decompression.
// The workaround is for hdr_buf and hdr_u_len to describe the Elf headers
// (typically less than 512 bytes) when .text is passed in, and include
// them in the calculation of shortest output.  Then the result
// this->ph.method  will say which [single] method to use for everything.
// The Elf headers are never filtered.  They are short enough (< 512 bytes)
// that compressing them more than once per method (once here when choosing,
// once again just before writing [because compressWithFilters discards])
// is OK because of the simplicity of not having two output arrays.
**************************************************************************/

int Packer::prepareMethods(int *methods, int ph_method, const int *all_methods) const {
    int nmethods = 0;
    if (!opt->all_methods || all_methods == nullptr || (-0x80 == (ph_method >> 24))) {
        methods[nmethods++] = forced_method(ph_method);
        return nmethods;
    }
    for (int mm = 0; all_methods[mm] != M_END; ++mm) {
        int method = all_methods[mm];
        if (method == M_ULTRA_BRUTE && !opt->ultra_brute)
            break;
        if (method == M_SKIP || method == M_ULTRA_BRUTE)
            continue;
        if (opt->all_methods && opt->all_methods_use_lzma != 1 && M_IS_LZMA(method))
            continue;
        // use this method
        assert(Packer::isValidCompressionMethod(method));
        methods[nmethods++] = method;
    }
    return nmethods;
}

static int prepareFilters(int *filters, int &filter_strategy, const int *all_filters) {
    int nfilters = 0;

    // setup filter filter_strategy
    if (filter_strategy == 0) {
        if (opt->all_filters)
            // choose best from all available filters
            filter_strategy = INT_MAX;
        else if (opt->filter >= 0 && Filter::isValidFilter(opt->filter, all_filters))
            // try opt->filter
            filter_strategy = -2;
        else
            // try the first working filter
            filter_strategy = -1;
    }
    assert(filter_strategy != 0);

    if (filter_strategy == -3)
        goto done;
    if (filter_strategy == -2) {
        if (opt->filter >= 0 && Filter::isValidFilter(opt->filter, all_filters)) {
            filters[nfilters++] = opt->filter;
            goto done;
        }
        filter_strategy = -1;
    }
    assert(filter_strategy >= -1);

    while (all_filters && *all_filters != FT_END) {
        int filter_id = *all_filters++;
        if (filter_id == FT_ULTRA_BRUTE && !opt->ultra_brute)
            break;
        if (filter_id == FT_SKIP || filter_id == FT_ULTRA_BRUTE)
            continue;
        if (filter_id == 0)
            continue;
        // use this filter
        assert(Filter::isValidFilter(filter_id));
        filters[nfilters++] = filter_id;
        if (filter_strategy >= 0 && nfilters >= filter_strategy)
            break;
    }

done:
    // filter_strategy now only means "stop after first successful filter"
    filter_strategy = (filter_strategy < 0) ? -1 : 0;
    // make sure that we have a "no filter" fallback
    for (int i = 0; i < nfilters; i++)
        if (filters[i] == 0)
            return nfilters;
    filters[nfilters++] = 0;
    return nfilters;
}

void Packer::compressWithFilters(upx_bytep i_ptr,
                                 unsigned const i_len,  // written and restored by filters
                                 upx_bytep const o_ptr, // where to put compressed output
                                 upx_bytep f_ptr,
                                 unsigned const f_len, // subset of [*i_ptr, +i_len)
                                 upx_bytep const hdr_ptr, unsigned const hdr_len,
                                 Filter *const parm_ft, // updated
                                 unsigned const overlap_range,
                                 upx_compress_config_t const *const cconf,
                                 int filter_strategy, // in+out for prepareFilters
                                 bool const inhibit_compression_check) {
    parm_ft->buf_len = f_len;
    // struct copies
    const PackHeader orig_ph = this->ph;
    PackHeader best_ph = this->ph;
    const Filter orig_ft = *parm_ft;
    Filter best_ft = *parm_ft;
    //
    best_ph.c_len = i_len;
    best_ph.overlap_overhead = 0;
    unsigned best_ph_lsize = 0;
    unsigned best_hdr_c_len = 0;

    // preconditions
    assert(orig_ph.filter == 0);
    assert(orig_ft.id == 0);

    // prepare methods and filters
    int methods[256];
    int nmethods = prepareMethods(methods, ph.method, getCompressionMethods(M_ALL, ph.level));
    assert(nmethods > 0);
    assert(nmethods < 256);
    int filters[256];
    int nfilters = prepareFilters(filters, filter_strategy, getFilters());
    assert(nfilters > 0);
    assert(nfilters < 256);
#if 0
    printf("compressWithFilters: m(%d):", nmethods);
    for (int i = 0; i < nmethods; i++) printf(" %#x", methods[i]);
    printf(" f(%d):", nfilters);
    for (int i = 0; i < nfilters; i++) printf(" %#x", filters[i]);
    printf("\n");
#endif

    // update total_passes; previous (ui_total_passes > 0) means incremental
    if (!is_forced_method(ph.method)) {
        if (uip->ui_total_passes > 0)
            uip->ui_total_passes -= 1;
        if (filter_strategy < 0)
            uip->ui_total_passes += nmethods;
        else
            uip->ui_total_passes += nfilters * nmethods;
    }

    // Working buffer for compressed data. Don't waste memory and allocate as needed.
    upx_bytep o_tmp = o_ptr;
    MemBuffer o_tmp_buf;

    // compress using all methods/filters
    int nfilters_success_total = 0;
    for (int mm = 0; mm < nmethods; mm++) // for all methods
    {
#if 0  //{
        printf("\nmethod %d (%d of %d)\n", methods[mm], 1+ mm, nmethods);
#endif //}
        assert(isValidCompressionMethod(methods[mm]));
        unsigned hdr_c_len = 0;
        if (hdr_ptr != nullptr && hdr_len) {
            if (nfilters_success_total != 0 && o_tmp == o_ptr) {
                // do not overwrite o_ptr
                o_tmp_buf.allocForCompression(UPX_MAX(hdr_len, i_len));
                o_tmp = o_tmp_buf;
            }
            int r = upx_compress(hdr_ptr, hdr_len, o_tmp, &hdr_c_len, nullptr, methods[mm], 10,
                                 nullptr, nullptr);
            if (r != UPX_E_OK)
                throwInternalError("header compression failed");
            if (hdr_c_len >= hdr_len)
                throwInternalError("header compression size increase");
        }
        int nfilters_success_mm = 0;
        for (int ff = 0; ff < nfilters; ff++) // for all filters
        {
            assert(isValidFilter(filters[ff]));
            // get fresh packheader
            ph = orig_ph;
            ph.method = methods[mm];
            ph.filter = filters[ff];
            ph.overlap_overhead = 0;
            // get fresh filter
            Filter ft = orig_ft;
            ft.init(ph.filter, orig_ft.addvalue);
            // filter
            optimizeFilter(&ft, f_ptr, f_len);
            bool success = ft.filter(f_ptr, f_len);
            if (ft.id != 0 && ft.calls == 0) {
                // filter did not do anything - no need to call ft.unfilter()
                success = false;
            }
            if (!success) {
                // filter failed or was useless
                if (filter_strategy >= 0) {
                    // adjust ui passes
                    if (uip->ui_pass >= 0)
                        uip->ui_pass++;
                }
                continue;
            }
            // filter success
#if 0
            printf("\nfilter: id 0x%02x size %6d, calls %5d/%5d/%3d/%5d/%5d, cto 0x%02x\n",
                   ft.id, ft.buf_len, ft.calls, ft.noncalls, ft.wrongcalls, ft.firstcall, ft.lastcall, ft.cto);
#endif
            if (nfilters_success_total != 0 && o_tmp == o_ptr) {
                o_tmp_buf.allocForCompression(i_len);
                o_tmp = o_tmp_buf;
            }
            nfilters_success_total++;
            nfilters_success_mm++;
            ph.filter_cto = ft.cto;
            ph.n_mru = ft.n_mru;
            // compress
            if (compress(i_ptr, i_len, o_tmp, cconf)) {
                unsigned lsize = 0;
                // findOverlapOperhead() might be slow; omit if already too big.
                if (ph.c_len + lsize + hdr_c_len <=
                    best_ph.c_len + best_ph_lsize + best_hdr_c_len) {
                    // get results
                    ph.overlap_overhead = findOverlapOverhead(o_tmp, i_ptr, overlap_range);
                    buildLoader(&ft);
                    lsize = getLoaderSize();
                    assert(lsize > 0);
                }
#if 0  //{
                printf("\n%2d %02x: %d +%4d +%3d = %d  (best: %d +%4d +%3d = %d)\n", ph.method, ph.filter,
                       ph.c_len, lsize, hdr_c_len, ph.c_len + lsize + hdr_c_len,
                       best_ph.c_len, best_ph_lsize, best_hdr_c_len, best_ph.c_len + best_ph_lsize + best_hdr_c_len);
#endif //}
                bool update = false;
                if (ph.c_len + lsize + hdr_c_len < best_ph.c_len + best_ph_lsize + best_hdr_c_len)
                    update = true;
                else if (ph.c_len + lsize + hdr_c_len ==
                         best_ph.c_len + best_ph_lsize + best_hdr_c_len) {
                    // prefer smaller loaders
                    if (lsize + hdr_c_len < best_ph_lsize + best_hdr_c_len)
                        update = true;
                    else if (lsize + hdr_c_len == best_ph_lsize + best_hdr_c_len) {
                        // prefer less overlap_overhead
                        if (ph.overlap_overhead < best_ph.overlap_overhead)
                            update = true;
                    }
                }
                if (update) {
                    assert((int) ph.overlap_overhead > 0);
                    // update o_ptr[] with best version
                    if (o_tmp != o_ptr)
                        memcpy(o_ptr, o_tmp, ph.c_len);
                    // save compression results
                    best_ph = ph;
                    best_ph_lsize = lsize;
                    best_hdr_c_len = hdr_c_len;
                    best_ft = ft;
                }
            }
            // restore - unfilter with verify
            ft.unfilter(f_ptr, f_len, true);
            if (filter_strategy < 0)
                break;
        }
        assert(nfilters_success_mm > 0);
    }

    // postconditions 1)
    assert(nfilters_success_total > 0);
    assert(best_ph.u_len == orig_ph.u_len);
    assert(best_ph.filter == best_ft.id);
    assert(best_ph.filter_cto == best_ft.cto);
    // FIXME  assert(best_ph.n_mru == best_ft.n_mru);

    // copy back results
    this->ph = best_ph;
    *parm_ft = best_ft;

    // Finally, check compression ratio.
    // Might be inhibited when blocksize < file_size, for instance.
    if (!inhibit_compression_check) {
        if (best_ph.c_len + best_ph_lsize >= best_ph.u_len)
            throwNotCompressible();
        if (!checkCompressionRatio(best_ph.u_len, best_ph.c_len))
            throwNotCompressible();

        // postconditions 2)
        assert(best_ph.overlap_overhead > 0);
    }

    // convenience
    buildLoader(&best_ft);
}

/*************************************************************************
//
**************************************************************************/

void Packer::compressWithFilters(Filter *ft, const unsigned overlap_range,
                                 const upx_compress_config_t *cconf, int filter_strategy,
                                 bool inhibit_compression_check) {
    // call the subroutine immediately below
    compressWithFilters(ft, overlap_range, cconf, filter_strategy, 0, 0, 0, nullptr, 0,
                        inhibit_compression_check);
}

void Packer::compressWithFilters(Filter *ft, const unsigned overlap_range,
                                 upx_compress_config_t const *cconf, int filter_strategy,
                                 unsigned filter_off, unsigned ibuf_off, unsigned obuf_off,
                                 upx_bytep const hdr_ptr, unsigned hdr_len,
                                 bool inhibit_compression_check) {
    ibuf.checkState();
    obuf.checkState();

    upx_bytep i_ptr = ibuf + ibuf_off;
    unsigned i_len = ph.u_len;
    upx_bytep o_ptr = obuf + obuf_off;
    upx_bytep f_ptr = ibuf + filter_off;
    unsigned f_len = ft->buf_len ? ft->buf_len : i_len;

    assert(f_ptr + f_len <= i_ptr + i_len);

    // call the first one in this file
    compressWithFilters(i_ptr, i_len, o_ptr, f_ptr, f_len, hdr_ptr, hdr_len, ft, overlap_range,
                        cconf, filter_strategy, inhibit_compression_check);

    ibuf.checkState();
    obuf.checkState();
}

/* vim:set ts=4 sw=4 et: */
