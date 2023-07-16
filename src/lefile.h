/* lefile.h --

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

#include "util/membuffer.h"

class InputFile;
class OutputFile;

/*************************************************************************
//
**************************************************************************/

class LeFile {
protected:
    explicit LeFile(InputFile *) noexcept;
    virtual ~LeFile() noexcept;

    virtual bool readFileHeader();
    virtual void writeFile(OutputFile *, bool);

protected:
    enum { FIXUP_EXTRA = 3 };

    struct alignas(1) le_header_t {
        // 0x00
        byte _[2];             // signature: 'LE' || 'LX'
        byte byte_order;       // 0 little endian
        byte word_order;       // 0 little endian
        LE32 exe_format_level; // 0
        LE16 cpu_type;         // 1->286..4->586
        LE16 target_os;        // 1->OS2
        byte _0[4];            // module_version = 0
        // 0x10
        LE32 module_type; // 0x200->compatible with PM windowing
        LE32 memory_pages;
        LE32 init_cs_object;
        LE32 init_eip_offset;
        // 0x20
        LE32 init_ss_object;
        LE32 init_esp_offset;
        LE32 memory_page_size;
        LE32 bytes_on_last_page;
        // 0x30
        LE32 fixup_size;
        byte _1[4]; // fixup_checksum = 0
        LE32 loader_size;
        byte _2[4]; // loader_checksum = 0
        // 0x40
        LE32 object_table_offset;
        LE32 object_table_entries;
        LE32 object_pagemap_offset;
        LE32 object_iterate_data_map_offset;
        // 0x50
        byte _3[4]; //  resource_offset
        LE32 resource_entries;
        LE32 resident_names_offset;
        LE32 entry_table_offset;
        // 0x60
        byte _4[4]; //  module_directives_table_offset = 0
        LE32 module_directives_entries;
        LE32 fixup_page_table_offset;
        LE32 fixup_record_table_offset;
        // 0x70
        LE32 imported_modules_name_table_offset;
        LE32 imported_modules_count;
        LE32 imported_procedures_name_table_offset;
        byte _5[4]; // per_page_checksum_table_offset =  0
        // 0x80
        LE32 data_pages_offset;
        byte _6[4]; // preload_page_count = 0
        LE32 non_resident_name_table_offset;
        LE32 non_resident_name_table_length;
        // 0x90
        byte _7[4]; // non_resident_names_checksum
        LE32 automatic_data_object;
#if 1
        byte _8[44];
#else
        LE32 debug_info_offset;
        LE32 debug_info_length;
        // 0xA0
        LE32 preload_instance_pages;
        LE32 demand_instance_pages;
        LE32 extra_heap_alloc;
        byte reserved[12];
        LE32 versioninfo;
        LE32 unknown;
        // 0xC0
        LE16 device_id;
        LE16 ddk_version;
#endif
    };

    struct alignas(1) le_object_table_entry_t {
        LE32 virtual_size;
        LE32 base_address;
        LE32 flags;
        LE32 pagemap_index;
        LE32 npages;
        LE32 reserved;
    };

    struct alignas(1) le_pagemap_entry_t {
        byte h;
        byte m;
        byte l;
        byte type; // 0x00-legal;0x40-iterated;0x80-invalid;0xC0-zeroed
    };

    virtual void readObjectTable();
    virtual void writeObjectTable();
    // virtual void encodeObjectTable(){oobject_table = iobject_table; iobject_table = nullptr;}
    // virtual void decodeObjectTable(){encodeObjectTable();}

    virtual void readFixupPageTable();
    virtual void writeFixupPageTable();
    // virtual void encodeFixupPageTable(){ofpage_table = ifpage_table; ifpage_table = nullptr;}
    // virtual void decodeFixupPageTable(){encodeFixupPageTable();}

    virtual void readPageMap();
    virtual void writePageMap();
    virtual void encodePageMap() {
        opm_entries = ipm_entries;
        ipm_entries = nullptr;
    }
    virtual void decodePageMap() { encodePageMap(); }

    virtual void readResidentNames();
    virtual void writeResidentNames();
    virtual void encodeResidentNames() {
        ores_names = ires_names;
        ires_names = nullptr;
    }
    virtual void decodeResidentNames() { encodeResidentNames(); }

    virtual void readNonResidentNames();
    virtual void writeNonResidentNames();
    virtual void encodeNonResidentNames() {
        ononres_names = inonres_names;
        inonres_names = nullptr;
    }
    virtual void decodeNonResidentNames() { encodeNonResidentNames(); }

    virtual void readEntryTable();
    virtual void writeEntryTable();
    // virtual void encodeEntryTable(){oentries = ientries; ientries = nullptr;}
    // virtual void decodeEntryTable(){encodeEntryTable();}

    virtual void readFixups();
    virtual void writeFixups();
    // virtual void encodeFixups(){ofixups = ifixups; ifixups = nullptr;}
    // virtual void decodeFixups(){encodeFixups();}

    virtual void readImage();
    virtual void writeImage();
    // virtual void encodeImage(){oimage = iimage; iimage = nullptr;}
    // virtual void decodeImage(){encodeImage();}

    void countFixups(unsigned *) const;
    unsigned getImageSize() const;

    InputFile *fif = nullptr;
    OutputFile *fof = nullptr;
    unsigned le_offset = 0;
    unsigned exe_offset = 0;

    le_header_t ih;
    le_header_t oh;

    le_object_table_entry_t *iobject_table = nullptr;
    le_object_table_entry_t *oobject_table = nullptr;
    unsigned *ifpage_table = nullptr;
    unsigned *ofpage_table = nullptr;
    le_pagemap_entry_t *ipm_entries = nullptr;
    le_pagemap_entry_t *opm_entries = nullptr;
    byte *ires_names = nullptr;
    byte *ores_names = nullptr;
    byte *ifixups = nullptr;
    byte *ofixups = nullptr;
    byte *inonres_names = nullptr;
    byte *ononres_names = nullptr;
    MemBuffer mb_iimage;
    SPAN_0(byte) iimage = nullptr;
    MemBuffer mb_oimage;
    SPAN_0(byte) oimage = nullptr;
    byte *ientries = nullptr;
    byte *oentries = nullptr;

    unsigned soobject_table = 0;
    unsigned sofpage_table = 0;
    unsigned sopm_entries = 0;
    unsigned sores_names = 0;
    unsigned sofixups = 0;
    unsigned sononres_names = 0;
    unsigned soimage = 0;
    unsigned soentries = 0;

private:
    // disable copy and move
    LeFile(const LeFile &) DELETED_FUNCTION;
    LeFile &operator=(const LeFile &) DELETED_FUNCTION;
    LeFile(LeFile &&) noexcept DELETED_FUNCTION;
    LeFile &operator=(LeFile &&) noexcept DELETED_FUNCTION;
};

/* vim:set ts=4 sw=4 et: */
