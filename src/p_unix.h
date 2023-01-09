/* p_unix.h --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2023 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2023 Laszlo Molnar
   Copyright (C) 2000-2023 John F. Reiser
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

   John F. Reiser
   <jreiser@users.sourceforge.net>
 */


#ifndef __UPX_P_UNIX_H
#define __UPX_P_UNIX_H 1


/*************************************************************************
// Abstract class for all Unix-type packers.
// Already provides most of the functionality.
**************************************************************************/

class PackUnix : public Packer
{
    typedef Packer super;
protected:
    PackUnix(InputFile *f);
public:
    virtual int getVersion() const override { return 13; }
    virtual const int *getFilters() const override { return nullptr; }
    virtual int getStrategy(Filter &);

    virtual void pack(OutputFile *fo) override;
    virtual void unpack(OutputFile *fo) override;

    virtual bool canPack() override;
    virtual int  canUnpack() override; // bool, except -1: format known, but not packed
    int find_overlay_offset(MemBuffer const &buf);

protected:
    // called by the generic pack()
    virtual void pack1(OutputFile *, Filter &);  // generate executable header
    virtual int  pack2(OutputFile *, Filter &);  // append compressed data
    virtual off_t pack3(OutputFile *, Filter &);  // append loader
    virtual void pack4(OutputFile *, Filter &);  // append PackHeader

    virtual void patchLoader() = 0;
    virtual void patchLoaderChecksum();
    virtual void updateLoader(OutputFile *) = 0;

    virtual void writePackHeader(OutputFile *fo);

    virtual bool checkCompressionRatio(unsigned, unsigned) const override;

protected:
    struct Extent {
        off_t offset;
        off_t size;
    };
    virtual void packExtent(const Extent &x,
        Filter *, OutputFile *,
        unsigned hdr_len = 0, unsigned b_extra = 0 ,
        bool inhibit_compression_check = false);
    virtual unsigned unpackExtent(unsigned wanted, OutputFile *fo,
        unsigned &c_adler, unsigned &u_adler,
        bool first_PF_X, unsigned szb_info,
        int is_rewrite = false  // 0(false): write; 1(true): rewrite; -1: no write
        );
    unsigned total_in, total_out;  // unpack

    int exetype;
    unsigned blocksize;
    unsigned progid;              // program id
    unsigned overlay_offset;      // used when decompressing

    MemBuffer loader;
    int lsize;
    MemBuffer pt_dynamic;
    int sz_dynamic;

    unsigned b_len;  // total length of b_info blocks
    unsigned methods_used;  // bitmask of compression methods

    // must agree with stub/linux.hh
    __packed_struct(b_info) // 12-byte header before each compressed block
        NE32 sz_unc;  // uncompressed_size
        NE32 sz_cpr;  //   compressed_size
        unsigned char b_method;  // compression algorithm
        unsigned char b_ftid;  // filter id
        unsigned char b_cto8;  // filter parameter
        unsigned char b_extra;
    __packed_struct_end()

    __packed_struct(l_info) // 12-byte trailer in header for loader
        LE32 l_checksum;
        LE32 l_magic;
        LE16 l_lsize;
        unsigned char l_version;
        unsigned char l_format;
    __packed_struct_end()

    __packed_struct(p_info) // 12-byte packed program header
        NE32 p_progid;
        NE32 p_filesize;
        NE32 p_blocksize;
    __packed_struct_end()

    struct l_info linfo;

    // do not change !!!
    enum { OVERHEAD = 2048 };
};


/*************************************************************************
// abstract classes encapsulating endian issues
// note: UPX_MAGIC is always stored in le32 format
**************************************************************************/
class PackUnixBe32 : public PackUnix
{
    typedef PackUnix super;
protected:
    PackUnixBe32(InputFile *f) : super(f) { bele = &N_BELE_RTP::be_policy; }

    // must agree with stub/linux.hh
    __packed_struct(b_info) // 12-byte header before each compressed block
        BE32 sz_unc;  // uncompressed_size
        BE32 sz_cpr;  //   compressed_size
        unsigned char b_method;  // compression algorithm
        unsigned char b_ftid;  // filter id
        unsigned char b_cto8;  // filter parameter
        unsigned char b_extra;
    __packed_struct_end()

    __packed_struct(l_info) // 12-byte trailer in header for loader
        BE32 l_checksum;
        BE32 l_magic;
        BE16 l_lsize;
        unsigned char l_version;
        unsigned char l_format;
    __packed_struct_end()

    __packed_struct(p_info) // 12-byte packed program header
        BE32 p_progid;
        BE32 p_filesize;
        BE32 p_blocksize;
    __packed_struct_end()
};


class PackUnixLe32 : public PackUnix
{
    typedef PackUnix super;
protected:
    PackUnixLe32(InputFile *f) : super(f) { bele = &N_BELE_RTP::le_policy; }

    // must agree with stub/linux.hh
    __packed_struct(b_info) // 12-byte header before each compressed block
        LE32 sz_unc;  // uncompressed_size
        LE32 sz_cpr;  //   compressed_size
        unsigned char b_method;  // compression algorithm
        unsigned char b_ftid;  // filter id
        unsigned char b_cto8;  // filter parameter
        unsigned char b_extra;
    __packed_struct_end()

    __packed_struct(l_info) // 12-byte trailer in header for loader
        LE32 l_checksum;
        LE32 l_magic;
        LE16 l_lsize;
        unsigned char l_version;
        unsigned char l_format;
    __packed_struct_end()

    __packed_struct(p_info) // 12-byte packed program header
        LE32 p_progid;
        LE32 p_filesize;
        LE32 p_blocksize;
    __packed_struct_end()
};


#endif /* already included */

/* vim:set ts=4 sw=4 et: */
