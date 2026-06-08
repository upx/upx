/* p_cpm86.cpp -- CP/M-86 .cmd executable format

   This file is part of the UPX executable compressor.

   Copyright (C) Markus Franz Xaver Johannes Oberhumer
   Copyright (C) Laszlo Molnar
   Copyright (C) Jeffrey H. Johnson
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
#include "filter.h"
#include "packer.h"
#include "p_cpm86.h"
#define WANT_EHDR_ENUM 1
#include "p_elf_enum.h"
#include "linker.h"

static const CLANG_FORMAT_DUMMY_STATEMENT
#include "stub/i086-cpm86.cmd.h"

// Packing strategy
// ----------------
// The whole .cmd file (header + every group's data) is compressed as a single
// NRV2B_8 stream and prepended with a small i086 loader stub.  At run time the
// stub decompresses the original file and reconstructs the program exactly as the
// CP/M-86 loader would (groups at their segments, base page, default DMA), then
// transfers control to it.  Two run-time methods are chosen automatically (see
// pack()); the choice is recorded in CPMINFO+127 and acted on by the stub:
//
//   method 1  in-place (preferred, no BDOS): the packed .cmd mirrors the original
//             group layout, so the CP/M-86 loader itself allocates the data/extra/
//             stack segments and builds the base page.  The stub decompresses into
//             the data group and distributes the slices.  Peak memory overhead vs
//             the unpacked program is a small constant (loader + copy routine).
//   method 0  BDOS scratch (fallback): used for the 8080 model (no data group to
//             decompress into) and when method 1 would not fit or would not meet
//             the compression ratio.  The stub MC_ALLOCs a whole-file scratch,
//             decompresses there, then MC_ALLOCs and fills each group.
//
// Limitation: a packed program needs slightly more memory than the original while
// starting up (see the note in pack()); programs that run at the memory limit may
// fail to start after packing.

// Layout of the CPMINFO table written into the loader (little-endian); the stub
// reads this at run time to rebuild the original program's groups.  Keep this in
// sync with section CPMINFO in stub/src/i086-cpm86.cmd.S.
//   +0  byte  ngroups
//   +1  byte  model_8080 (1 => CP/M-80 style code+data in one group)
//   +2  word  whole-file size in paragraphs (method 0: scratch to MC_ALLOC)
//   +4  word  sp_top        (initial SP and data-group descriptor length)
//   +6  desc[4][6]  base-page group descriptors (code,data,extra,stack):
//          +0..2 length in bytes (24-bit), +3..4 segment (0; filled by the stub),
//          +5 flag (1 for the 8080 model)
//   +30 group[ngroups][8], in file order:
//          +0 byte type   +1 byte (reserved)
//          +2 word length_paras   +4 word alloc_paras   +6 word (reserved)
//   +94 word  code_alloc (in-place: original code-group allocation, paragraphs)
//   +96 word  in-place dispatch offset (loader offset of cpm_ip_entry, or 0)
//   +127 byte method (0 = BDOS scratch, 1 = in-place); see the stub
#define CPMINFO_DESC    6
#define CPMINFO_GRP_OFF 30
#define CPMINFO_GRP     8
#define CPMINFO_METHOD  127
#define CPMINFO_MAX     128

/*************************************************************************
//
**************************************************************************/

PackCpm86::PackCpm86(InputFile *f) : super(f) {
    bele = &N_BELE_RTP::le_policy;
    ngroups = 0;
    grp_code = grp_data = grp_extra = grp_stack = nullptr;
    model_8080 = false;
    memset(ih, 0, sizeof(ih));
    memset(groups, 0, sizeof(groups));
}

Linker *PackCpm86::newLinker() const { return new ElfLinkerX86(); }

const int *PackCpm86::getCompressionMethods(int method, int level) const {
    // the i086 nrv2b_d8 decompressor handles the byte-oriented (8) variant
    static const int m_nrv2b[] = {M_NRV2B_8, M_END};
    UNUSED(method);
    UNUSED(level);
    return m_nrv2b;
}

const int *PackCpm86::getFilters() const { return nullptr; }

/*************************************************************************
// header parsing
**************************************************************************/

