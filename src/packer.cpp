/* packer.cpp --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2000 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2000 Laszlo Molnar

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

   Markus F.X.J. Oberhumer                   Laszlo Molnar
   markus.oberhumer@jk.uni-linz.ac.at        ml1050@cdata.tvnet.hu
 */


//#define WANT_STL
#include "conf.h"
#include "file.h"
#include "packer.h"
#include "filter.h"
#include "linker.h"
#include "ui.h"


/*************************************************************************
//
**************************************************************************/

Packer::Packer(InputFile *f) :
    fi(f), file_size(-1), ph_format(-1), ph_version(-1),
    uip(NULL), pass(0), total_passes(0), linker(NULL),
    last_patch(NULL), last_patch_offset(0)
{
    file_size = f->st.st_size;
    uip = new UiPacker(this);
    memset(&ph,0,sizeof(ph));
}


Packer::~Packer()
{
    delete uip; uip = NULL;
    delete linker; linker = NULL;
}


/*************************************************************************
// public entries called from class PackMaster
**************************************************************************/

void Packer::doPack(OutputFile *fo)
{
    uip->uiPackStart(fo);
    pack(fo);
    uip->uiPackEnd(fo);
}

void Packer::doUnpack(OutputFile *fo)
{
    uip->uiUnpackStart(fo);
    unpack(fo);
    uip->uiUnpackEnd(fo);
}

void Packer::doTest()
{
    uip->uiTestStart();
    test();
    uip->uiTestEnd();
}

void Packer::doList()
{
    uip->uiListStart();
    list();
    uip->uiListEnd();
}

void Packer::doFileInfo()
{
    uip->uiFileInfoStart();
    fileInfo();
    uip->uiFileInfoEnd();
}


/*************************************************************************
// default actions
**************************************************************************/

void Packer::test()
{
    unpack(NULL);
}


void Packer::list()
{
    uip->uiList();
}


void Packer::fileInfo()
{
    // FIXME: subclasses should list their sections here
    // We also should try to get a nice layout...
}


bool Packer::testUnpackVersion(int version) const
{
    if (version != ph_version && ph_version != -1)
        throwCantUnpack("program has been modified; run a virus checker!");
    if (!canUnpackVersion(version))
        throwCantUnpack("I am not compatible with older versions of UPX");
    return true;
}


bool Packer::testUnpackFormat(int format) const
{
    if (format != ph_format && ph_format != -1)
        throwCantUnpack("program has been modified; run a virus checker!");
    return canUnpackFormat(format);
}


/*************************************************************************
// compress
**************************************************************************/

