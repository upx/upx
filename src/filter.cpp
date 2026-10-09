/* filter.cpp --

   This file is part of the UPX executable compressor.

   Copyright (C) Markus Franz Xaver Johannes Oberhumer
   Copyright (C) Laszlo Molnar
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
#include "filter.h"
#include "file.h"

/*************************************************************************
// util
**************************************************************************/

static void initFilter(Filter *f, byte *buf, unsigned buf_len) noexcept {
    f->buf = buf;
    f->buf_len = buf_len;
    // clear output parameters
    f->calls = f->wrongcalls = f->noncalls = f->firstcall = f->lastcall = 0;
}

/*************************************************************************
// get a FilterEntry
**************************************************************************/

/*static*/ const FilterImpl::FilterEntry *FilterImpl::getFilter(int id) noexcept {
    static upx_uint8_t filter_map[256];
    static upx_std_once_flag init_done;

    upx_std_call_once(init_done, []() noexcept {
        // init the filter_map[] (using a lambda function)
        assert_noexcept(n_filters <= 254); // as 0xff means "empty slot"
        memset(filter_map, 0xff, sizeof(filter_map));
        for (int i = 0; i < n_filters; i++) {
            int filter_id = filters[i].id;
            assert_noexcept(filter_id >= 0 && filter_id <= 255);
            assert_noexcept(filter_map[filter_id] == 0xff);
            filter_map[filter_id] = (upx_uint8_t) i;
        }
    });

    if (id < 0 || id > 255)
        return nullptr;
    unsigned index = filter_map[id];
    if (index == 0xff) // empty slot
        return nullptr;
    assert_noexcept(filters[index].id == id);
    return &filters[index];
}

/*static*/ bool Filter::isValidFilter(int filter_id) noexcept {
    const FilterImpl::FilterEntry *const fe = FilterImpl::getFilter(filter_id);
    return fe != nullptr;
}

/*static*/ bool Filter::isValidFilter(int filter_id, const int *allowed_filters) noexcept {
    if (!isValidFilter(filter_id))
        return false;
    if (filter_id == 0)
        return true;
    if (allowed_filters == nullptr)
        return false;
    while (*allowed_filters != FT_END)
        if (*allowed_filters++ == filter_id)
            return true;
    return false;
}

/*************************************************************************
// high level API
**************************************************************************/

Filter::Filter(int level) noexcept : clevel(level) { init(); }

void Filter::init(int id_, unsigned addvalue_) noexcept {
    this->id = id_;
    initFilter(this, nullptr, 0);
    // clear input parameters
    this->addvalue = addvalue_;
    this->preferred_ctos = nullptr;
    // clear input/output parameters
    this->cto = 0;
    this->n_mru = 0;
}

bool Filter::filter(SPAN_0(byte) xbuf, unsigned buf_len_) {
    byte *const buf_ = raw_bytes(xbuf, buf_len_);
    initFilter(this, buf_, buf_len_);

    const FilterImpl::FilterEntry *const fe = FilterImpl::getFilter(id);
    if (fe == nullptr)
        throwInternalError("filter-1");
    if (fe->id == 0)
        return true;
    if (buf_len < fe->min_buf_len)
        return false;
    if (fe->max_buf_len && buf_len > fe->max_buf_len)
        return false;
    if (!fe->do_filter)
        throwInternalError("filter-2");

    // save checksum
    this->adler = 0;
    if (clevel != 1)
        this->adler = upx_adler32(this->buf, this->buf_len);

    NO_printf("filter: %02x %p %d\n", this->id, this->buf, this->buf_len);
    // OutputFile::dump("filter.dat", buf, buf_len);
    int r = (*fe->do_filter)(this);
    NO_printf("filter: %02x %d\n", fe->id, r);
    if (r > 0)
        throwFilterException();
    if (r == 0)
        return true;
    return false;
}

void Filter::unfilter(SPAN_0(byte) xbuf, unsigned buf_len_, bool verify_checksum) {
    byte *const buf_ = raw_bytes(xbuf, buf_len_);
    initFilter(this, buf_, buf_len_);

    const FilterImpl::FilterEntry *const fe = FilterImpl::getFilter(id);
    if (fe == nullptr)
        throwInternalError("unfilter-1");
    if (fe->id == 0)
        return;
    if (buf_len < fe->min_buf_len)
        return;
    if (fe->max_buf_len && buf_len > fe->max_buf_len)
        return;
    if (!fe->do_unfilter)
        throwInternalError("unfilter-2");

    NO_printf("unfilter: %02x %p %d\n", this->id, this->buf, this->buf_len);
    int r = (*fe->do_unfilter)(this);
    NO_printf("unfilter: %02x %d\n", fe->id, r);
    if (r != 0)
        throwInternalError("unfilter-3");
    // OutputFile::dump("unfilter.dat", buf, buf_len);

    // verify checksum
    if (verify_checksum && clevel != 1) {
        if (this->adler != upx_adler32(this->buf, this->buf_len))
            throwInternalError("unfilter-4");
    }
}

