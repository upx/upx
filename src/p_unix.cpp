/* p_unix.cpp --

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

   Markus F.X.J. Oberhumer                   Laszlo Molnar
   markus.oberhumer@jk.uni-linz.ac.at        ml1050@cdata.tvnet.hu
 */


#include "conf.h"

#include "file.h"
#include "packer.h"
#include "p_unix.h"
#include "p_elf.h"

// do not change
#define BLOCKSIZE       (512*1024)


/*************************************************************************
//
**************************************************************************/

PackUnix::PackUnix(InputFile *f) :
    super(f), exetype(0), blocksize(0), overlay_offset(0), lsize(0)
{
    COMPILE_TIME_ASSERT(sizeof(Elf_LE32_Ehdr) == 52);
    COMPILE_TIME_ASSERT(sizeof(Elf_LE32_Phdr) == 32);
    COMPILE_TIME_ASSERT(sizeof(l_info) == 12);
    COMPILE_TIME_ASSERT(sizeof(p_info) == 12);
}


// common part of canPack(), enhanced by subclasses
bool PackUnix::canPack()
{
    if (exetype == 0)
        return false;

#if defined(__unix__)
    // must be executable by owner
    if ((fi->st.st_mode & S_IXUSR) == 0)
        throwCantPack("file not executable; try `chmod +x'");
#endif
    if (file_size < 4096)
        throwCantPack("file is too small");

    // info: currently the header is 36 (32+4) bytes before EOF
    unsigned char buf[256];
    fi->seek(-(long)sizeof(buf), SEEK_END);
    fi->readx(buf, sizeof(buf));
    checkAlreadyPacked(buf, sizeof(buf));

    return true;
}


/*************************************************************************
// Generic Unix pack(). Subclasses must provide patchLoader().
//
// A typical compressed Unix executable looks like this:
//   - loader stub
//   - 12 bytes header info
//   - the compressed blocks, each with a 8 byte header for block sizes
//   - 4 bytes block end marker (uncompressed size 0)
//   - 32 bytes UPX packheader
//   - 4 bytes overlay offset (needed for decompression)
**************************************************************************/

// see note below and Packer::compress()
bool PackUnix::checkCompressionRatio(unsigned, unsigned) const
{
    return true;
}


void PackUnix::pack(OutputFile *fo)
{
    // set options
    blocksize = opt->unix.blocksize;
    if (blocksize <= 0)
        blocksize = BLOCKSIZE;
    if ((off_t)blocksize > file_size)
        blocksize = file_size;
    // create a pseudo-unique program id for our paranoid stub
    progid = getRandomId();

    // prepare loader
    lsize = getLoaderSize();
    loader.alloc(lsize + sizeof(p_info));
    memcpy(loader,getLoader(),lsize);

    // patch loader, prepare header info
    patchLoader();  // can change lsize by packing C-code of upx_main etc.
    p_info *const hbuf = (p_info *)(loader + lsize);
    set_native32(&hbuf->p_progid, progid);
    set_native32(&hbuf->p_filesize, file_size);
    set_native32(&hbuf->p_blocksize, blocksize);
    fo->write(loader, lsize + sizeof(p_info));

    // init compression buffers
    ibuf.alloc(blocksize);
    obuf.allocForCompression(blocksize);

    // compress blocks
    unsigned total_in = 0;
    unsigned total_out = 0;
    ui_total_passes = (file_size + blocksize - 1) / blocksize;
    if (ui_total_passes == 1)
        ui_total_passes = 0;
    fi->seek(0, SEEK_SET);
    for (;;)
    {
        int l = fi->read(ibuf, blocksize);
        if (l == 0)
            break;

        // Note: compression for a block can fail if the
        //       file is e.g. blocksize + 1 bytes long

        // compress
        ph.u_len = l;
        ph.overlap_overhead = 0;
        (void) compress(ibuf, obuf);    // ignore return value

        if (ph.c_len < ph.u_len)
        {
            ph.overlap_overhead = OVERHEAD;
            if (!testOverlappingDecompression(obuf, ph.overlap_overhead))
                throwNotCompressible();
        }
        else
        {
            // block is not compressible
            ph.c_len = ph.u_len;
            // must manually update checksum of compressed data
            ph.c_adler = upx_adler32(ph.c_adler, ibuf, ph.u_len);
        }

        // write block sizes
        unsigned char size[8];
        set_native32(size+0, ph.u_len);
        set_native32(size+4, ph.c_len);
        fo->write(size, 8);

        // write compressed data
        if (ph.c_len < ph.u_len)
        {
            fo->write(obuf, ph.c_len);
            verifyOverlappingDecompression();
        }
        else
            fo->write(ibuf, ph.u_len);

        total_in += ph.u_len;
        total_out += ph.c_len;
    }
    if ((off_t)total_in != file_size)
        throwEOFException();

    // write block end marker (uncompressed size 0)
    fo->write("\x00\x00\x00\x00", 4);

    // update header with totals
    ph.u_len = total_in;
    ph.c_len = total_out;

    // write packheader
    const int hsize = ph.getPackHeaderSize();
    set_le32(obuf, UPX_MAGIC_LE32);             // note: always le32
    patchPackHeader(obuf, hsize);
    fo->write(obuf, hsize);

    // write overlay offset (needed for decompression)
    set_native32(obuf, lsize);
    fo->write(obuf, 4);

    updateLoader(fo);

    // finally check the compression ratio
    if (!checkFinalCompressionRatio(fo))
        throwNotCompressible();
}


