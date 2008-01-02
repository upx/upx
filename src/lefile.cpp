/* lefile.cpp --

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


#include "conf.h"
#include "file.h"
#include "mem.h"
#include "lefile.h"


LeFile::LeFile(InputFile *f) :
    fif(f), fof(NULL),
    le_offset(0), exe_offset(0)
{
    COMPILE_TIME_ASSERT(sizeof(le_header_t) == 196);
    COMPILE_TIME_ASSERT(sizeof(le_object_table_entry_t) == 24);
    COMPILE_TIME_ASSERT(sizeof(le_pagemap_entry_t) == 4);
    memset(&ih,0,sizeof ih);
    memset(&oh,0,sizeof oh);
    iobject_table = oobject_table = NULL;
    ifpage_table = ofpage_table = NULL;
    ipm_entries = opm_entries = NULL;
    ires_names = ores_names = NULL;
    ifixups = ofixups = NULL;
    inonres_names = ononres_names = NULL;
    ientries = oentries = NULL;
}


LeFile::~LeFile()
{
    delete [] iobject_table;
    delete [] oobject_table;
    delete [] ifpage_table;
    delete [] ofpage_table;
    delete [] ipm_entries;
    delete [] opm_entries;
    delete [] ires_names;
    delete [] ores_names;
    delete [] ifixups;
    delete [] ofixups;
    delete [] inonres_names;
    delete [] ononres_names;
    delete [] ientries;
    delete [] oentries;
}


#define objects ih.object_table_entries
#define pages   ih.memory_pages
#define mps     ih.memory_page_size


void LeFile::readObjectTable()
{
    iobject_table = new le_object_table_entry_t[soobject_table = objects];
    fif->seek(le_offset + ih.object_table_offset,SEEK_SET);
    fif->readx(iobject_table,sizeof(*iobject_table)*objects);
}


void LeFile::writeObjectTable()
{
    if (fof && oobject_table)
        fof->write(oobject_table,sizeof(*iobject_table)*soobject_table);
}


void LeFile::readPageMap()
{
    ipm_entries = new le_pagemap_entry_t[sopm_entries = pages];
    fif->seek(le_offset + ih.object_pagemap_offset,SEEK_SET);
    fif->readx(ipm_entries,sizeof(*ipm_entries)*pages);

    for (unsigned ic = 0; ic < pages; ic++)
        if ((ipm_entries[ic].type & 0xC0) != 0 && (ipm_entries[ic].type & 0xC0) != 0xC0)
            throwCantPack("unexpected value in page map table");
}


void LeFile::writePageMap()
{
    if (fof && opm_entries)
        fof->write(opm_entries,sizeof(*ipm_entries)*sopm_entries);
}


void LeFile::readResidentNames()
{
    sores_names = ih.entry_table_offset - ih.resident_names_offset;
    ires_names = new upx_byte[sores_names];
    fif->seek(le_offset+ih.resident_names_offset,SEEK_SET);
    fif->readx(ires_names,sores_names);
}


void LeFile::writeResidentNames()
{
    if (fof && ores_names)
        fof->write(ores_names,sores_names);
}


void LeFile::readEntryTable()
{
    soentries = ih.fixup_page_table_offset - ih.entry_table_offset;
    fif->seek(le_offset + ih.entry_table_offset,SEEK_SET);
    ientries = new upx_byte[soentries];
    fif->readx(ientries,soentries);
}


void LeFile::writeEntryTable()
{
    if (fof && oentries)
        fof->write(oentries,soentries);
}


void LeFile::readFixupPageTable()
{
    ifpage_table = new unsigned[sofpage_table = 1+pages];
    fif->seek(le_offset + ih.fixup_page_table_offset,SEEK_SET);
    fif->readx(ifpage_table,4*sofpage_table);
}


void LeFile::writeFixupPageTable()
{
    if (fof && ofpage_table)
        fof->write(ofpage_table,4*sofpage_table);
}


void LeFile::readFixups()
{
    sofixups = get_le32(ifpage_table+pages)-get_le32(ifpage_table);
    ifixups = new upx_byte[sofixups];
    fif->seek(le_offset + ih.fixup_record_table_offset,SEEK_SET);
    fif->readx(ifixups,sofixups);
}


void LeFile::writeFixups()
{
    if (fof && ofixups)
        fof->write(ofixups,sofixups);
}


unsigned LeFile::getImageSize() const
{
    unsigned n = 0;
    if (ih.memory_pages > 0)
    {
        n = (ih.memory_pages - 1) * ih.memory_page_size;
        n += ih.bytes_on_last_page;
    }
    return n;
}


void LeFile::readImage()
{
    soimage = pages*mps;
    iimage.alloc(soimage);
    memset(iimage,0,soimage);

    unsigned ic,jc;
    for (ic = jc = 0; ic < pages; ic++)
    {
        if ((ipm_entries[ic].type & 0xC0) == 0)
        {
            fif->seek(ih.data_pages_offset + exe_offset +
                (ipm_entries[ic].m*0x100 + ipm_entries[ic].l-1) * mps,SEEK_SET);
            fif->readx(iimage+jc,ic != pages-1 ? mps : ih.bytes_on_last_page);
        }
        jc += mps;
    }
}


void LeFile::writeImage()
{
    if (fof && oimage != NULL)
        fof->write(oimage, soimage);
}


void LeFile::readNonResidentNames()
{
    if (ih.non_resident_name_table_length)
    {
        inonres_names = new upx_byte[sononres_names = ih.non_resident_name_table_length];
        fif->seek(exe_offset+ih.non_resident_name_table_offset,SEEK_SET);
        fif->readx(inonres_names,sononres_names);
    }
}


void LeFile::writeNonResidentNames()
{
    if (fof && ononres_names)
        fof->write(ononres_names,sononres_names);
}


bool LeFile::readFileHeader()
{
#define H(x) get_le16(header+2*(x))
    upx_byte header[0x40];
    le_offset = exe_offset = 0;
    int ic;

    for (ic = 0; ic < 20; ic++)
    {
        fif->seek(le_offset,SEEK_SET);
        fif->readx(header,sizeof(header));

        if (memcmp(header,"MZ",2) == 0) // normal dos exe
        {
            exe_offset = le_offset;
            if (H(0x18/2) >= 0x40
                && memcmp(header+0x19,"TIPPACH",7)) // new format exe
                le_offset += H(0x3c/2)+H(0x3e/2)*65536;
            else
            {
                le_offset += H(2)*512+H(1);
                if (H(1))
                    le_offset -= 512;
                else if (H(2) == 0)
                    return false;
            }
        }
        else if (memcmp(header,"BW",2) == 0) // used in dos4gw.exe
            le_offset += H(2)*512+H(1);
        else if (memcmp(header,"LE",2) == 0)
            break;
        else if (memcmp(header,"PMW1",4) == 0)
            throwCantPack("already packed with PMWLITE");
        else
            return false;
    }
    if (ic == 20)
        return false;
    fif->seek(le_offset,SEEK_SET);
    fif->readx(&ih,sizeof(ih));
    return true;
#undef H
}


void LeFile::writeFile(OutputFile *f, bool le)
{
    fof = f;
    memcpy (&oh,&ih,(char*)&oh.memory_pages-(char*)&oh); // copy some members of the orig. header
    oh.memory_page_size = mps;
    oh.object_table_offset = sizeof(oh);
    oh.object_table_entries = soobject_table;
    oh.object_pagemap_offset = oh.object_table_offset + soobject_table*sizeof(*iobject_table);
    oh.resident_names_offset = oh.object_pagemap_offset + sopm_entries*sizeof(*ipm_entries);
    oh.entry_table_offset = oh.resident_names_offset + sores_names;
    oh.fixup_page_table_offset = oh.entry_table_offset + soentries;
    oh.fixup_record_table_offset = oh.fixup_page_table_offset + sofpage_table*4;
    oh.imported_modules_name_table_offset = oh.fixup_record_table_offset + sofixups - FIXUP_EXTRA;
    oh.imported_procedures_name_table_offset = oh.imported_modules_name_table_offset;
    oh.data_pages_offset = oh.fixup_record_table_offset + sofixups + (le ? 0 : le_offset-exe_offset);
    if (ih.non_resident_name_table_length)
    {
        oh.non_resident_name_table_offset = oh.data_pages_offset + soimage;
        oh.non_resident_name_table_length = sononres_names;
    }
    oh.fixup_size = sofixups + 4*sofpage_table;
    oh.loader_size = oh.fixup_size + oh.fixup_page_table_offset - sizeof(oh);

    fof->write(&oh,sizeof(oh));
    writeObjectTable();
    writePageMap();
    writeResidentNames();
    writeEntryTable();
    writeFixupPageTable();
    writeFixups();
    writeImage();
    writeNonResidentNames();
}


void LeFile::countFixups(unsigned *counts) const
{
    const unsigned o = objects;
    memset(counts,0,sizeof(unsigned)*(o+2));
    // counts[0..objects-1] - # of 32-bit offset relocations in for that objects
    // counts[objects]      - # of selector fixups
    // counts[objects+1]    - # of self-relative fixups

    const upx_byte *fix = ifixups;
    const unsigned sfixups = get_le32(ifpage_table+pages);
    unsigned ll;

    while ((unsigned)(fix - ifixups) < sfixups)
    {
        if ((fix[1] & ~0x10) != 0)
            throwCantPack("unsupported fixup record");
        switch (*fix)
        {
            case 2:       // selector fixup
                counts[o] += 9;
                fix += 5;
                break;
            case 0x12:    // alias selector
                throwCantPack("16-bit selector alias fixup not yet supported");
            case 5:       // 16-bit offset
                fix += (fix[1] & 0x10) ? 9 : 7;
                break;
            case 6:       // 16:32 pointer
                counts[o] += 9;
            case 7:       // 32-bit offset
                counts[fix[4]-1] += 4;
                fix += (fix[1] & 0x10) ? 9 : 7;
                break;
            case 0x27:    // 32-bit offset list
                ll = fix[2];
                counts[fix[3]-1] += ll*4;
                fix += (fix[1] & 0x10) ? 6 : 4;
                fix += ll*2;
                break;
            case 8:       // 32-bit self relative fixup
                counts[o+1] += 4;
                fix += (fix[1] & 0x10) ? 9 : 7;
                break;
            default:
                throwCantPack("unsupported fixup record");
        }
    }
    counts[o]++; // extra space for 'ret'
    counts[o+1] += 4; // extra space for 0xFFFFFFFF
}


/*
vi:ts=4:et
*/