void Filter::verifyUnfilter() {
    // Note:
    //   This verify is just because of complete paranoia that there
    //   could be a hidden bug in the filter implementation, and
    //   it should not be necessary at all.
    //   Maybe we will remove it at some future point.
    //
    // See also:
    //   Packer::verifyOverlappingDecompression()

    NO_printf("verifyUnfilter: %02x %p %d\n", this->id, this->buf, this->buf_len);
    if (clevel != 1)
        unfilter(this->buf, this->buf_len, true);
}

bool Filter::scan(SPAN_0(const byte) xbuf, unsigned buf_len_) {
    const byte *const buf_ = raw_bytes(xbuf, buf_len_);
    // Note: must use const_cast here. This is fine as the scan
    //   implementations (fe->do_scan) actually don't change the buffer.
    byte *const b = const_cast<byte *>(buf_);
    initFilter(this, b, buf_len_);

    const FilterImpl::FilterEntry *const fe = FilterImpl::getFilter(id);
    if (fe == nullptr)
        throwInternalError("scan-1");
    if (fe->id == 0)
        return true;
    if (buf_len < fe->min_buf_len)
        return false;
    if (fe->max_buf_len && buf_len > fe->max_buf_len)
        return false;
    if (!fe->do_scan)
        throwInternalError("scan-2");

    NO_printf("filter: %02x %p %d\n", this->id, this->buf, this->buf_len);
    int r = (*fe->do_scan)(this);
    NO_printf("filter: %02x %d\n", fe->id, r);
    if (r > 0)
        throwFilterException();
    if (r == 0)
        return true;
    return false;
}

/*************************************************************************
// doctest checks
**************************************************************************/

TEST_CASE("ARM64 branch filters") {
    const struct {
        unsigned instruction;
        unsigned mask;
        unsigned shift;
    } instructions[] = {
        {0xd503201f, 0, 0},          // nop
        {0x17ffffff, 0x03ffffff, 0}, // b
        {0x94000010, 0x03ffffff, 0}, // bl
        {0x54ffffe1, 0x00ffffe0, 5}, // b.ne
        {0x34000065, 0x00ffffe0, 5}, // cbz w5
        {0xb4ffffc5, 0x00ffffe0, 5}, // cbz x5
        {0x35ffffe7, 0x00ffffe0, 5}, // cbnz w7
        {0xb5000047, 0x00ffffe0, 5}, // cbnz x7
        {0x361ffff1, 0x0007ffe0, 5}, // tbz
        {0xb7f80024, 0x0007ffe0, 5}, // tbnz
        {0x91000400, 0, 0},          // add
        {0x14000002, 0x03ffffff, 0}, // final b
    };
    constexpr unsigned n_words = sizeof(instructions) / sizeof(instructions[0]);
    constexpr unsigned n_bytes = 4 * n_words + 3;
    byte original[n_bytes];
    memset(original, 0xa5, sizeof(original));
    for (unsigned i = 0; i < n_words; ++i)
        set_le32(original + 4 * i, instructions[i].instruction);

    const int ids[] = {
        0x53,
        0x52,
    };
    for (int id : ids) {
        CHECK(Filter::isValidFilter(id));
        for (unsigned addvalue : {0u, 0x1000u, 0x1234u, 0xfffffff0u}) {
            for (unsigned len = 0; len <= n_bytes; ++len) {
                CAPTURE(id);
                CAPTURE(addvalue);
                CAPTURE(len);
                byte buf[n_bytes], expected[n_bytes];
                memcpy(buf, original, sizeof(buf));
                memcpy(expected, original, sizeof(expected));
                Filter f(3);
                f.init(id, addvalue);
                if (len < 8) {
                    CHECK_FALSE(f.scan(buf, len));
                    CHECK_FALSE(f.filter(buf, len));
                    CHECK(memcmp(buf, original, sizeof(buf)) == 0);
                    continue;
                }
                unsigned calls = 0;
                for (unsigned i = 0; i < n_words; ++i) {
                    const unsigned a = 4 * i;
                    if (a + 4 > len || (id == 0x52 && a + 4 == len))
                        break;
                    const auto &insn = instructions[i];
                    if (insn.mask == 0 || (id == 0x52 && insn.shift != 0))
                        continue;
                    const unsigned d = a / 4 + (insn.shift == 0 ? addvalue : 0);
                    const unsigned field = (insn.instruction >> insn.shift) + d;
                    set_le32(expected + a,
                             (insn.instruction & ~insn.mask) | ((field << insn.shift) & insn.mask));
                    ++calls;
                }
                REQUIRE(f.scan(buf, len));
                CHECK(f.calls == calls);
                CHECK(memcmp(buf, original, sizeof(buf)) == 0);
                REQUIRE(f.filter(buf, len));
                CHECK(f.calls == calls);
                CHECK(memcmp(buf, expected, sizeof(buf)) == 0);
                f.unfilter(buf, len, true);
                CHECK(memcmp(buf, original, sizeof(buf)) == 0);
                UNUSED(calls);
            }
        }
    }
}

/* vim:set ts=4 sw=4 et: */