bool Packer::compress(upx_bytep in, upx_bytep out,
                      unsigned max_offset, unsigned max_match)
{
#if defined(UNUPX)
    throwInternalError("compression failed");
    return false;
#else
    ph.c_len = 0;

    const int level = ph.level;
    assert(level >= 1); assert(level <= 10);

    // update checksum of uncompressed data
    unsigned saved_u_adler = ph.u_adler;
    ph.u_adler = upx_adler32(ph.u_adler,in,ph.u_len);

    // set compression paramters
    upx_compress_config_t conf;
    memset(&conf, 0xff, sizeof(conf));
    // arguments
    if (max_offset != 0)
        conf.max_offset = max_offset;
    if (max_match != 0)
        conf.max_match = max_match;
    // options
    if (opt->crp.c_flags != -1)
        conf.c_flags = opt->crp.c_flags;
#if 0
    else
        // this is based on experimentation
        conf.c_flags = (level >= 7) ? 0 : 1 | 2;
#endif
    if (opt->crp.p_level != -1)
        conf.p_level = opt->crp.p_level;
    if (opt->crp.h_level != -1)
        conf.h_level = opt->crp.h_level;
    else if (level >= 7)
    {
        static const int h_level[9] =
            { 0, 17, 18, 19, 20, 4369, 4626, 4883, 5140 };
        int ml = opt->mem_level;
        if (ml < 0)
            ml = 9;
        if (ml < 1 || ml > 9)
            ml = 1;
        conf.h_level = h_level[ml - 1];
    }
    if (opt->crp.max_offset != UPX_UINT_MAX && opt->crp.max_offset < conf.max_offset)
        conf.max_offset = opt->crp.max_offset;
    if (opt->crp.max_match != UPX_UINT_MAX && opt->crp.max_match < conf.max_match)
        conf.max_match = opt->crp.max_match;

    // Avoid too many progress bar updates. 64 is s->bar_len in ui.cpp.
    unsigned step = (ph.u_len < 64*1024) ? 0 : ph.u_len / 64;
#if defined(WITH_NRV)
    if ((level >= 7) || (level >= 4 && ph.u_len >= 512*1024))
        step = 0;
#endif
    if (this->pass >= 0)
        this->pass++;
    uip->startCallback(ph.u_len, step, this->pass, this->total_passes);
    uip->firstCallback();

    //OutputFile::dump("data.raw", in, ph.u_len);

    // compress
    unsigned result[16];
    memset(result, 0, sizeof(result));
    int r = upx_compress(in, ph.u_len, out, &ph.c_len,
                         uip->getCallback(),
                         ph.method, level, &conf, result);
    if (r == UPX_E_OUT_OF_MEMORY)
        throwCantPack("out of memory");
    if (r != UPX_E_OK)
        throwInternalError("compression failed");

    //ph.min_offset_found = result[0];
    ph.max_offset_found = result[1];
    //ph.min_match_found = result[2];
    ph.max_match_found = result[3];
    //ph.min_run_found = result[4];
    ph.max_run_found = result[5];
    ph.first_offset_found = result[6];
    //ph.same_match_offsets_found = result[7];
    assert(max_offset == 0 || max_offset >= ph.max_offset_found);
    assert(max_match == 0 || max_match >= ph.max_match_found);

    uip->endCallback();

    //printf("Packer::compress: %d/%d: %7d -> %7d\n", ph.method, ph.level, ph.u_len, ph.c_len);
    if (!checkCompressionRatio(ph.c_len, ph.u_len))
        return false;
    // return in any case if not compressible
    if (ph.c_len >= ph.u_len)
        return false;

    // update checksum of compressed data
    ph.c_adler = upx_adler32(ph.c_adler,out,ph.c_len);
    // Decompress and verify. Skip this when using the fastest level.
    if (level > 1)
    {
        // decompress
        unsigned new_len = ph.u_len;
        r = upx_decompress(out,ph.c_len,in,&new_len,ph.method);
        //printf("%d %d: %d %d %d\n", ph.method, r, ph.c_len, ph.u_len, new_len);
        if (r != UPX_E_OK || new_len != ph.u_len)
            throwInternalError("decompression failed");

        // verify decompression
        if (ph.u_adler != upx_adler32(saved_u_adler,in,ph.u_len))
            throwInternalError("decompression failed (checksum error)");
    }
    return true;
#endif /* UNUPX */
}


bool Packer::checkCompressionRatio(unsigned c_len, unsigned u_len) const
{
#if 1
    if (c_len >= u_len - u_len / 8)         // min. 12.5% gain
        return false;
    if (c_len + 512 >= u_len)               // min. 512 bytes gain
        return false;
#else
    if (c_len >= u_len)
        return false;
#endif
    return true;
}


/*************************************************************************
//
**************************************************************************/

void Packer::decompress(const upx_bytep in, upx_bytep out,
                        bool verify_checksum)
{
    unsigned adler;

    // verify checksum of compressed data
    if (verify_checksum)
    {
        adler = upx_adler32(0,NULL,0);
        adler = upx_adler32(adler,in,ph.c_len);
        if (adler != ph.c_adler)
            throwChecksumError();
    }

    // decompress
    unsigned new_len = ph.u_len;
    int r = upx_decompress(in,ph.c_len,out,&new_len,ph.method);
    if (r != UPX_E_OK || new_len != ph.u_len)
        throwCompressedDataViolation();

    // verify checksum of decompressed data
    if (verify_checksum)
    {
        adler = upx_adler32(0,NULL,0);
        adler = upx_adler32(adler,out,ph.u_len);
        if (adler != ph.u_adler)
            throwChecksumError();
    }
}


/*************************************************************************
//
**************************************************************************/