int PackCpm86::readFileHeader() {
    ngroups = 0;
    grp_code = grp_data = grp_extra = grp_stack = nullptr;
    memset(groups, 0, sizeof(groups));

    fi->seek(0, SEEK_SET);
    if (fi->read(ih, CMD_HDR_SIZE) != (int) CMD_HDR_SIZE)
        return 0;

    // the header tail (after the 8 group descriptors) must be all zero
    for (unsigned i = CMD_NGROUPS * GD_SIZE; i < CMD_HDR_SIZE; i++)
        if (ih[i] != 0)
            return 0;

    unsigned fpos = CMD_HDR_SIZE;
    int n = 0;
    for (unsigned i = 0; i < CMD_NGROUPS; i++) {
        const byte *d = ih + i * GD_SIZE;
        unsigned type = d[0];
        if (type == GT_UNUSED)
            continue;
        if (type > GT_ESCAPE)
            return 0; // not a valid CMD
        Group &g = groups[n];
        g.type = type;
        g.length = get_le16(d + 1);
        g.base = get_le16(d + 3);
        g.min = get_le16(d + 5);
        g.max = get_le16(d + 7);
        g.file_off = fpos;
        fpos += g.length * 16;
        if (type == GT_CODE && !grp_code)
            grp_code = &g;
        else if (type == GT_DATA && !grp_data)
            grp_data = &g;
        else if (type == GT_EXTRA && !grp_extra)
            grp_extra = &g;
        else if (type == GT_STACK && !grp_stack)
            grp_stack = &g;
        n++;
    }
    ngroups = n;
    if (!grp_code || grp_code->length == 0)
        return 0; // must have a code group with content
    image_end = fpos;
    // a CMD file may be shorter than its (paragraph-rounded) group extents (the
    // tail is uninitialized BSS/dictionary space); the stub zero-fills it. Cap the
    // implied zero-fill so a corrupt header can't blow up the work buffer.
    if (fpos > file_size_u + 0x10000)
        return 0;
    model_8080 = (grp_data == nullptr);
    return UPX_F_CPM86_CMD;
}

/*************************************************************************
//
**************************************************************************/

tribool PackCpm86::canPack() {
    byte buf[CMD_HDR_SIZE];

    fi->readx(buf, sizeof(buf));
    fi->seek(0, SEEK_SET);
    // reject DOS MZ / ZM executables outright
    if (memcmp(buf, "MZ", 2) == 0 || memcmp(buf, "ZM", 2) == 0)
        return false;

    const bool has_ext = fn_has_ext(fi->getName(), "cmd");
    // a clean group-descriptor header: first descriptor a code group with a
    // non-zero length (mirrors emu2's cpm86_detect()).
    const bool looks_cmd = (buf[0] == GT_CODE) && (buf[1] || buf[2]);
    if (!has_ext && !looks_cmd)
        return false;

    if (!readFileHeader())
        return false;

    checkAlreadyPacked(buf, sizeof(buf));
    if (file_size < 256)
        throwCantPack("file is too small for cpm86/cmd");
    if (file_size > 0xFE00)
        throwCantPack("file is too large for cpm86/cmd");
    return true;
}

/*************************************************************************
// helpers
**************************************************************************/

// run-time allocation size (paragraphs) for a group, mirroring emu2's loader.
/*static*/ unsigned PackCpm86::allocParas(const Group *g, unsigned type, bool model_8080) {
    if (!g)
        return 0;
    unsigned par = g->max ? g->max : g->length;
    if (par < g->length)
        par = g->length;
    if (type == GT_CODE) {
        if (model_8080 && par < 0x1000)
            par = 0x1000;
    } else if (type == GT_DATA) {
        // CP/M-86 gives the program a full 64K data/stack group unless g_max asks
        // for less (see emu2 cpm86_load_cmd).
        par = (g->max && g->max >= g->length && g->max < 0x1000) ? g->max : 0x1000;
    } else {
        // extra / stack: honor g_max (fall back to g_min)
        par = g->max > g->min ? g->max : g->min;
        if (par < g->length)
            par = g->length;
    }
    return par;
}

/*************************************************************************
//
**************************************************************************/

