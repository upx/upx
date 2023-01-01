/* packer_f.cpp -- Packer filter handling

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


#include "conf.h"
#include "packer.h"
#include "filter.h"
#include "linker.h"


/*************************************************************************
// filter util
**************************************************************************/

bool Packer::isValidFilter(int filter_id) const
{
    return Filter::isValidFilter(filter_id, getFilters());
}


/*************************************************************************
// addFilter32
**************************************************************************/

#define NOFILT 0  // no filter
#define FNOMRU 1  // filter, but not using mru
#define MRUFLT 2  // mru filter

static inline unsigned f80_call(int filter_id)
{
    return (1+ (0x0f & filter_id)) % 3;
}

static inline unsigned f80_jmp1(int filter_id)
{
    return ((1+ (0x0f & filter_id)) / 3) % 3;
}

static inline unsigned f80_jcc2(int filter_id)
{
    return f80_jmp1(filter_id);
}


void Packer::addFilter32(int filter_id)
{
    assert(filter_id > 0);
    assert(isValidFilter(filter_id));

    if (filter_id < 0x80) {
        if (0x50==(0xF0 & filter_id)) {
            addLoader("ctok32.00",
                ((0x50==filter_id) ? "ctok32.50" :
                 (0x51==filter_id) ? "ctok32.51" : ""),
                "ctok32.10", nullptr);
        }
        else if ((filter_id & 0xf) % 3 == 0) {
            if (filter_id < 0x40) {
                addLoader("CALLTR00",
                        (filter_id > 0x20) ? "CTCLEVE1" : "",
                        "CALLTR01",
                        (filter_id & 0xf) > 3 ? (filter_id > 0x20 ? "CTBSHR01,CTBSWA01" : "CTBROR01,CTBSWA01") : "",
                        "CALLTR02",
                        nullptr
                        );
            }
            else if (0x40==(0xF0 & filter_id)) {
                addLoader("ctok32.00", nullptr);
                if (9<=(0xf & filter_id)) {
                    addLoader("ctok32.10", nullptr);
                }
                addLoader("ctok32.20", nullptr);
                if (9<=(0xf & filter_id)) {
                    addLoader("ctok32.30", nullptr);
                }
                addLoader("ctok32.40", nullptr);
            }
        }
        else
            addLoader("CALLTR10",
                    (filter_id & 0xf) % 3  == 1 ? "CALLTRE8" : "CALLTRE9",
                    "CALLTR11",
                    (filter_id > 0x20) ? "CTCLEVE2" : "",
                    "CALLTR12",
                    (filter_id & 0xf) > 3 ? (filter_id > 0x20 ? "CTBSHR11,CTBSWA11" : "CTBROR11,CTBSWA11") : "",
                    "CALLTR13",
                    nullptr
                    );
    }
    if (0x80==(filter_id & 0xF0)) {
        bool const x386 = (opt->cpu <= opt->CPU_386);
        unsigned const n_mru = ph.n_mru ? 1+ ph.n_mru : 0;
        bool const mrupwr2 = (0!=n_mru) && 0==((n_mru -1) & n_mru);
        unsigned const f_call = f80_call(filter_id);
        unsigned const f_jmp1 = f80_jmp1(filter_id);
        unsigned const f_jcc2 = f80_jcc2(filter_id);

        if (NOFILT!=f_jcc2) {
                addLoader("LXJCC010", nullptr);
                if (n_mru) {
                    addLoader("LXMRU045", nullptr);
                }
                else {
                    addLoader("LXMRU046", nullptr);
                }
                if (0==n_mru || MRUFLT!=f_jcc2) {
                    addLoader("LXJCC020", nullptr);
                }
                else { // 0!=n_mru
                    addLoader("LXJCC021", nullptr);
                }
                if (NOFILT!=f_jcc2) {
                    addLoader("LXJCC023", nullptr);
                }
        }
        addLoader("LXUNF037", nullptr);
        if (x386) {
            if (n_mru) {
                addLoader("LXUNF386", nullptr);
            }
            addLoader("LXUNF387", nullptr);
            if (n_mru) {
                addLoader("LXUNF388", nullptr);
            }
        }
        else {
            addLoader("LXUNF486", nullptr);
            if (n_mru) {
                addLoader("LXUNF487", nullptr);
            }
        }
        if (n_mru) {
            addLoader("LXMRU065", nullptr);
            if (256==n_mru) {
                addLoader("MRUBYTE3", nullptr);
            }
            else {
                addLoader("MRUARB30", nullptr);
                if (mrupwr2) {
                    addLoader("MRUBITS3", nullptr);
                }
                else {
                    addLoader("MRUARB40", nullptr);
                }
            }
            addLoader("LXMRU070", nullptr);
            if (256==n_mru) {
                addLoader("MRUBYTE4", nullptr);
            }
            else if (mrupwr2) {
                addLoader("MRUBITS4", nullptr);
            }
            else {
                addLoader("MRUARB50", nullptr);
            }
            addLoader("LXMRU080", nullptr);
            if (256==n_mru) {
                addLoader("MRUBYTE5", nullptr);
            }
            else {
                addLoader("MRUARB60", nullptr);
                if (mrupwr2) {
                    addLoader("MRUBITS5", nullptr);
                }
                else {
                    addLoader("MRUARB70", nullptr);
                }
            }
            addLoader("LXMRU090", nullptr);
            if (256==n_mru) {
                addLoader("MRUBYTE6", nullptr);
            }
            else {
                addLoader("MRUARB80", nullptr);
                if (mrupwr2) {
                        addLoader("MRUBITS6", nullptr);
                }
                else {
                    addLoader("MRUARB90", nullptr);
                }
            }
            addLoader("LXMRU100", nullptr);
        }
        addLoader("LXUNF040", nullptr);
        if (n_mru) {
            addLoader("LXMRU110", nullptr);
        }
        else {
            addLoader("LXMRU111", nullptr);
        }
        addLoader("LXUNF041", nullptr);
        addLoader("LXUNF042", nullptr);
        if (n_mru) {
            addLoader("LXMRU010", nullptr);
            if (NOFILT!=f_jmp1 && NOFILT==f_call) {
                addLoader("LXJMPA00", nullptr);
            }
            else {
                addLoader("LXCALLB0", nullptr);
            }
            addLoader("LXUNF021", nullptr);
        }
        else {
            addLoader("LXMRU022", nullptr);
            if (NOFILT!=f_jmp1 && NOFILT==f_call) {
                addLoader("LXJMPA01", nullptr);
            }
            else {
                addLoader("LXCALLB1", nullptr);
            }
        }
        if (n_mru) {
            if (256!=n_mru && mrupwr2) {
                addLoader("MRUBITS1", nullptr);
            }
            addLoader("LXMRU030", nullptr);
            if (256==n_mru) {
                addLoader("MRUBYTE1", nullptr);
            }
            else {
                addLoader("MRUARB10", nullptr);
            }
            addLoader("LXMRU040", nullptr);
        }

        addLoader("LXUNF030", nullptr);
        if (NOFILT!=f_jcc2) {
            addLoader("LXJCC000", nullptr);
        }
        if (NOFILT!=f_call || NOFILT!=f_jmp1) { // at least one is filtered
            // shift opcode origin to zero
            if (0==n_mru) {
                addLoader("LXCJ0MRU", nullptr);
            }
            else {
                addLoader("LXCJ1MRU", nullptr);
            }

            // determine if in range
            if ((NOFILT!=f_call) && (NOFILT!=f_jmp1)) { // unfilter both
                addLoader("LXCALJMP", nullptr);
            }
            if ((NOFILT==f_call) ^  (NOFILT==f_jmp1)) { // unfilter just one
                if (0==n_mru) {
                    addLoader("LXCALL00", nullptr);
                }
                else {
                    addLoader("LXCALL01", nullptr);
                }
            }

            // determine if mru applies
            if (0==n_mru || ! ((FNOMRU==f_call) || (FNOMRU==f_jmp1)) ) {
                addLoader("LXCJ2MRU", nullptr);  // no mru, or no exceptions
            }
            else {  // mru on one, but not the other
                addLoader("LXCJ4MRU", nullptr);
                if (MRUFLT==f_jmp1) { // JMP only
                    addLoader("LXCJ6MRU", nullptr);
                } else
                if (MRUFLT==f_call) { // CALL only
                    addLoader("LXCJ7MRU", nullptr);
                }
                addLoader("LXCJ8MRU", nullptr);
            }
        }
        addLoader("LXUNF034", nullptr);
        if (n_mru) {
            addLoader("LXMRU055", nullptr);
            if (256==n_mru) {
                addLoader("MRUBYTE2", nullptr);
            }
            else if (mrupwr2) {
                addLoader("MRUBITS2", nullptr);
            }
            else if (n_mru) {
                addLoader("MRUARB20", nullptr);
            }
            addLoader("LXMRU057", nullptr);
        }
    }
}