bool Packer::testOverlappingDecompression(const upx_bytep buf,
                                          unsigned overlap_overhead) const
{
#if defined(UNUPX)
    UNUSED(buf);
    UNUSED(overlap_overhead);
    return false;
#else
    if (ph.c_len >= ph.u_len)
        return false;

    assert((int)overlap_overhead >= 0);
    // Because we are not using the asm_fast decompressor here
    // we must account for extra 3 bytes or else we may fail
    // at UPX decompression time.
    if (overlap_overhead <= 4 + 3)  // don't waste time here
        return false;
    overlap_overhead -= 3;

    unsigned src_off = ph.u_len + overlap_overhead - ph.c_len;
    unsigned new_len = ph.u_len;
    int r = upx_test_overlap(buf - src_off, src_off,
                             ph.c_len, &new_len, ph.method);
    return (r == UPX_E_OK && new_len == ph.u_len);
#endif /* UNUPX */
}


void Packer::verifyOverlappingDecompression(MemBuffer *buf,
                                            unsigned overlap_overhead)
{
#if defined(UNUPX)
#else
    // FIXME (well, this is complete paranoia anyway)
    //
    // Idea:
    //   buf was allocated with MemBuffer.allocForCompression(), and
    //   its contents are no longer needed, i.e. the compressed data
    //   must have been already written.
    //   We now could performa a real overlapping decompression and
    //   verify the checksum.
    //
    // Note:
    //   This verify is just because of complete paranoia that there
    //   could be a hidden bug in the upx_test_overlap implementation,
    //   and it should not be necessary at all.
    //
    // See also:
    //   Filter::verifyUnfilter()
#endif
    UNUSED(buf);
    UNUSED(overlap_overhead);
}


/*************************************************************************
// Find overhead for in-place decompression in an heuristic way
// (using a binary search). Return 0 on error.
//
// To speed up things:
//   - you can pass the range of an acceptable interval (so that
//     we can succeed early)
//   - you can enforce an upper_limit (so that we can fail early)
**************************************************************************/

unsigned Packer::findOverlapOverhead(const upx_bytep buf,
                                     unsigned range,
                                     unsigned upper_limit) const
{
#if defined(UNUPX)
    throwInternalError("not implemented");
    return 0;
#else
    assert((int) range >= 0);

    // prepare to deal with very pessimistic values
    unsigned low = 1;
    unsigned high = UPX_MIN(ph.u_len / 4 + 512, upper_limit);
    // but be optimistic for first try (speedup)
    unsigned m = UPX_MIN(16, high);
    //
    unsigned overhead = 0;
    unsigned nr = 0;          // statistics

    while (high >= low)
    {
        assert(m >= low); assert(m <= high);
        assert(m < overhead || overhead == 0);
        nr++;
        if (testOverlappingDecompression(buf, m))
        {
            overhead = m;
            // Succeed early if m lies in [low .. low+range-1], i.e. if
            // if the range of the current interval is <= range.
            //if (m <= low + range - 1)
            if (m + 1 <= low + range)   // avoid underflow
                break;
            high = m - 1;
        }
        else
            low = m + 1;
        m = (low + high) / 2;
    }

    //printf("findOverlapOverhead: %d (%d tries)\n", overhead, nr);
    if (overhead == 0)
        throwInternalError("this is an oo bug");

    UNUSED(nr);
    return overhead;
#endif /* UNUPX */
}


/*************************************************************************
// file io utils
**************************************************************************/

void Packer::handleStub(InputFile *fif, OutputFile *fo, long size)
{
    if (fo)
    {
        if (size > 0)
        {
            // copy stub from exe
            info("Copying original stub: %ld bytes", size);
            autoheap_array(char, stub, size);
            fif->seek(0,SEEK_SET);
            fif->readx(stub,size);
            fo->write(stub,size);
        }
        else
        {
            // no stub
        }
    }
}


void Packer::checkOverlay(unsigned overlay)
{
    assert((int)overlay >= 0);
    assert((off_t)overlay < file_size);
    if (overlay == 0)
        return;
    info("Found overlay: %d bytes", overlay);
    if (opt->overlay == opt->SKIP_OVERLAY)
        throw OverlayException("file has overlay -- skipped; try `--overlay=copy'");
}