void PackCpm86::buildLoader(const Filter *ft) {
    UNUSED(ft);
    initLoader(EM_386, stub_i086_cpm86_cmd, sizeof(stub_i086_cpm86_cmd));

    // clang-format off
    addLoader("CPMENTRY");
    // nrv2b_d8 decompressor (same section selection as dos/exe), used as a subroutine
    // CP/M-86 is always an 8086 target, so use the 8086 decompressor variants
    // (the 286 variants use shl reg,imm which does not exist on the 8086). The
    // in-place path decompresses with a small ES, which exercises the d8 match
    // underflow handler that the BDOS-scratch path never reaches.
    addLoader("NRV2B16S",
              ph.u_len > 0xffff ? "N2B64K01" : "",
              "NRV2BEX1",
              "N2BX8601",
              "NRV2BEX2",
              "N2BX8602",
              "NRV2BEX3",
              ph.c_len > 0xffff ? "N2B64K02" : "",
              "NRV2BEX9");
    addLoader("CPMDRET",      // ret (end of decompressor subroutine)
              "CPMTAIL");     // method 0 tail (BDOS scratch)
    if (link_in_place)
        addLoader("CPMIP",    // method 1 tail (in-place, no BDOS)
                  "CPMCOPY"); // method 1 relocated code-copy routine
    addLoader("CPMVARS",
              "CPMINFO",
              "UPX1HEAD",
              "CPMCUT");
    // clang-format on
}

/*************************************************************************
//
**************************************************************************/

