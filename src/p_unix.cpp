/* p_unix.cpp --

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
    assert(sizeof(Elf_LE32_Ehdr) == 52);
    assert(sizeof(Elf_LE32_Phdr) == 32);
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
    fi->readx(buf,sizeof(buf));
    if (find_le32(buf,sizeof(buf),UPX_MAGIC_LE32))  // note: always le32
        throwAlreadyPacked();

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
    this->total_passes = (file_size + blocksize - 1) / blocksize;
    if (this->total_passes == 1)
        this->total_passes = 0;
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
        (void) compress(ibuf, obuf);   // ignore return value

        if (ph.c_len < ph.u_len)
        {
            if (!testOverlappingDecompression(obuf, OVERHEAD))
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
            verifyOverlappingDecompression(&obuf, OVERHEAD);
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
    set_le32(obuf, ph.magic);               // note: always le32
    putPackHeader(obuf, hsize);
    fo->write(obuf, hsize);

    // write overlay offset (needed for decompression)
    set_native32(obuf, lsize);
    fo->write(obuf, 4);

    updateLoader(fo);

    // finally check compression ratio
    if (!Packer::checkCompressionRatio(fo->getBytesWritten(), ph.u_len))
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
    if (!readPackHeader(128, -1, buf))
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


/*************************************************************************
// Linux/i386 specific (execve format)
**************************************************************************/

static const
#include "stub/l_lx_n2b.h"
static const
#include "stub/l_lx_n2d.h"


int PackLinuxI386::getCompressionMethod() const
{
    if (M_IS_NRV2B(opt->method))
        return M_NRV2B_LE32;
    if (M_IS_NRV2D(opt->method))
        return M_NRV2D_LE32;
    return opt->level > 1 && file_size >= 512*1024 ? M_NRV2D_LE32 : M_NRV2B_LE32;
}


const upx_byte *PackLinuxI386::getLoader() const
{
    if (M_IS_NRV2B(opt->method))
        return linux_i386exec_nrv2b_loader;
    if (M_IS_NRV2D(opt->method))
        return linux_i386exec_nrv2d_loader;
    return NULL;
}

int PackLinuxI386::getLoaderSize() const
{
    if (0!=lsize) {
        return lsize;
    }
    if (M_IS_NRV2B(opt->method))
        return sizeof(linux_i386exec_nrv2b_loader);
    if (M_IS_NRV2D(opt->method))
        return sizeof(linux_i386exec_nrv2d_loader);
    return 0;
}

int PackLinuxI386::getLoaderPrefixSize() const
{
    return 116;
}

bool PackLinuxI386::canPack()
{
    Elf_LE32_Ehdr ehdr;
    unsigned char *buf = ehdr.e_ident;

    fi->readx(&ehdr, sizeof(ehdr));
    fi->seek(0, SEEK_SET);

    exetype = 0;
    const unsigned l = get_le32(buf);
    if (!memcmp(buf, "\x7f\x45\x4c\x46\x01\x01\x01", 7)) // ELF 32-bit LSB
    {
        exetype = 1;
        // now check the ELF header
        if (!memcmp(buf+8, "FreeBSD", 7))               // branded
            exetype = 0;
        if (ehdr.e_type != 2)                           // executable
            exetype = 0;
        if (ehdr.e_machine != 3 && ehdr.e_machine != 6) // Intel 80[34]86
            exetype = 0;
        if (ehdr.e_version != 1)                        // version
            exetype = 0;
    }
    else if (l == 0x00640107 || l == 0x00640108 || l == 0x0064010b || l == 0x006400cc)
    {
        // OMAGIC / NMAGIC / ZMAGIC / QMAGIC
        exetype = 2;
        // FIXME: N_TRSIZE, N_DRSIZE
        // FIXME: check for aout shared libraries
    }
#if defined(__linux__)
    // only compress scripts when running under Linux
    else if (!memcmp(buf, "#!/", 3))                    // #!/bin/sh
        exetype = -1;
    else if (!memcmp(buf, "#! /", 4))                   // #! /bin/sh
        exetype = -1;
    else if (!memcmp(buf, "\xca\xfe\xba\xbe", 4))       // Java bytecode
        exetype = -2;
#endif

    return super::canPack();
}


