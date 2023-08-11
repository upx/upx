/* packhead.h --

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

class Filter;

/*************************************************************************
// PackHeader
// also see stub/src/include/header.S
**************************************************************************/

struct PackHeader final {
    explicit PackHeader() noexcept;
    ~PackHeader() noexcept = default;

    void reset() noexcept;

    void putPackHeader(SPAN_S(byte) p) const;
    bool decodePackHeaderFromBuf(SPAN_S(const byte) b, int blen);

    int getPackHeaderSize() const;

    // fields stored in compressed file => see stub/src/include/header.S
    // enum { magic = UPX_MAGIC_LE32 };
    int version;
    int format; // executable format UPX_F_xxx
    int method; // compresison method M_xxx
    int level;  // compresison level 1..10
    unsigned u_len;
    unsigned c_len;
    unsigned u_adler;
    unsigned c_adler;
    unsigned u_file_size;
    int filter;
    int filter_cto;
    union {
        int filter_misc; // generic name
        int n_mru;       // specific name for filter ctojr
    };
    int header_checksum;

    // support fields for verifying decompression
    unsigned saved_u_adler;
    unsigned saved_c_adler;

    // info fields set by decodePackHeaderFromBuf()
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

/*************************************************************************
// ph default util functions
**************************************************************************/

bool ph_is_forced_method(int method) noexcept; // predicate
int ph_force_method(int method) noexcept;      // (0x80ul<<24)|method
int ph_forced_method(int method) noexcept;     // (0x80ul<<24)|method ==> method

bool ph_skipVerify(const PackHeader &ph) noexcept;

void ph_decompress(PackHeader &ph, SPAN_P(const byte) in, SPAN_P(byte) out, bool verify_checksum,
                   Filter *ft);

bool ph_testOverlappingDecompression(const PackHeader &ph, const byte *buf, const byte *tbuf,
                                     unsigned overlap_overhead);