void PackCpm86::pack(OutputFile *fo) {
    // We compress the entire original file (header included), so unpack is a
    // verbatim write. The file may be a few bytes shorter than its paragraph-
    // rounded group extents; pad to image_end so each group's full declared data
    // is present, but remember the true size for a lossless unpack.
    unsigned u_len = file_size;
    if (image_end > u_len)
        u_len = image_end;
    ibuf.alloc(u_len);
    obuf.allocForCompression(u_len);
    fi->seek(0, SEEK_SET);
    fi->readx(ibuf, file_size);
    if (u_len > file_size)
        memset(ibuf + file_size, 0, u_len - file_size);

    ph.u_len = u_len;
    ph.u_file_size = file_size;

    // In-place (method 1) is possible only with a data group to use as the
    // decompress buffer and a whole file that fits in one 64K segment.  Link the
    // in-place tail for those; pack() may still fall back to method 0 below.
    const bool ip_feasible = !model_8080 && grp_data != nullptr && u_len <= 0x10000;
    link_in_place = ip_feasible;

    Filter ft(ph.level);
    upx_compress_config_t cconf;
    cconf.reset();
    cconf.conf_ucl.max_match = 0x2000;
    compressWithFilters(&ft, 0, &cconf);

    if (ph.max_run_found + ph.max_match_found > 0x8000)
        throwCantPack("decompressor limit exceeded, send a bugreport");

    // ----- group geometry (model-level; method-independent) -----
    const unsigned code_alloc = allocParas(grp_code, GT_CODE, model_8080);
    const unsigned data_alloc = grp_data ? allocParas(grp_data, GT_DATA, model_8080) : 0;
    const unsigned extra_alloc = grp_extra ? allocParas(grp_extra, GT_EXTRA, model_8080) : 0;
    const unsigned stack_alloc = grp_stack ? allocParas(grp_stack, GT_STACK, model_8080) : 0;
    const unsigned sp_top =
        model_8080 ? 0xFFF0 : (data_alloc >= 0x1000 ? 0xFFF0 : (unsigned) (data_alloc * 16 - 16));
    const unsigned copyr_paras = 0x30; // reserve for the relocated copy routine

    // ----- choose the run-time strategy -----
    // Method 1 (in-place, no BDOS): the loader allocates the original groups and
    // builds the base page; the stub decompresses the whole file into the data
    // group, distributes the slices, and relocates a copy routine to move the code
    // down to code:0.  The stub stages the code over the consumed compressed data
    // (at cs:cpm_src = byte e_len), so the code group is inflated only by the staged
    // code + the copy routine (not by our whole compressed image), and the data
    // group is sized to just hold the whole-file decompress buffer.
    //
    // KNOWN LIMITATION (runtime memory): a packed program needs slightly more
    // memory than the original while running.  In-place adds a flat ~180 paragraphs
    // (~2.9 KB: the loader code + copy routine) on top of the original's footprint,
    // independent of file size.  The 8080 model has no data group to decompress
    // into, so it falls back to method 0, which transiently MC_ALLOCs a whole-file
    // scratch buffer (freed before the program runs) -- a larger, file-size-
    // dependent peak.  Programs that run close to the memory limit may therefore
    // fail to start after packing; this is inherent to decompress-at-load.
    //
    // compressWithFilters already built the in-place loader (link_in_place set
    // above).  Prefer in-place, but if it would not meet the compression ratio
    // (its tail enlarges the loader), rebuild a leaner method-0-only loader so
    // marginal files still pack.
    unsigned e_len = getLoaderSize();
    unsigned code_alloc_ip = 0, data_alloc_ip = 0, sp_top_ip = 0;
    bool in_place = false;
    if (ip_feasible) {
        const unsigned stage_paras = (e_len + 15) / 16; // staging starts at cs:cpm_src
        code_alloc_ip = stage_paras + grp_code->length + copyr_paras;
        if (code_alloc_ip < code_alloc)
            code_alloc_ip = code_alloc;    // room for the original code group's BSS
        data_alloc_ip = (u_len + 15) / 16; // big enough for the whole-file buffer ...
        if (data_alloc_ip < data_alloc)
            data_alloc_ip = data_alloc; // ... and the program's own data allocation
        sp_top_ip = (data_alloc_ip >= 0x1000) ? 0xFFF0 : (unsigned) (data_alloc_ip * 16 - 16);
        const unsigned packed_ip = 128 + ((e_len + ph.c_len + 15) / 16) * 16;
        if (code_alloc_ip <= 0x1000 && checkDefaultCompressionRatio(file_size, packed_ip))
            in_place = true;
    }
    if (!in_place && link_in_place) {
        link_in_place = false; // rebuild without the in-place tail (leaner loader)
        buildLoader(&ft);
        e_len = getLoaderSize();
    }
    const unsigned method = in_place ? 1u : 0u;
    const unsigned sp_top_eff = in_place ? sp_top_ip : sp_top;
    const unsigned code_bytes = e_len + ph.c_len;
    const unsigned code_file_paras = (code_bytes + 15) / 16;

    relocateLoader();
    MemBuffer loader(e_len);
    memcpy(loader, getLoader(), e_len);

    // ----- build CPMINFO (see the stub) -----
    byte info[CPMINFO_MAX];
    memset(info, 0, sizeof(info));
    info[0] = (byte) ngroups;
    info[1] = (byte) (model_8080 ? 1 : 0);
    set_le16(info + 2, (u_len + 15) / 16); // whole-file paragraphs
    set_le16(info + 4, sp_top_eff);
    set_le16(info + 94, code_alloc); // in-place: original code-group allocation (paras)
    // in-place dispatch target: the loader offset of cpm_ip_entry (= CPMIP section),
    // read by the entry stub's indirect jump.  0 when the in-place tail is absent.
    set_le16(info + 96, in_place ? (unsigned) getLoaderSection("CPMIP") : 0u);
    info[CPMINFO_METHOD] = (byte) method;
    // base-page group descriptors (method 0 rebuilds the base page from these; the
    // segment word is filled in by the stub at run time)
    const byte flag = (byte) (model_8080 ? 1 : 0);
#define SET_DESC(slot, len24)                                                                      \
    do {                                                                                           \
        byte *d_ = info + CPMINFO_DESC + (slot) *6;                                                \
        set_le24(d_, (len24));                                                                     \
        d_[5] = flag;                                                                              \
    } while (0)
    SET_DESC(0, code_alloc * 16);
    if (grp_data || model_8080)
        SET_DESC(1, sp_top_eff);
    if (grp_extra)
        SET_DESC(2, extra_alloc * 16);
    if (grp_stack)
        SET_DESC(3, stack_alloc * 16);
#undef SET_DESC
    for (int i = 0; i < ngroups; i++) {
        const Group &g = groups[i];
        byte *e = info + CPMINFO_GRP_OFF + i * CPMINFO_GRP;
        e[0] = (byte) g.type;
        set_le16(e + 2, g.length);
        unsigned a = allocParas(&g, g.type, model_8080);
        if (in_place && g.type == GT_DATA)
            a = data_alloc_ip; // sized to hold the whole-file decompress buffer
        set_le16(e + 4, a);
    }

    int info_off = getLoaderSection("CPMINFO");
    assert(info_off > 0);
    memcpy(loader + info_off, info, CPMINFO_MAX);
    patchPackHeader(loader, e_len);

    // ----- emit the packed .cmd header -----
    byte hdr[CMD_HDR_SIZE];
    memset(hdr, 0, sizeof(hdr));
    if (in_place) {
        // Mirror the original group structure so the CP/M-86 loader allocates the
        // data/extra/stack segments and builds the base page.  Only the code group
        // carries file content (our stub + compressed image); the others have
        // length 0 (the stub fills them from the decompressed image).
        int slot = 0;
        byte *d = hdr + slot++ * GD_SIZE;
        d[0] = GT_CODE;
        set_le16(d + 1, code_file_paras); // length stored in the file
        set_le16(d + 5, code_alloc_ip);   // min = inflated (room to stage code + copyr)
        set_le16(d + 7, code_alloc_ip);   // max
        d = hdr + slot++ * GD_SIZE;
        d[0] = GT_DATA; // length 0; the stub fills it from the decompressed image
        set_le16(d + 1, 0);
        set_le16(d + 5, data_alloc_ip);
        set_le16(d + 7, data_alloc_ip);
        if (grp_extra) {
            d = hdr + slot++ * GD_SIZE;
            d[0] = GT_EXTRA;
            set_le16(d + 5, extra_alloc);
            set_le16(d + 7, extra_alloc);
        }
        if (grp_stack) {
            d = hdr + slot++ * GD_SIZE;
            d[0] = GT_STACK;
            set_le16(d + 5, stack_alloc);
            set_le16(d + 7, stack_alloc);
        }
    } else {
        // Method 0: a small-model cmd (code group + a tiny scratch data group for
        // the base page and the stub's stack).
        const unsigned scratch_data_paras = 0x80;
        hdr[0] = GT_CODE;
        set_le16(hdr + 1, code_file_paras);
        set_le16(hdr + 5, code_file_paras);
        set_le16(hdr + 7, code_file_paras);
        hdr[9 + 0] = GT_DATA;
        set_le16(hdr + 9 + 5, scratch_data_paras);
        set_le16(hdr + 9 + 7, scratch_data_paras);
    }

    // write header + code group (loader + compressed, zero-padded to a paragraph)
    fo->write(hdr, sizeof(hdr));
    fo->write(loader, e_len);
    fo->write(obuf, ph.c_len);
    const unsigned pad = code_file_paras * 16 - code_bytes;
    if (pad) {
        byte zero[16];
        memset(zero, 0, sizeof(zero));
        fo->write(zero, pad);
    }

    if (!checkFinalCompressionRatio(fo))
        throwNotCompressible();
}

