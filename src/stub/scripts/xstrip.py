#! /usr/bin/env python
## vim:set ts=4 sw=4 et: -*- coding: utf-8 -*-
#
#  xstrip.py -- truncate ELF objects created by multiarch-objcopy-2.17
#
#  This file is part of the UPX executable compressor.
#
#  Copyright (C) 1996-2006 Markus Franz Xaver Johannes Oberhumer
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
#  <mfx@users.sourceforge.net>          <ml1050@users.sourceforge.net>
#


import getopt, os, re, string, struct, sys


class opts:
    dry_run = 0
    dump = None
    verbose = 0


# /***********************************************************************
# //
# ************************************************************************/

def strip_with_dump(eh, idata):
    new_len = 0
    lines = open(opts.dump, "rb").readlines()
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
        ##print opts.dump, new_len
        return eh, idata[:new_len-len(eh)]
    return eh, idata


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
    if opts.dump:
        eh, odata = strip_with_dump(eh, idata)
    else:
        pos = idata.find("\0.symtab\0.strtab\0.shstrtab\0")
        if pos >= 0:
            odata = idata[:pos]

    if eh and odata and not opts.dry_run:
        fp.write(eh)
        fp.write(odata)
        fp.truncate()
    fp.close()


def main(argv):
    shortopts, longopts = "qv", [
        "dry-run", "dump=", "quiet", "verbose"
    ]
    xopts, args = getopt.gnu_getopt(argv[1:], shortopts, longopts)
    for opt, optarg in xopts:
        if 0: pass
        elif opt in ["-q", "--quiet"]: opts.verbose = opts.verbose - 1
        elif opt in ["-v", "--verbose"]: opts.verbose = opts.verbose + 1
        elif opt in ["--dry-run"]: opts.dry_run = opts.dry_run + 1
        elif opt in ["--dump"]: opts.dump = optarg
        else: assert 0, ("getopt problem:", opt, optarg, xopts, args)
    # process arguments
    if not args:
        raise Exception, "error: no arguments given"
    for arg in args:
        do_file(arg)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))

