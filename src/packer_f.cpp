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
// filter util; see class FilterImpl
**************************************************************************/

bool Packer::isValidFilter(int filter_id) const {
    return Filter::isValidFilter(filter_id, getFilters());
}

/*************************************************************************
// addFilter32
**************************************************************************/

#define NOFILT 0 // no filter
#define FNOMRU 1 // filter, but not using mru
#define MRUFLT 2 // mru filter

static inline unsigned f80_call(int filter_id) { return (1 + (0x0f & filter_id)) % 3; }

static inline unsigned f80_jmp1(int filter_id) { return ((1 + (0x0f & filter_id)) / 3) % 3; }

static inline unsigned f80_jcc2(int filter_id) { return f80_jmp1(filter_id); }

void Packer::addFilter32(int filter_id) {
    assert(filter_id > 0);
    assert(isValidFilter(filter_id));

    if (filter_id < 0x80) {
        if (0x50 == (0xF0 & filter_id)) {
            addLoader("ctok32.00",
                      ((0x50 == filter_id)   ? "ctok32.50"
                       : (0x51 == filter_id) ? "ctok32.51"
                                             : ""),
                      "ctok32.10");
        } else if ((filter_id & 0xf) % 3 == 0) {
            if (filter_id < 0x40) {
                addLoader("CALLTR00", (filter_id > 0x20) ? "CTCLEVE1" : "", "CALLTR01",
                          (filter_id & 0xf) > 3
                              ? (filter_id > 0x20 ? "CTBSHR01,CTBSWA01" : "CTBROR01,CTBSWA01")
                              : "",
                          "CALLTR02");
            } else if (0x40 == (0xF0 & filter_id)) {
                addLoader("ctok32.00");
                if (9 <= (0xf & filter_id)) {
                    addLoader("ctok32.10");
                }
                addLoader("ctok32.20");
                if (9 <= (0xf & filter_id)) {
                    addLoader("ctok32.30");
                }
                addLoader("ctok32.40");
            }
        } else
            addLoader("CALLTR10", (filter_id & 0xf) % 3 == 1 ? "CALLTRE8" : "CALLTRE9", "CALLTR11",
                      (filter_id > 0x20) ? "CTCLEVE2" : "", "CALLTR12",
                      (filter_id & 0xf) > 3
                          ? (filter_id > 0x20 ? "CTBSHR11,CTBSWA11" : "CTBROR11,CTBSWA11")
                          : "",
                      "CALLTR13");
    }
    if (0x80 == (filter_id & 0xF0)) {
        const bool x386 = (opt->cpu_x86 <= opt->CPU_386);
        const unsigned n_mru = ph.n_mru ? 1 + ph.n_mru : 0;
        const bool mrupwr2 = (0 != n_mru) && 0 == ((n_mru - 1) & n_mru);
        const unsigned f_call = f80_call(filter_id);
        const unsigned f_jmp1 = f80_jmp1(filter_id);
        const unsigned f_jcc2 = f80_jcc2(filter_id);

        if (NOFILT != f_jcc2) {
            addLoader("LXJCC010");
            if (n_mru) {
                addLoader("LXMRU045");
            } else {
                addLoader("LXMRU046");
            }
            if (0 == n_mru || MRUFLT != f_jcc2) {
                addLoader("LXJCC020");
            } else { // 0 != n_mru
                addLoader("LXJCC021");
            }
            if (NOFILT != f_jcc2) {
                addLoader("LXJCC023");
            }
        }
        addLoader("LXUNF037");
        if (x386) {
            if (n_mru) {
                addLoader("LXUNF386");
            }
            addLoader("LXUNF387");
            if (n_mru) {
                addLoader("LXUNF388");
            }
        } else {
            addLoader("LXUNF486");
            if (n_mru) {
                addLoader("LXUNF487");
            }
        }
        if (n_mru) {
            addLoader("LXMRU065");
            if (256 == n_mru) {
                addLoader("MRUBYTE3");
            } else {
                addLoader("MRUARB30");
                if (mrupwr2) {
                    addLoader("MRUBITS3");
                } else {
                    addLoader("MRUARB40");
                }
            }
            addLoader("LXMRU070");
            if (256 == n_mru) {
                addLoader("MRUBYTE4");
            } else if (mrupwr2) {
                addLoader("MRUBITS4");
            } else {
                addLoader("MRUARB50");
            }
            addLoader("LXMRU080");
            if (256 == n_mru) {
                addLoader("MRUBYTE5");
            } else {
                addLoader("MRUARB60");
                if (mrupwr2) {
                    addLoader("MRUBITS5");
                } else {
                    addLoader("MRUARB70");
                }
            }
            addLoader("LXMRU090");
            if (256 == n_mru) {
                addLoader("MRUBYTE6");
            } else {
                addLoader("MRUARB80");
                if (mrupwr2) {
                    addLoader("MRUBITS6");
                } else {
                    addLoader("MRUARB90");
                }
            }
            addLoader("LXMRU100");
        }
        addLoader("LXUNF040");
        if (n_mru) {
            addLoader("LXMRU110");
        } else {
            addLoader("LXMRU111");
        }
        addLoader("LXUNF041");
        addLoader("LXUNF042");
        if (n_mru) {
            addLoader("LXMRU010");
            if (NOFILT != f_jmp1 && NOFILT == f_call) {
                addLoader("LXJMPA00");
            } else {
                addLoader("LXCALLB0");
            }
            addLoader("LXUNF021");
        } else {
            addLoader("LXMRU022");
            if (NOFILT != f_jmp1 && NOFILT == f_call) {
                addLoader("LXJMPA01");
            } else {
                addLoader("LXCALLB1");
            }
        }
        if (n_mru) {
            if (256 != n_mru && mrupwr2) {
                addLoader("MRUBITS1");
            }
            addLoader("LXMRU030");
            if (256 == n_mru) {
                addLoader("MRUBYTE1");
            } else {
                addLoader("MRUARB10");
            }
            addLoader("LXMRU040");
        }

        addLoader("LXUNF030");
        if (NOFILT != f_jcc2) {
            addLoader("LXJCC000");
        }
        if (NOFILT != f_call || NOFILT != f_jmp1) { // at least one is filtered
            // shift opcode origin to zero
            if (0 == n_mru) {
                addLoader("LXCJ0MRU");
            } else {
                addLoader("LXCJ1MRU");
            }

            // determine if in range
            if ((NOFILT != f_call) && (NOFILT != f_jmp1)) { // unfilter both
                addLoader("LXCALJMP");
            }
            if ((NOFILT == f_call) ^ (NOFILT == f_jmp1)) { // unfilter just one
                if (0 == n_mru) {
                    addLoader("LXCALL00");
                } else {
                    addLoader("LXCALL01");
                }
            }

            // determine if mru applies
            if (0 == n_mru || !((FNOMRU == f_call) || (FNOMRU == f_jmp1))) {
                // no mru, or no exceptions
                addLoader("LXCJ2MRU");
            } else {
                // mru on one, but not the other
                addLoader("LXCJ4MRU");
                if (MRUFLT == f_jmp1) { // JMP only
                    addLoader("LXCJ6MRU");
                } else if (MRUFLT == f_call) { // CALL only
                    addLoader("LXCJ7MRU");
                }
                addLoader("LXCJ8MRU");
            }
        }
        addLoader("LXUNF034");
        if (n_mru) {
            addLoader("LXMRU055");
            if (256 == n_mru) {
                addLoader("MRUBYTE2");
            } else if (mrupwr2) {
                addLoader("MRUBITS2");
            } else {
                addLoader("MRUARB20");
            }
            addLoader("LXMRU057");
        }
    }
}