void PackLinuxI386::patchLoader()
{
    lsize = getLoaderSize();

    // patch loader
    // note: we only can use /proc/<pid>/fd when exetype > 0.
    //   also, we sleep much longer when compressing a script.
    patch_le32(loader,lsize,"UPX4",exetype > 0 ? 3 : 15);   // sleep time
    patch_le32(loader,lsize,"UPX3",exetype > 0 ? 0 : 0x7fffffff);
    patch_le32(loader,lsize,"UPX2",progid);
    patchVersion(loader,lsize);

    Elf_LE32_Ehdr *const ehdr = (Elf_LE32_Ehdr *)(void *)loader;
    Elf_LE32_Phdr *const phdr = (Elf_LE32_Phdr *)(1+ehdr);

    // stub/scripts/setfold.pl puts address of 'fold_begin' in phdr[1].p_offset
    off_t const fold_begin = phdr[1].p_offset;
    assert(fold_begin > 0);
    assert(fold_begin < (off_t)lsize);
    MemBuffer cprLoader(lsize);

    // compress compiled C-code portion of loader
    upx_compress_config_t conf; memset(&conf, 0xff, sizeof(conf));
    conf.c_flags = 0;
    upx_uint result_buffer[16];
    size_t const uncLsize = lsize - fold_begin;
    size_t       cprLsize;
    upx_compress(
        loader + fold_begin, uncLsize,
        cprLoader, &cprLsize,
        0,  // progress_callback_t ??
        getCompressionMethod(), 9,
        &conf,
        result_buffer
    );
    memcpy(fold_begin+loader, cprLoader, cprLsize);
    lsize = fold_begin + cprLsize;
    phdr->p_filesz = lsize;
    // phdr->p_memsz is the decompressed size

    // The beginning of our loader consists of a elf_hdr (52 bytes) and
    // one     section elf_phdr (32 byte) now,
    // another section elf_phdr (32 byte) later, so we have 12 free bytes
    // from offset 116 to the program start at offset 128.
    // These 12 bytes are used for l_info by ::patchLoaderChecksum().
    assert(get_le32(loader + 28) == 52);        // e_phoff
    assert(get_le32(loader + 32) == 0);         // e_shoff
    assert(get_le16(loader + 40) == 52);        // e_ehsize
    assert(get_le16(loader + 42) == 32);        // e_phentsize
    assert(get_le16(loader + 44) == 1);         // e_phnum
    assert(get_le16(loader + 48) == 0);         // e_shnum
    assert(lsize > 128 && lsize < 4096);

    patchLoaderChecksum();
}


void PackLinuxI386::patchLoaderChecksum()
{
    l_info *const lp = (l_info *)(loader + getLoaderPrefixSize());
    // checksum for loader + p_info
    lp->l_checksum = 0;  // (this checksum is currently unused)
    lp->l_magic = UPX_ELF_MAGIC;
    lp->l_lsize = lsize;
    lp->l_version = (unsigned char) ph.version;
    lp->l_format  = (unsigned char) ph.format;
    unsigned adler = upx_adler32(0,NULL,0);
    adler = upx_adler32(adler, loader, lsize + sizeof(p_info));
    lp->l_checksum = adler;
}


void PackLinuxI386::updateLoader(OutputFile *fo)
{
#define PAGE_MASK (~0<<12)
    Elf_LE32_Ehdr *ehdr = (Elf_LE32_Ehdr *)(unsigned char *)loader;
    ehdr->e_phnum = 2;

    // The first Phdr maps the stub (instructions, data, bss) rwx.
    // The second Phdr maps the overlay r--,
    // to defend against /usr/bin/strip removing the overlay.
    Elf_LE32_Phdr *const phdro = 1+(Elf_LE32_Phdr *)(1+ehdr);

    phdro->p_type = PT_LOAD;
    phdro->p_offset = lsize;
    phdro->p_paddr = phdro->p_vaddr = 0x00400000 + (lsize &~ PAGE_MASK);
    phdro->p_memsz = phdro->p_filesz = fo->getBytesWritten() - lsize;
    phdro->p_flags = PF_R;
    phdro->p_align = -PAGE_MASK;

    patchLoaderChecksum();
    fo->seek(0, SEEK_SET);
    fo->rewrite(loader, 0x80);
#undef PAGE_MASK
}
/*
vi:ts=4:et
*/