/*************************************************************************
// Generic Unix canUnpack().
**************************************************************************/

int PackUnix::canUnpack()
{
    upx_byte buf[128];
    const int bufsize = sizeof(buf);

    fi->seek(-bufsize, SEEK_END);
    fi->readx(buf, bufsize);
    if (!getPackHeader(buf, bufsize))
        return false;

    int l = ph.buf_offset + ph.getPackHeaderSize();
    if (l < 0 || l + 4 > bufsize)
        throwCantUnpack("file corrupted");
    overlay_offset = get_native32(buf+l);
    if ((off_t)overlay_offset >= file_size)
        throwCantUnpack("file corrupted");

    return true;
}


/*************************************************************************
// Generic Unix unpack().
//
// This code looks much like the one in stub/l_linux.c
// See notes there.
**************************************************************************/

void PackUnix::unpack(OutputFile *fo)
{
    unsigned c_adler = upx_adler32(0, NULL, 0);
    unsigned u_adler = upx_adler32(0, NULL, 0);

    // defaults for ph.version == 8
    unsigned orig_file_size = 0;
    blocksize = 512 * 1024;

    fi->seek(overlay_offset, SEEK_SET);
    if (ph.version > 8)
    {
        p_info hbuf;
        fi->readx(&hbuf, sizeof(hbuf));
        orig_file_size = get_native32(&hbuf.p_filesize);
        blocksize = get_native32(&hbuf.p_blocksize);

        if (file_size > (off_t)orig_file_size || blocksize > orig_file_size)
            throwCantUnpack("file header corrupted");
    }
    else
    {
        // skip 4 bytes (program id)
        fi->seek(4, SEEK_CUR);
    }

    ibuf.alloc(blocksize + OVERHEAD);

    // decompress blocks
    unsigned total_in = 0;
    unsigned total_out = 0;
    for (;;)
    {
#define buf ibuf
        int i;
        int size[2];

        fi->readx(buf, 8);
        ph.u_len = size[0] = get_native32(buf+0);
        ph.c_len = size[1] = get_native32(buf+4);

        if (size[0] == 0)                   // uncompressed size 0 -> EOF
        {
            // note: must reload size[1] as magic is always stored le32
            size[1] = get_le32(buf+4);
            if (size[1] != UPX_MAGIC_LE32)  // size[1] must be h->magic
                throwCompressedDataViolation();
            break;
        }
        if (size[0] <= 0 || size[1] <= 0)
            throwCompressedDataViolation();
        if (size[1] > size[0] || size[0] > (int)blocksize)
            throwCompressedDataViolation();

        i = blocksize + OVERHEAD - size[1];
        fi->readx(buf+i, size[1]);
        // update checksum of compressed data
        c_adler = upx_adler32(c_adler, buf + i, size[1]);
        // decompress
        if (size[1] < size[0])
        {
            decompress(buf+i, buf, false);
            i = 0;
        }
        // update checksum of uncompressed data
        u_adler = upx_adler32(u_adler, buf + i, size[0]);
        total_in += size[1];
        total_out += size[0];
        // write block
        if (fo)
            fo->write(buf + i, size[0]);
#undef buf
    }

    // update header with totals
    ph.c_len = total_in;
    ph.u_len = total_out;

    // all bytes must be written
    if (ph.version > 8 && total_out != orig_file_size)
        throwEOFException();

    // finally test the checksums
    if (ph.c_adler != c_adler || ph.u_adler != u_adler)
        throwChecksumError();
}


/*
vi:ts=4:et
*/

