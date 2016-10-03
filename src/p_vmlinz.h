/* p_vmlinz.h --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2016 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2016 Laszlo Molnar
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


#ifndef __UPX_P_VMLINZ_H
#define __UPX_P_VMLINZ_H 1


/*************************************************************************
// vmlinuz/i386 (gzip compressed Linux kernel image)
**************************************************************************/

class PackVmlinuzI386 : public Packer
{
    typedef Packer super;
public:
    PackVmlinuzI386(InputFile *f);
    virtual int getVersion() const { return 13; }
    virtual int getFormat() const { return UPX_F_VMLINUZ_i386; }
    virtual const char *getName() const { return "vmlinuz/i386"; }
    virtual const char *getFullName(const options_t *) const { return "i386-linux.kernel.vmlinuz"; }
    virtual const int *getCompressionMethods(int method, int level) const;
    virtual const int *getFilters() const;
    virtual int getStrategy(Filter &);

    virtual void pack(OutputFile *fo);
    virtual void unpack(OutputFile *fo);

    virtual bool canPack();
    virtual int canUnpack();

protected:
    virtual int readFileHeader();
    virtual int decompressKernel();
    virtual void readKernel();

    virtual void buildLoader(const Filter *ft);
    virtual Linker* newLinker() const;

//    virtual upx_byte *getLoader() const;
//    virtual int getLoaderSize() const;

    __packed_struct(boot_sect_t)
        char            _[0x1f1];
        unsigned char   setup_sects;
        char            __[2];
        LE32            sys_size;
        char            ___[6];
        LE16            boot_flag;      // 0xAA55
        // 0x200
        char            ____[2];
        unsigned char   hdrs[4];        // "HdrS"
        LE16            version;        // boot protocol
        char            _____[9];
        unsigned char   load_flags;
        char            ______[2];
        LE32            code32_start;
        char            _7[0x230 - (0x214 + 4)];
        LE32            kernel_alignment;
        char            relocatable_kernel;
        char            _8[0x248 - (0x234 + 1)];
        LE32            payload_offset;
        LE32            payload_length;
        // some more uninteresting fields here ...
        // see /usr/src/linux/Documentation/i386/boot.txt
    __packed_struct_end()

    boot_sect_t h;

    MemBuffer setup_buf;
    int setup_size;
    unsigned physical_start;
    unsigned page_offset;
    unsigned config_physical_align;
    unsigned filter_len;
};


/*************************************************************************
// bvmlinuz/i386 (gzip compressed Linux kernel image)
**************************************************************************/

class PackBvmlinuzI386 : public PackVmlinuzI386
{
    typedef PackVmlinuzI386 super;
public:
    PackBvmlinuzI386(InputFile *f) : super(f) { }
    virtual int getFormat() const { return UPX_F_BVMLINUZ_i386; }
    virtual const char *getName() const { return "bvmlinuz/i386"; }
    virtual const char *getFullName(const options_t *) const { return "i386-linux.kernel.bvmlinuz"; }
    virtual const int *getFilters() const;

    virtual void pack(OutputFile *fo);

protected:
    virtual void buildLoader(const Filter *ft);
};


/*************************************************************************
// elks/8086 (linux-8086 uncompressed kernel image)
**************************************************************************/

class PackElks8086 : public PackVmlinuzI386
{
    typedef PackVmlinuzI386 super;
public:
    PackElks8086(InputFile *f) : super(f) { }
    virtual int getFormat() const { return UPX_F_ELKS_8086; }
    virtual const char *getName() const { return "elks/i086"; }
    virtual const char *getFullName(const options_t *) const { return "i086-elks ???"; }
    virtual const int *getCompressionMethods(int method, int level) const;
    virtual const int *getFilters() const;

    virtual void pack(OutputFile *fo);
    virtual void unpack(OutputFile *fo);

    virtual int canUnpack();

protected:
    virtual int decompressKernel();
    virtual void readKernel();

    virtual void buildLoader(const Filter *ft);
};


/*************************************************************************
// vmlinuz/armel (gzip compressed Linux kernel image)
**************************************************************************/

class PackVmlinuzARMEL : public Packer
{
    typedef Packer super;
public:
    PackVmlinuzARMEL(InputFile *f);
    virtual int getVersion() const { return 13; }
    virtual int getFormat() const { return UPX_F_VMLINUZ_ARMEL; }
    virtual const char *getName() const { return "vmlinuz/arm"; }
    virtual const char *getFullName(const options_t *) const { return "arm-linux.kernel.vmlinuz"; }
    virtual const int *getCompressionMethods(int method, int level) const;
    virtual const int *getFilters() const;
    virtual int getStrategy(Filter &);

    virtual void pack(OutputFile *fo);
    virtual void unpack(OutputFile *fo);

    virtual bool canPack();
    virtual int canUnpack();

protected:
    virtual int readFileHeader();
    virtual int decompressKernel();
    virtual void readKernel();

    virtual void buildLoader(const Filter *ft);
    virtual unsigned write_vmlinuz_head(OutputFile *const fo);
    virtual void defineDecompressorSymbols();
    virtual Linker* newLinker() const;

//    virtual upx_byte *getLoader() const;
//    virtual int getLoaderSize() const;


    MemBuffer setup_buf;
    int setup_size;
//    unsigned physical_start;
//    unsigned page_offset;
//    unsigned config_physical_align;
    unsigned filter_len;
};


#endif /* already included */

/* vim:set ts=4 sw=4 et: */
