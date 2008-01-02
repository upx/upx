/* lefile.h --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2008 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2008 Laszlo Molnar
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
   <markus@oberhumer.com>               <ml1050@users.sourceforge.net>
 */


#ifndef __UPX_LEFILE_H
#define __UPX_LEFILE_H

class InputFile;
class OutputFile;


/*************************************************************************
//
**************************************************************************/

class LeFile
{
public:
    LeFile(InputFile *);
    virtual ~LeFile();

    virtual bool readFileHeader();
    virtual void writeFile(OutputFile *, bool);

protected:
    enum { FIXUP_EXTRA = 3 };

    struct le_header_t
    {   // 0x00
        char  _[2];             // signature: 'LE' || 'LX'
        char  byte_order;       // 0 little endian
        char  word_order;       // 0 little endian
        LE32  exe_format_level; // 0
        LE16  cpu_type;         // 1->286..4->586
        LE16  target_os;        // 1->OS2
        char  _0[4];            // module_version = 0
        // 0x10
        LE32  module_type;      // 0x200->compatible with PM windowing
        LE32  memory_pages;
        LE32  init_cs_object;
        LE32  init_eip_offset;
        // 0x20
        LE32  init_ss_object;
        LE32  init_esp_offset;
        LE32  memory_page_size;
        LE32  bytes_on_last_page;
        // 0x30
        LE32  fixup_size;
        char  _1[4];            // fixup_checksum = 0
        LE32  loader_size;
        char  _2[4];            // loader_checksum = 0
        // 0x40
        LE32  object_table_offset;
        LE32  object_table_entries;
        LE32  object_pagemap_offset;
        LE32  object_iterate_data_map_offset;
        // 0x50
        char  _3[4];            //  resource_offset
        LE32  resource_entries;
        LE32  resident_names_offset;
        LE32  entry_table_offset;
        // 0x60
        char  _4[4];            //  module_directives_table_offset = 0
        LE32  module_directives_entries;
        LE32  fixup_page_table_offset;
        LE32  fixup_record_table_offset;
        // 0x70
        LE32  imported_modules_name_table_offset;
        LE32  imported_modules_count;
        LE32  imported_procedures_name_table_offset;
        char  _5[4];            // per_page_checksum_table_offset =  0
        // 0x80
        LE32  data_pages_offset;
        char  _6[4];            // preload_page_count = 0
        LE32  non_resident_name_table_offset;
        LE32  non_resident_name_table_length;
        // 0x90
        char  _7[4]; //non_resident_names_checksum
        LE32  automatic_data_object;
#if 1
        char  _8[44];
#else
        LE32  debug_info_offset;
        LE32  debug_info_length;
        // 0xA0
        LE32  preload_instance_pages;
        LE32  demand_instance_pages;
        LE32  extra_heap_alloc;
        char  reserved[12];
        LE32  versioninfo;
        LE32  unkown;
        // 0xC0
        LE16  device_id;
        LE16  ddk_version;
#endif
    }
    __attribute_packed;

    struct le_object_table_entry_t
    {
        LE32  virtual_size;
        LE32  base_address;
        LE32  flags;
        LE32  pagemap_index;
        LE32  npages;
        LE32  reserved;
    }
    __attribute_packed;

    struct le_pagemap_entry_t
    {
        unsigned char h;
        unsigned char m;
        unsigned char l;
        unsigned char type;  // 0x00-legal;0x40-iterated;0x80-invalid;0xC0-zeroed
    }
    __attribute_packed;

    virtual void readObjectTable();
    virtual void writeObjectTable();
    //virtual void encodeObjectTable(){oobject_table = iobject_table; iobject_table = NULL;}
    //virtual void decodeObjectTable(){encodeObjectTable();}

    virtual void readFixupPageTable();
    virtual void writeFixupPageTable();
    //virtual void encodeFixupPageTable(){ofpage_table = ifpage_table; ifpage_table = NULL;}
    //virtual void decodeFixupPageTable(){encodeFixupPageTable();}

    virtual void readPageMap();
    virtual void writePageMap();
    virtual void encodePageMap(){opm_entries = ipm_entries; ipm_entries = NULL;}
    virtual void decodePageMap(){encodePageMap();}

    virtual void readResidentNames();
    virtual void writeResidentNames();
    virtual void encodeResidentNames(){ores_names = ires_names; ires_names = NULL;}
    virtual void decodeResidentNames(){encodeResidentNames();}

    virtual void readNonResidentNames();
    virtual void writeNonResidentNames();
    virtual void encodeNonResidentNames(){ononres_names = inonres_names; inonres_names = NULL;}
    virtual void decodeNonResidentNames(){encodeNonResidentNames();}

    virtual void readEntryTable();
    virtual void writeEntryTable();
    //virtual void encodeEntryTable(){oentries = ientries; ientries = NULL;}
    //virtual void decodeEntryTable(){encodeEntryTable();}

    virtual void readFixups();
    virtual void writeFixups();
    //virtual void encodeFixups(){ofixups = ifixups; ifixups = NULL;}
    //virtual void decodeFixups(){encodeFixups();}

    virtual void readImage();
    virtual void writeImage();
    //virtual void encodeImage(){oimage = iimage; iimage = NULL;}
    //virtual void decodeImage(){encodeImage();}

    void countFixups(unsigned *) const;
    unsigned getImageSize() const;

    InputFile *fif;
    OutputFile *fof;
    long le_offset;
    long exe_offset;

    le_header_t ih;
    le_header_t oh;

    le_object_table_entry_t *iobject_table;
    le_object_table_entry_t *oobject_table;
    unsigned *ifpage_table;
    unsigned *ofpage_table;
    le_pagemap_entry_t *ipm_entries;
    le_pagemap_entry_t *opm_entries;
    upx_byte *ires_names;
    upx_byte *ores_names;
    upx_byte *ifixups;
    upx_byte *ofixups;
    upx_byte *inonres_names;
    upx_byte *ononres_names;
    MemBuffer iimage;
    MemBuffer oimage;
    upx_byte *ientries;
    upx_byte *oentries;

    unsigned soobject_table;
    unsigned sofpage_table;
    unsigned sopm_entries;
    unsigned sores_names;
    unsigned sofixups;
    unsigned sononres_names;
    unsigned soimage;
    unsigned soentries;

private:
    // disable copy and assignment
    LeFile(const LeFile &); // {}
    LeFile& operator= (const LeFile &); // { return *this; }
};


#endif /* already included */


/*
vi:ts=4:et
*/

