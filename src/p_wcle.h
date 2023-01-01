/* p_wcle.h --

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
#ifndef UPX_P_WCLE_H__
#define UPX_P_WCLE_H__ 1

/*************************************************************************
// watcom/le
**************************************************************************/

class PackWcle final : public Packer, public LeFile {
    typedef Packer super;

public:
    PackWcle(InputFile *f) : super(f), LeFile(f) { bele = &N_BELE_RTP::le_policy; }
    virtual int getVersion() const override { return 13; }
    virtual int getFormat() const override { return UPX_F_WATCOM_LE; }
    virtual const char *getName() const override { return "watcom/le"; }
    virtual const char *getFullName(const options_t *) const override {
        return "i386-dos32.watcom.le";
    }
    virtual const int *getCompressionMethods(int method, int level) const override;
    virtual const int *getFilters() const override;

    virtual void pack(OutputFile *fo) override;
    virtual void unpack(OutputFile *fo) override;

    virtual bool canPack() override;
    virtual int canUnpack() override;

protected:
    virtual void handleStub(OutputFile *fo);

    virtual void buildLoader(const Filter *ft) override;
    virtual Linker *newLinker() const override;

    virtual void readObjectTable() override;
    virtual void encodeObjectTable();
    virtual void decodeObjectTable();

    virtual void encodeFixupPageTable();
    virtual void decodeFixupPageTable();

    virtual void encodePageMap() override;

    virtual void encodeEntryTable();
    virtual void decodeEntryTable();

    virtual void preprocessFixups();
    virtual void encodeFixups();
    virtual void decodeFixups();

    virtual void encodeImage(Filter *ft);
    virtual void decodeImage();

    static void virt2rela(const le_object_table_entry_t *, unsigned *objn, unsigned *addr);

    // temporary copy of the object descriptors
    MemBuffer iobject_desc;

    int big_relocs;
    bool has_extra_code;
    unsigned neweip;
};

#endif /* already included */

/* vim:set ts=4 sw=4 et: */