/*************************************************************************
//
**************************************************************************/

tribool PackCpm86::canUnpack() {
    if (!readFileHeader())
        return false;
    // the loader + pack header live at the start of the code group
    fi->seek(grp_code->file_off, SEEK_SET);
    if (!readPackHeader(4096))
        return false;
    return file_size_u > grp_code->file_off + ph.c_len;
}

/*************************************************************************
//
**************************************************************************/

void PackCpm86::unpack(OutputFile *fo) {
    ibuf.alloc(file_size);
    obuf.allocForDecompression(ph.u_len);

    fi->seek(0, SEEK_SET);
    fi->readx(ibuf, file_size);

    // compressed data starts right after the pack header in the code group
    unsigned e_len = ph.buf_offset + ph.getPackHeaderSize();
    unsigned c_off = grp_code->file_off + e_len;
    if (file_size_u < c_off + ph.c_len)
        throwCantUnpack("file damaged");

    // decompress -> the entire original file (header + all group data)
    decompress(ibuf + c_off, obuf);

    // ph.u_len may include padding past the real end of file; write only the
    // original byte count for a lossless result.  u_file_size is taken verbatim
    // from the pack header and must not exceed u_len (the obuf size), else the
    // write below would read past the decompression buffer.
    unsigned out_len = ph.u_file_size ? ph.u_file_size : ph.u_len;
    if (out_len > ph.u_len)
        throwCantUnpack("file damaged");
    if (fo)
        fo->write(obuf, out_len);
}

/* vim:set ts=4 sw=4 et: */
