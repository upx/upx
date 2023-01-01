/* p_tos.h --

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
#ifndef UPX_P_TOS_H__
#define UPX_P_TOS_H__ 1

/*************************************************************************
// atari/tos
**************************************************************************/

class PackTos final : public Packer {
    typedef Packer super;

public:
    PackTos(InputFile *f);
    virtual int getVersion() const override { return 13; }
    virtual int getFormat() const override { return UPX_F_ATARI_TOS; }
    virtual const char *getName() const override { return "atari/tos"; }
    virtual const char *getFullName(const options_t *) const override { return "m68k-atari.tos"; }
    virtual const int *getCompressionMethods(int method, int level) const override;
    virtual const int *getFilters() const override;

    virtual void pack(OutputFile *fo) override;
    virtual void unpack(OutputFile *fo) override;

    virtual bool canPack() override;
    virtual int canUnpack() override;

    virtual void fileInfo() override;

protected:
    virtual Linker *newLinker() const override;
    virtual void buildLoader(const Filter *ft) override;

    unsigned getDecomprOffset(int method, int small) const;

    int readFileHeader();
    bool checkFileHeader();

    struct alignas(1) tos_header_t {
        BE16 fh_magic;
        BE32 fh_text;
        BE32 fh_data;
        BE32 fh_bss;
        BE32 fh_sym;
        BE32 fh_reserved;
        BE32 fh_flag;
        BE16 fh_reloc;
    };

    tos_header_t ih, oh;

    // symbols for buildLoader()
    struct LinkerSymbols {
        enum { LOOP_NONE, LOOP_SUBQ_L, LOOP_SUBQ_W, LOOP_DBRA };
        struct LoopInfo {
            unsigned mode;
            unsigned count;
            unsigned value;
            void init(unsigned count, bool allow_dbra = true);
        };
        // buildLoader() input
        bool need_reloc;
        LoopInfo loop1;
        LoopInfo loop2;
        LoopInfo loop3;
        unsigned up21_d4;
        unsigned up21_a6;
        unsigned up31_base_d4;
        unsigned up31_base_a6;
        // buildLoader() output
        unsigned up31_d4;
        unsigned up31_a6;
        // currently not used by buildLoader()
        unsigned flush_cache_rts_offset;
        unsigned clear_dirty_stack_len;
        unsigned copy_to_stack_len;

        void reset() { memset(this, 0, sizeof(*this)); }
    };
    LinkerSymbols symbols;
};

#endif /* already included */

/* vim:set ts=4 sw=4 et: */