#undef NOFILT
#undef FNOMRU
#undef MRUFLT

/*************************************************************************
//
**************************************************************************/

void Packer::defineFilterSymbols(const Filter *ft) {
    if (ft->id == 0) {
        linker->defineSymbol("filter_length", 0);
        linker->defineSymbol("filter_cto", 0);
        return;
    }
    assert(ft->calls > 0);
    assert(ft->buf_len > 0);

    if (ft->id >= 0x40 && ft->id <= 0x4f) {
        linker->defineSymbol("filter_length", ft->buf_len);
        linker->defineSymbol("filter_cto", ft->cto);
    } else if (ft->id >= 0x50 && ft->id <= 0x5f) {
        linker->defineSymbol("filter_id", ft->id);
        linker->defineSymbol("filter_cto", ft->cto);
    } else if ((ft->id & 0xf) % 3 == 0) {
        linker->defineSymbol("filter_length", ft->calls);
        linker->defineSymbol("filter_cto", ft->cto);
    } else {
        linker->defineSymbol("filter_length", ft->lastcall - ft->calls * 4);
        linker->defineSymbol("filter_cto", ft->cto);
    }

#if 0
    if (0x80 == (ft->id & 0xF0)) {
        const int mru = ph.n_mru ? 1 + ph.n_mru : 0;
        if (mru && mru != 256) {
            const unsigned is_pwr2 = (0 == ((mru - 1) & mru));
            // patch_le32(0x80 + (char *) loader, lsize - 0x80, "NMRU", mru - is_pwr2);
        }
    }
#endif
}

/* vim:set ts=4 sw=4 et: */
