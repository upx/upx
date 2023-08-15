/* p_exe.h -- dos/exe executable format

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
// dos/exe
**************************************************************************/

class PackExe final : public Packer {
    typedef Packer super;

public:
    explicit PackExe(InputFile *f);
    virtual int getVersion() const override { return 13; }
    virtual int getFormat() const override { return UPX_F_DOS_EXE; }
    virtual const char *getName() const override { return "dos/exe"; }
    virtual const char *getFullName(const Options *) const override { return "i086-dos16.exe"; }
    virtual const int *getCompressionMethods(int method, int level) const override;
    virtual const int *getFilters() const override;

    virtual void pack(OutputFile *fo) override;
    virtual void unpack(OutputFile *fo) override;

    virtual tribool canPack() override;
    virtual tribool canUnpack() override;

    // unpacker capabilities
    virtual bool canUnpackVersion(int version) const override {
        // NOTE: could adapt p_exe.cpp to support (version >= 8)
        return (version >= 10);
    }
    virtual bool canUnpackFormat(int format) const override {
        return (format == UPX_F_DOS_EXE || format == UPX_F_DOS_EXEH);
    }

protected:
    struct exe_header_t;

    int readFileHeader(void);

    int fillExeHeader(struct exe_header_t *) const;
    virtual void buildLoader(const Filter *ft) override;
    virtual Linker *newLinker() const override;
    void addLoaderEpilogue(int flag);

    struct alignas(1) exe_header_t {
        LE16 ident;
        LE16 m512;
        LE16 p512;
        LE16 relocs;
        LE16 headsize16;
        LE16 min;
        LE16 max;
        LE16 ss;
        LE16 sp;
        byte _[2]; // checksum
        LE16 ip;
        LE16 cs;
        LE16 relocoffs;
        byte __[2]; // overlnum
        LE32 firstreloc;
    };

    exe_header_t ih, oh;

    unsigned ih_exesize = 0;
    unsigned ih_imagesize = 0;
    unsigned ih_overlay = 0;
    unsigned relocsize = 0;

    bool has_9a = false;
    bool device_driver = false;

    enum { NORELOC = 1, USEJUMP = 2, SS = 4, SP = 8, MINMEM = 16, MAXMEM = 32 };

    unsigned stack_for_lzma = 0; // stack size required for lzma
    bool use_clear_dirty_stack = false;
};

/* vim:set ts=4 sw=4 et: */
