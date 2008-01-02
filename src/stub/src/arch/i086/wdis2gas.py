#! /usr/bin/env python
## vim:set ts=4 sw=4 et: -*- coding: utf-8 -*-
#
#  wdis2gas.py --
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


import getopt, os, re, string, sys


class opts:
    arch = "i086"
    label_prefix = ".L"
    verbose = 0


# /***********************************************************************
# // main
# ************************************************************************/

def main(argv):
    shortopts, longopts = "qv", [
        "arch", "label-prefix=", "quiet", "verbose"
    ]
    xopts, args = getopt.gnu_getopt(argv[1:], shortopts, longopts)
    for opt, optarg in xopts:
        if 0: pass
        elif opt in ["-q", "--quiet"]: opts.verbose = opts.verbose - 1
        elif opt in ["-v", "--verbose"]: opts.verbose = opts.verbose + 1
        elif opt in ["--arch"]: opts.arch = optarg
        elif opt in ["--label-prefix"]: opts.label_prefix = optarg
        else: assert 0, ("getopt problem:", opt, optarg, xopts, args)

    #
    assert len(args) == 2
    ifile = args[0]
    ofile = args[1]
    # read ifile
    lines = open(ifile, "rb").readlines()
    lines = map(string.rstrip, lines)
    #
    section = None
    func = None
    olines = []
    for i in range(len(lines)):
        l = lines[i]
        if not l: continue
        m = re.search(r"^No disassembly errors", l)
        if m: continue
        m = re.search(r"^Module:", l)
        if m: continue
        m = re.search(r"^GROUP:", l)
        if m: continue
        m = re.search(r"^(BSS|Routine) Size:", l)
        if m: continue
        m = re.search(r"^Segment:\s+(.+)\s+([0-9a-fA-F]+)\s+bytes$", l)
        if m:
            s = re.split(r"\s+", m.group(1))
            assert len(s) == 3, (i, l, s, m.groups())
            section = s
            func = None
            continue
        m = re.search(r"^Comdat:\s+(.+)\s+SEGMENT NONE '(\w+)'\s+([0-9a-fA-F]+)\s+bytes$", l)
        if m:
            section = [m.group(2)]
            assert section[0].endswith("_TEXT"), (i, l, section)
            func = re.sub(r"^[_@]+|[_@]+$", "", m.group(1))
            olines.append(".section .text." + func)
            continue
        assert section, (i, l)
        m = re.search(r"^0000\s+(\w+):$", l)
        if m:
            assert section[0].endswith("_TEXT"), (i, l, section)
            func = re.sub(r"^[_@]+|[_@]+$", "", m.group(1))
            olines.append(".section .text." + func)
            continue
        assert func, (i, l, section)
        m = re.search(r"^[0-9a-fA-F]{4}\s+L\$(\d+):$", l)
        if m:
            olines.append(opts.label_prefix + m.group(1) + ":")
            continue
        m = re.search(r"^[0-9a-fA-F]{4}    (([0-9a-fA-F]{2} )+)\s+(.+)$", l)
        assert m, (i, l)
        if m.group(3).startswith("call"):
            s = re.split(r"\s+", m.group(3))
            assert len(s) == 2, (i, l, s, m.groups())
            f = re.sub(r"^[@]+|[@]+$", "", s[1])
            olines.append("  call " + f)
        elif 1:
            s = m.group(3).strip()
            s = re.sub(r"L\$(\d+)", opts.label_prefix + r"\g<1>", s)
            olines.append("  " + s)
        else:
            s = re.split(r"\s+", m.group(1).strip())
            assert 1 <= len(s) <= 5, (i, l, s, m.groups())
            s = ["0x" + x for x in s]
            olines.append("  .byte " + ",".join(s))


    # write ofile
    ofp = open(ofile, "wb")
    ofp.write(".code16\n")
    ofp.write(".intel_syntax noprefix\n")
    if opts.arch in ["i086", "8086", "i8086"]:
        ofp.write(".arch i8086, jumps\n")
    elif opts.arch in ["i286"]:
        ofp.write(".arch i286, jumps\n")
    else:
        assert 0, ("invalid arch", opts.arch)
    if 0:
        for sym in ["__AHSHIFT", "__AHINCR", "__LMUL", "__aNahdiff"]:
            ofp.write(".extern %s\n" % (sym))
            ofp.write(".type %s,@function\n" % (sym))
            ofp.write(".size %s,2\n" % (sym))
    for l in olines:
        ofp.write(l.rstrip() + "\n")
    ofp.close()
    ##print olines


if __name__ == "__main__":
    sys.exit(main(sys.argv))