#undef NOFILT
#undef FNOMRU
#undef MRUFLT


/*************************************************************************
//
**************************************************************************/

void Packer::defineFilterSymbols(const Filter *ft)
{
    if (ft->id == 0)
    {
        linker->defineSymbol("filter_length", 0);
        linker->defineSymbol("filter_cto", 0);
        return;
    }
    assert(ft->calls > 0);
    assert(ft->buf_len > 0);

    if (ft->id >= 0x40 && ft->id <= 0x4f)
    {
        linker->defineSymbol("filter_length", ft->buf_len);
        linker->defineSymbol("filter_cto", ft->cto);
    }
    else if (ft->id >= 0x50 && ft->id <= 0x5f)
    {
        linker->defineSymbol("filter_id", ft->id);
        linker->defineSymbol("filter_cto", ft->cto);
    }
    else if ((ft->id & 0xf) % 3 == 0)
    {
        linker->defineSymbol("filter_length", ft->calls);
        linker->defineSymbol("filter_cto", ft->cto);
    }
    else
    {
        linker->defineSymbol("filter_length", ft->lastcall - ft->calls * 4);
        linker->defineSymbol("filter_cto", ft->cto);
    }

#if 0
    if (0x80==(ft->id & 0xF0)) {
        int const mru = ph.n_mru ? 1+ ph.n_mru : 0;
        if (mru && mru!=256) {
            unsigned const is_pwr2 = (0==((mru -1) & mru));
            //patch_le32(0x80 + (char *)loader, lsize - 0x80, "NMRU", mru - is_pwr2);
        }
    }
#endif
}

/* vim:set ts=4 sw=4 et: */