void Packer::copyOverlay(OutputFile *fo, unsigned overlay,
                         MemBuffer *buf,
                         bool do_seek)
{
    assert((int)overlay >= 0);
    assert((off_t)overlay < file_size);
    if (!fo || overlay == 0)
        return;
    if (opt->overlay != opt->COPY_OVERLAY)
    {
        assert(opt->overlay == opt->STRIP_OVERLAY);
        infoWarning("stripping overlay: %d bytes", overlay);
        return;
    }
    info("Copying overlay: %d bytes", overlay);
    if (do_seek)
        fi->seek(-(long)overlay, SEEK_END);

    // get buffer size, align to improve i/o speed
    unsigned buf_size = buf->getSize();
    if (buf_size >= 65536)
        buf_size = ALIGN_DOWN(buf_size, 4096);
    assert((int)buf_size > 0);

    do {
        unsigned len = overlay < buf_size ? overlay : buf_size;
        fi->readx(*buf, len);
        fo->write(*buf, len);
        overlay -= len;
    } while (overlay > 0);
}


// Create a pseudo-unique program id.
unsigned Packer::getRandomId() const
{
    unsigned id = 0;
#if 0 && defined(__unix__)
    // Don't consume precious bytes from /dev/urandom.
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0)
        fd = open("/dev/random", O_RDONLY);
    if (fd >= 0)
    {
        if (read(fd, &id, 4) != 4)
            id = 0;
        close(fd);
    }
