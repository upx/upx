#! /usr/bin/env python
## vim:set ts=4 sw=4 et: -*- coding: utf-8 -*-
#
#  xstrip.py -- truncate ELF objects created by multiarch-objcopy-2.17
#
#  This file is part of the UPX executable compressor.
#
#  Copyright (C) 1996-2008 Markus Franz Xaver Johannes Oberhumer
#  All Rights Reserved.
#
#  UPX and the UCL library are free software; you can redistribute them
#  and/or modify them under the terms of the GNU General Public License as
#  published by the Free Software Foundation; either version 2 of
#  the License, or (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; see the file COPYING.
#  If not, write to the Free Software Foundation, Inc.,
#  59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
#
#  Markus F.X.J. Oberhumer              Laszlo Molnar
#  <markus@oberhumer.com>               <ml1050@users.sourceforge.net>
#


import getopt, os, re, string, struct, sys


class opts:
    dry_run = 0
    verbose = 0
    bindump = None
    with_dump = None


# /***********************************************************************
# //
# ************************************************************************/

def strip_with_dump(dump_fn, eh, idata):
    new_len = 0
    lines = open(dump_fn, "rb").readlines()
    for l in lines:
        l = re.sub(r"\s+", " ", l.strip())
        f = l.split(" ")
        if len(f) >= 8:
            if f[7].startswith("CONTENTS"):
                sh_offset = int("0x" + f[5], 16)
                sh_size   = int("0x" + f[2], 16)
                if sh_offset + sh_size > new_len:
                    new_len = sh_offset + sh_size
                    ##print sh_offset, sh_size, f
    if new_len > len(eh):
        ##print dump_fn, new_len
        return eh, idata[:new_len-len(eh)]
    return eh, idata


# /***********************************************************************
# // FIXME - this is only a first stub version
# ************************************************************************/

def create_bindump(bindump_fn, dump_fn):
    data = ""
    lines = open(dump_fn, "rb").readlines()
    lines = map(lambda l: re.sub(r"\s+", " ", l.strip()).strip(), lines)
    lines = filter(None, lines)
    d = "\n".join(lines)
    psections = d.find("Sections:\n")
    psymbols  = d.find("SYMBOL TABLE:\n")
    prelocs   = d.find("RELOCATION RECORDS FOR ");
    assert 0 <= psections < psymbols < prelocs
    # preprocessSections
    sections = []
    section_names = {}
    for l in d[psections:psymbols].split("\n")[2:]:
        if not l: continue
        f = l.split(" ")
        assert len(f) >= 8, (l, f)
        assert f[6].startswith("2**"), (l, f)
        assert f[7].startswith("CONTENTS"), (l, f)
        assert int(f[0], 10) == len(sections)
        e = f[1], int(f[2], 16), int(f[5], 16), int(f[6][3:], 10), len(sections)
        sections.append(e)
        assert not section_names.has_key(e[0]), e
        assert not e[0].endswith(":"), ("bad section name", e)
        section_names[e[0]] = e
    ##print sections
    # preprocessSymbols
    symbols = []
    section = None
    for l in d[psymbols:prelocs].split("\n")[1:]:
        if not l: continue
        f = l.split(" ")
        if len(f) == 6:
            assert f[1] in "gl", (l, f)
            assert f[2] in "dFO", (l, f)
            section = section_names[f[3]]
        elif len(f) == 5 and f[2] == "*ABS*":
            pass
        elif len(f) == 5:
            assert f[1] in "gl", (l, f)
            section = section_names[f[2]]
        elif len(f) == 4:
            assert f[1] in ["*UND*"], (l, f)
            section = None
        else:
            assert 0, (l, f)
        pass
    # preprocessRelocations
    relocs = []
    section = None
    for l in d[prelocs:].split("\n")[1:]:
        if not l: continue
        m = re.search(r"^RELOCATION RECORDS FOR \[(.+)\]", l)
        if m:
            section = section_names[m.group(1)]
            continue
        f = l.split(" ")
        if f[0] == "OFFSET": continue
        assert len(f) == 3, (l, f)
        pass
    fp = open(bindump_fn, "wb")
    fp.write(data)
    fp.write(struct.pack("<I", len(data) + 4))
    fp.close()


# /***********************************************************************
# //
# ************************************************************************/

def do_file(fn):
    odata = None
    if opts.dry_run:
        fp = open(fn, "rb")
    else:
        fp = open(fn, "r+b")
    fp.seek(0, 0)
    idata = fp.read()
    fp.seek(0, 0)
    if idata[:4] != "\x7f\x45\x4c\x46":
        raise Exception, "%s is not %s" % (fn, "ELF")
    if idata[4:7] == "\x01\x01\x01":
        # ELF32 LE
        eh, idata = idata[:52], idata[52:]
        e_shnum, e_shstrndx = struct.unpack("<HH", eh[48:52])
        assert e_shstrndx + 3 == e_shnum
        ##eh = eh[:48] + struct.pack("<HH", e_shnum - 3, e_shstrndx)
    elif idata[4:7] == "\x01\x02\x01":
        # ELF32 BE
        eh, idata = idata[:52], idata[52:]
        e_shnum, e_shstrndx = struct.unpack(">HH", eh[48:52])
        assert e_shstrndx + 3 == e_shnum
    elif idata[4:7] == "\x02\x01\x01":
        # ELF64 LE
        eh, idata = idata[:64], idata[64:]
        e_shnum, e_shstrndx = struct.unpack("<HH", eh[60:64])
        assert e_shstrndx + 3 == e_shnum
    elif idata[4:7] == "\x02\x02\x01":
        # ELF64 BE
        eh, idata = idata[:64], idata[64:]
        e_shnum, e_shstrndx = struct.unpack(">HH", eh[60:64])
        assert e_shstrndx + 3 == e_shnum
    else:
        raise Exception, "%s is not %s" % (fn, "ELF")

    odata = None
    pos = idata.find("\0.symtab\0.strtab\0.shstrtab\0")
    if opts.with_dump:
        eh, odata = strip_with_dump(opts.with_dump, eh, idata)
        assert len(odata) == pos, "unexpected strip_with_dump"
    else:
        if pos >= 0:
            odata = idata[:pos]

    if eh and odata and not opts.dry_run:
        fp.write(eh)
        fp.write(odata)
        fp.truncate()
    fp.close()


def main(argv):
    shortopts, longopts = "qv", [
        "create-bindump=", "dry-run", "quiet", "verbose", "with-dump="
    ]
    xopts, args = getopt.gnu_getopt(argv[1:], shortopts, longopts)
    for opt, optarg in xopts:
        if 0: pass
        elif opt in ["-q", "--quiet"]: opts.verbose = opts.verbose - 1
        elif opt in ["-v", "--verbose"]: opts.verbose = opts.verbose + 1
        elif opt in ["--dry-run"]: opts.dry_run = opts.dry_run + 1
        elif opt in ["--create-bindump"]: opts.bindump = optarg
        elif opt in ["--with-dump"]: opts.with_dump = optarg
        else: assert 0, ("getopt problem:", opt, optarg, xopts, args)
    if not args:
        raise Exception, "error: no arguments given"
    if opts.with_dump or opts.bindump:
        assert len(args) == 1, "need exactly one file"
    # process arguments
    for arg in args:
        do_file(arg)
        if opts.bindump:
            assert opts.with_dump, "need --with-dump"
            create_bindump(opts.bindump, opts.with_dump)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))