#endif
    while (id == 0)
    {
#if !defined(HAVE_GETTIMEOFDAY) || defined(__DJGPP__)
        id ^= (unsigned) time(NULL);
        id ^= ((unsigned) clock()) << 12;
#else
        struct timeval tv;
        gettimeofday(&tv, 0);
        id ^= (unsigned) tv.tv_sec;
        id ^= ((unsigned) tv.tv_usec) << 12;  // shift into high-bits
#endif
#if defined(HAVE_GETPID)
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
void Packer::initPackHeader()
{
    memset(&ph, 0, sizeof(ph));
    ph.magic = UPX_MAGIC_LE32;                    // "UPX!"
    ph.version = getVersion();
    ph.format = getFormat();
    ph.method = -1;
    ph.level = -1;
    ph.c_adler = ph.u_adler = upx_adler32(0,NULL,0);
    ph.buf_offset = -1;
    ph.u_file_size = file_size;
}

// this is called directly after canPack() from class PackMaster
void Packer::updatePackHeader()
{
    ph.method = getCompressionMethod();
    ph.level = getCompressionLevel();
    // update our local options
    opt->method = ph.method;
    opt->level = ph.level;
}


void Packer::putPackHeader(upx_bytep buf, unsigned len)
{
    assert(isValidFilter(ph.filter));
    ph.putPackHeader(buf, len);
}


bool Packer::readPackHeader(unsigned len, off_t seek_offset, upx_byte *buf)
{
    assert((int)len > 0);

    MemBuffer hbuf;
    if (buf == NULL)
    {
        hbuf.alloc(len);
        buf = hbuf;
    }
    memset(buf, 0, len);

    if (seek_offset != -1)
    {
        if (seek_offset >= 0)
            fi->seek(seek_offset, SEEK_SET);
    }
    len = fi->read(buf,len);

    if (!ph.fillPackHeader(buf, len))
        return false;
    if (!ph.checkPackHeader(buf + ph.buf_offset, len - ph.buf_offset))
        return false;

    if (ph.version > getVersion())
        throwCantUnpack("need a newer version of UPX");
    // Some formats might be able to unpack old versions because
    // their implementation hasn't changed. Ask them.
    if (opt->cmd != CMD_FILEINFO)
        if (!testUnpackVersion(ph.version))
            return false;

    if (ph.c_len >= ph.u_len || (off_t)ph.c_len >= file_size
        || ph.version <= 0 || ph.version >= 0xff)
        throwCantUnpack("header corrupted");
    else if ((off_t)ph.u_len > ph.u_file_size)
    {
#if 0
        // FIXME: does this check make sense w.r.t. overlays ???
        if (ph.format == UPX_F_W32_PE || ph.format == UPX_F_DOS_EXE)
            // may get longer
            ((void)0);
        else
            throwCantUnpack("header size corrupted");
#endif
    }
    if (ph.method < M_NRV2B_LE32 || ph.method > M_NRV2D_LE16)
        throwCantUnpack("unknown compression method");

    // Some formats might be able to unpack "subformats". Ask them.
    if (!testUnpackFormat(ph.format))
        return false;

    return true;
}


/*************************************************************************
// patch util for loader
**************************************************************************/

void Packer::checkPatch(void *l, void *p, int size)
{
    if (l == NULL && p == NULL && size == 0)
    {
        // reset
        last_patch = NULL;
        last_patch_offset = 0;
        return;
    }
    if (l == NULL || p == NULL || p < l || size <= 0)
        throwBadLoader();
    long offset = (upx_bytep) p - (upx_bytep) l;
    //printf("checkPatch: %p %5ld %d\n", l, offset, size);
    if (l == last_patch)
    {
        if (offset + size > last_patch_offset)
            throwInternalError("invalid patch order");
    }
    else
        last_patch = l;
    last_patch_offset = offset;
}


void Packer::patch_be16(void *l, int llen, unsigned old, unsigned new_)
{
    void *p = find_be16(l,llen,old);
    checkPatch(l,p,2);
    set_be16(p,new_);
}


void Packer::patch_be16(void *l, int llen, const void * old, unsigned new_)
{
    void *p = find(l,llen,old,2);
    checkPatch(l,p,2);
    set_be16(p,new_);
}


void Packer::patch_be32(void *l, int llen, unsigned old, unsigned new_)
{
    void *p = find_be32(l,llen,old);
    checkPatch(l,p,4);
    set_be32(p,new_);
}


void Packer::patch_be32(void *l, int llen, const void * old, unsigned new_)
{
    void *p = find(l,llen,old,4);
    checkPatch(l,p,4);
    set_be32(p,new_);
}


void Packer::patch_le16(void *l, int llen, unsigned old, unsigned new_)
{
    void *p = find_le16(l,llen,old);
    checkPatch(l,p,2);
    set_le16(p,new_);
}


void Packer::patch_le16(void *l, int llen, const void * old, unsigned new_)
{
    void *p = find(l,llen,old,2);
    checkPatch(l,p,2);
    set_le16(p,new_);
}


void Packer::patch_le32(void *l, int llen, unsigned old, unsigned new_)
{
    void *p = find_le32(l,llen,old);
    checkPatch(l,p,4);
    set_le32(p,new_);
}


void Packer::patch_le32(void *l, int llen, const void * old, unsigned new_)
{
    void *p = find(l,llen,old,4);
    checkPatch(l,p,4);
    set_le32(p,new_);
}


// patch version into stub/ident_n.ash
void Packer::patchVersion(void *l, int llen)
{
    upx_byte *p = find(l,llen,"$Id: UPX UPXV ",14);
    checkPatch(l,p,14);
    unsigned char buf[4+1];
    memset(buf, ' ', 4);
    size_t len = UPX_MIN(strlen(UPX_VERSION_STRING), 4);
    memcpy(buf, UPX_VERSION_STRING, len);
    memcpy(p + 9, buf, 4);
}


/*************************************************************************
// relocation util
**************************************************************************/

upx_byte *Packer::optimizeReloc32(upx_byte *in, unsigned relocnum,
                                  upx_byte *out, upx_byte *image,
                                  int bswap, int *big)
{
#if defined(UNUPX)
    return out;
#else
    *big = 0;
    if (relocnum == 0)
        return out;
    qsort(in,relocnum,4,le32_compare);

    unsigned jc,pc,oc;
    upx_byte *fix = out;

    pc = (unsigned) -4;
    for (jc = 0; jc<relocnum; jc++)
    {
        oc = get_le32(in+jc*4) - pc;
        if (oc == 0)
            continue;
        else if (oc < 4)
            throwCantPack("overlapping fixups");
        else if (oc < 0xF0)
            *fix++ = (unsigned char) oc;
        else if (oc < 0x100000)
        {
            *fix++ = (unsigned char) (0xF0+(oc>>16));
            *fix++ = (unsigned char) oc;
            *fix++ = (unsigned char) (oc>>8);
        }
        else
        {
            *big = 1;
            *fix++ = 0xf0;
            *fix++ = 0;
            *fix++ = 0;
            set_le32(fix,oc);
            fix += 4;
        }
        pc += oc;
        if (bswap)
            set_be32(image+pc,get_le32(image+pc));
    }
    *fix++ = 0;
    return fix;
#endif /* UNUPX */
}


unsigned Packer::unoptimizeReloc32(upx_byte **in, upx_byte *image,
                                   MemBuffer *out, int bswap)
{
    upx_byte *p;
    unsigned relocn = 0;
    for (p = *in; *p; p++, relocn++)
        if (*p >= 0xF0)
        {
            if (*p == 0xF0 && get_le16(p+1) == 0)
                p += 4;
            p += 2;
        }
    //fprintf(stderr,"relocnum=%x\n",relocn);
    out->alloc(4*relocn+4); // one extra data
    LE32 *outp = (LE32*) (unsigned char *) *out;
    LE32 *relocs = outp;
    unsigned jc = (unsigned) -4;
    for (p = *in; *p; p++)
    {
        if (*p < 0xF0)
            jc += *p;
        else
        {
            unsigned dif = (*p & 0x0F)*0x10000 + get_le16(p+1);
            p += 2;
            if (dif == 0)
            {
                dif = get_le32(p+1);
                p += 4;
            }
            jc += dif;
        }
        *relocs++ = jc;
        if (bswap && image)
            set_be32(image+jc,get_le32(image+jc));
    }
    //fprintf(stderr,"relocnum=%x\n",relocn);
    *in = p+1;
    return relocs - outp;
}


/*************************************************************************
// loader util
**************************************************************************/

void Packer::initLoader(const void *pdata, int plen, int pinfo)
{
    delete linker;
    if (pinfo < 0)
        pinfo =  ~3 & (3 + get_le16(pdata, plen - 2));
    linker = new Linker(pdata,plen,pinfo);

    static const char identbig[] =
        "\n\0"
        "$Info: This file is packed with the UPX executable packer http://upx.tsx.org $"
        "\n\0"
        "$Id: UPX " UPX_VERSION_STRING " Copyright (C) 1996-2000 the UPX Team. All Rights Reserved. $"
        "\n";

    static const char identsmall[] =
        "\n"
        "$Id: UPX (C) 1996-2000 the UPX Team. All Rights Reserverd. http://upx.tsx.org $"
        "\n";

    if (opt->small)
        addSection("IDENTSTR",identsmall,sizeof(identsmall));
    else
        addSection("IDENTSTR",identbig,sizeof(identbig));
}


void Packer::addLoader(const char *s, ...)
{
    const char *p;
    va_list ap;

    linker->addSection(s);
    va_start(ap, s);
    while((p = va_arg(ap, const char *)) != NULL)
        linker->addSection(p);
    va_end(ap);
}

void Packer::addSection(const char *sname, const char *sdata, unsigned len)
{
    linker->addSection(sname,sdata,len);
}

int Packer::getLoaderSection(const char *name, int *slen)
{
    return linker->getSection(name,slen);
}


const upx_byte *Packer::getLoader() const
{
    return (const upx_byte *) linker->getLoader(NULL);
}


int Packer::getLoaderSize() const
{
    int l;
    (void) linker->getLoader(&l);
    return l;
}


const char *Packer::getDecompressor() const
{
    static const char nrv2b_le32_small[] =
        "N2BSMA10""N2BDEC10""N2BSMA20""N2BDEC20""N2BSMA30"
        "N2BDEC30""N2BSMA40""N2BSMA50""N2BDEC50""N2BSMA60"
        "N2BDEC60";

    static const char nrv2b_le32_fast[] =
        "N2BFAS10""+80CXXXX""N2BFAS11""N2BDEC10""N2BFAS20"
        "N2BDEC20""N2BFAS30""N2BDEC30""N2BFAS40""N2BFAS50"
        "N2BDEC50""N2BFAS60""+40CXXXX""N2BFAS61""N2BDEC60";

    static const char nrv2d_le32_small[] =
        "N2DSMA10""N2DDEC10""N2DSMA20""N2DDEC20""N2DSMA30"
        "N2DDEC30""N2DSMA40""N2DSMA50""N2DDEC50""N2DSMA60"
        "N2DDEC60";

    static const char nrv2d_le32_fast[] =
        "N2DFAS10""+80CXXXX""N2DFAS11""N2DDEC10""N2DFAS20"
        "N2DDEC20""N2DFAS30""N2DDEC30""N2DFAS40""N2DFAS50"
        "N2DDEC50""N2DFAS60""+40CXXXX""N2DFAS61""N2DDEC60";

    if (ph.method == M_NRV2B_LE32)
        return opt->small ? nrv2b_le32_small : nrv2b_le32_fast;
    if (ph.method == M_NRV2D_LE32)
        return opt->small ? nrv2d_le32_small : nrv2d_le32_fast;
    return NULL;
}


void Packer::addFilter32(int filter_id)
{
    assert(filter_id > 0);
    assert(isValidFilter(filter_id));

    if ((filter_id & 0xf) % 3 == 0)
        addLoader("CALLTR00",
                  (filter_id > 0x20) ? "CTCLEVE1" : "",
                  "CALLTR01",
                  (filter_id & 0xf) > 3 ? (filter_id > 0x20 ? "CTBSHR01""CTBSWA01" : "CTBROR01""CTBSWA01") : "",
                  "CALLTR02",
                  NULL
                 );
    else
        addLoader("CALLTR10",
                  (filter_id & 0xf) % 3  == 1 ? "CALLTRE8" : "CALLTRE9",
                  "CALLTR11",
                  (filter_id > 0x20) ? "CTCLEVE2" : "",
                  "CALLTR12",
                  (filter_id & 0xf) > 3 ? (filter_id > 0x20 ? "CTBSHR11""CTBSWA11" : "CTBROR11""CTBSWA11") : "",
                  "CALLTR13",
                  NULL
                 );
}


/*************************************************************************
// Try compression with several filters, choose the best or first one.
// Needs buildLoader().
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
// - ibuf[] is restored to unfiltered version
// - obuf[] contains the best compressed version
//
// strategy:
//   0:  try all filters, use best one
//   n:  try the first N filters in parm_filters[], use best one
//  -1:  try all filters, use first working one
//  -2:  try only the opt->filter filter
//
// This has been prepared for generalization into class Packer so that
// opt->all_filters is available for all executable formats.
//
// It will replace the tryFilters() / compress() call sequence.
**************************************************************************/

void Packer::compressWithFilters(Filter *parm_ft, unsigned *parm_overlapoh,
                                 const unsigned overlap_range,
                                 int strategy, const int *parm_filters,
                                 unsigned max_offset, unsigned max_match)
{
    const int *f;
    //
    const PackHeader orig_ph = this->ph;
          PackHeader best_ph = this->ph;
    //
    const Filter orig_ft = *parm_ft;
          Filter best_ft = *parm_ft;
    //
    const unsigned buf_len = orig_ph.u_len;
    const unsigned filter_len = orig_ft.buf_len ? orig_ft.buf_len : buf_len;
    //
    best_ph.c_len = orig_ph.u_len;
    unsigned best_ph_lsize = 0;
    unsigned best_overlapoh = 0;

    // preconditions
    assert(orig_ph.filter == 0);
    assert(orig_ft.id == 0);
    assert(filter_len <= buf_len);

    // setup raw_filters
    static const int no_filters[] = { 0, -1 };
    int strategy_filters[] = { 0, -1 };

    const int *raw_filters = parm_filters;
    if (raw_filters == NULL)
        raw_filters = getFilters();
    if (raw_filters == NULL)
        raw_filters = no_filters;
    if (strategy == -2)
    {
        assert(opt->filter >= 0);
        strategy_filters[0] = opt->filter;
        raw_filters = strategy_filters;
    }

    // first pass - count number of filters
    int raw_nfilters = 0;
    for (f = raw_filters; *f >= 0; f++)
    {
        assert(isValidFilter(*f));
        raw_nfilters++;
    }

    // copy filters, eliminate duplicates, add a 0
    int nfilters = 0;
    bool zero_seen = false;
    autoheap_array(int, filters, raw_nfilters + 2);
    for (f = raw_filters; *f >= 0; f++)
    {
        if (*f == 0)
            zero_seen = true;
        bool duplicate = false;
        for (int i = 0; i < nfilters; i++)
            if (filters[i] == *f)
                duplicate = true;
        if (!duplicate)
            filters[nfilters++] = *f;
    }
    if (!zero_seen)
        filters[nfilters++] = 0;
    filters[nfilters] = -1;

    // update total_passes
    if (strategy < 0)
        this->total_passes += 1;
    else
    {
        if (strategy > nfilters)
            nfilters = strategy;
        this->total_passes += nfilters;
    }

    // Working buffer for compressed data. Don't waste memory.
    upx_byte *otemp = obuf;
    MemBuffer otemp_buf;
    if (nfilters > 1 && strategy >= 0)
    {
        otemp_buf.allocForCompression(buf_len);
        otemp = otemp_buf;
    }

    // compress
    int nfilters_success = 0;
    for (int i = 0; i < nfilters; i++)
    {
        // get fresh packheader
        ph = orig_ph;
        ph.filter = filters[i];
        // get fresh filter
        Filter ft = orig_ft;
        ft.init(filters[i], orig_ft.addvalue);
        // filter
        optimizeFilter(&ft, ibuf, filter_len);
        bool success = ft.filter(ibuf, filter_len);
        if (ft.id != 0 && ft.calls == 0)
        {
            // filter did not do anything - no need to call ft.unfilter()
            success = false;
        }
        if (!success)
        {
            // filter failed or was usesless
            if (strategy >= 0)
                this->total_passes -= 1;
            continue;
        }
        // filter success
        nfilters_success++;
        ph.filter_cto = ft.cto;
        // compress
        if (compress(ibuf, otemp, max_offset, max_match))
        {
            // get results
            const unsigned lsize = buildLoader(&ft);
#if 0
            printf("\n%02x: %d + %d = %d  (best: %d + %d = %d)\n", ft.id,
                   ph.c_len, getLoaderSize(), ph.c_len + getLoaderSize(),
                   best_ph.c_len, best_ph_lsize, best_ph.c_len + best_ph_lsize);
#endif
            if (ph.c_len + lsize < best_ph.c_len + best_ph_lsize)
            {
                // update obuf[] with best version
                if (otemp != obuf)
                    memcpy(obuf, otemp, ph.c_len);
                // save compression results
                best_ph = ph;
                best_ph_lsize = lsize;
                best_ft = ft;
                best_overlapoh = findOverlapOverhead(obuf, overlap_range);
            }
        }
        // restore ibuf[] - unfilter with verify
        ft.unfilter(ibuf, filter_len, true);
        //
        if (strategy < 0)
            break;
    }

    // postconditions
    assert(nfilters_success > 0);
    assert(best_ph.u_len == orig_ph.u_len);
    assert(best_ph.filter == best_ft.id);
    assert(best_ph.filter_cto == best_ft.cto);

    // copy back results
    this->ph = best_ph;
    *parm_ft = best_ft;
    *parm_overlapoh = best_overlapoh;

    // finally check compression ratio
    if (best_ph.c_len + best_ph_lsize >= best_ph.u_len)
        throwNotCompressible();
    if (!checkCompressionRatio(best_ph.c_len, best_ph.u_len))
        throwNotCompressible();

    // convenience
    buildLoader(&best_ft);
}


/*************************************************************************
// filter util
**************************************************************************/

bool Packer::isValidFilter(int filter_id) const
{
    if (filter_id == 0)
        return true;
    for (const int *f = getFilters(); f && *f >= 0; f++)
    {
        if (*f == filter_id)
            return true;
    }
    return false;
}


void Packer::tryFilters(Filter *ft, upx_byte *buf, unsigned buf_len,
                        unsigned addvalue) const
{
    // debug
    //scanFilters(ft, buf, buf_len, addvalue);

    ft->init();
    if (opt->filter == 0)
        return;
    for (const int *f = getFilters(); f && *f >= 0; f++)
    {
        if (*f == 0)        // skip no-filter
            continue;
        if (opt->filter < 0 || *f == opt->filter)
        {
            ft->init(*f, addvalue);
            optimizeFilter(ft, buf, buf_len);
            if (ft->filter(buf, buf_len) && ft->calls > 0)
                break;                      // success
            ft->init();
        }
    }
}


void Packer::scanFilters(Filter *ft, const upx_byte *buf, unsigned buf_len,
                         unsigned addvalue) const
{
    ft->init();
    if (opt->filter == 0)
        return;
    for (const int *f = getFilters(); f && *f >= 0; f++)
    {
        if (*f == 0)        // skip no-filter
            continue;
        ft->init(*f, addvalue);
        //static const int pc[] = { 0xff, 0xfe, 0x80, 0x22, -1 };
        //ft->preferred_ctos = pc;
        if (ft->scan(buf, buf_len))
        {
            printf("scanFilters: id 0x%02x size: %6d: calls %5d/%5d/%3d, cto 0x%02x\n",
                   ft->id, ft->buf_len, ft->calls, ft->noncalls, ft->wrongcalls, ft->cto);
        }
        ft->init();
    }
}


/*
vi:ts=4:et:nowrap
*/

