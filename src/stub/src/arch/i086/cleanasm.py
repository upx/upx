#! /usr/bin/env python
## vim:set ts=4 sw=4 et: -*- coding: utf-8 -*-
#
#  cleanasm.py --
#
#  This file is part of the UPX executable compressor.
#
#  Copyright (C) 1996-2007 Markus Franz Xaver Johannes Oberhumer
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


import getopt, os, re, string, sys


class opts:
    label_prefix = ".L"
    verbose = 0
    # optimizer flags
    auto_inline = 1
    call_rewrite = 1


inline_map = {
##    "__PIA":        "WCC_PIA",
    "__PTS":        "WCC_PTS",
##    "__U4M_V01":    "WCC_U4M_V01",
    "__PIA_V02":    "WCC_PIA_V02",
    "__PIA_V01":    "WCC_PIA_V01",
}


# /***********************************************************************
# // main
# ************************************************************************/

def main(argv):
    shortopts, longopts = "qv", [
        "label-prefix=", "quiet", "verbose"
    ]
    xopts, args = getopt.gnu_getopt(argv[1:], shortopts, longopts)
    for opt, optarg in xopts:
        if 0: pass
        elif opt in ["-q", "--quiet"]: opts.verbose = opts.verbose - 1
        elif opt in ["-v", "--verbose"]: opts.verbose = opts.verbose + 1
        elif opt in ["--label-prefix"]: opts.label_prefix = optarg
        else: assert 0, ("getopt problem:", opt, optarg, xopts, args)

    #
    assert len(args) == 2
    ifile = args[0]
    ofile = args[1]
    # read ifile
    lines = open(ifile, "rb").readlines()
    lines = filter(None, map(string.rstrip, lines))
    #
    #
    labels = {}
    def parse_label(inst, args):
        m = re.search("^([0-9a-z]+)\s+<", args)
        if m:
            # local label
            k, v = m.group(1).strip(), [0, 0, None, 0]
        m = re.search(r"^(.*?)\b2\s+(__\w+)$", args)
        if m:
            # external 2-byte label
            k, v = m.group(2).strip(), [1, 2, None, 0]
        v[2] = k                # new name
        if labels.has_key(k):
            assert labels[k][:2] == v[:2]
        return k, v
    def add_label(k, v):
        if labels.has_key(k):
            assert labels[k][:2] == v[:2]
        else:
            labels[k] = v
        labels[k][3] += 1       # usage counter
        return k

    olines = []
    def omatch(pos, m):
        i = 0
        dpos = []
        while i < len(m):
            if pos < 0 or pos >= len(olines):
                return False
            dpos.append(pos)
            o = olines[pos][1:3]
            assert len(m[i]) == 2, (i, m)
            if o[0].lower() != m[i][0].lower():
                return False
            if o[1].lower() != m[i][1].lower():
                return []
            pos += 1
            i += 1
        return dpos
    def orewrite(i, k, v, dpos):
        for pos in dpos:
            olines[pos][1] = "*DEL*"
        v[2] = k
        olines[i][2] = None
        olines[i][3] = add_label(k, v)


    #
    # pass 1
    func = None
    for i in range(len(lines)):
        l = lines[i]
        m = re.search(r"^0000000000000000\s*<(\w+)>:", l)
        if m:
            func = re.sub(r"^_+|_+$", "", m.group(1))
        if not func in ["LzmaDecode"]:
            continue
        m = re.search(r"^(\s*[0-9a-z]+):\s+(\w+)(.*)", l)
        if not m:
            continue
        label = m.group(1).strip()
        inst = m.group(2).strip()
        args = ""
        if m.group(3): args = m.group(3).strip()
        #
        if inst in ["movl",] and re.search(r"\b[de]s\b", args):
            # fix bug in objdump
            inst = "movw"
        m = re.search(r"^(.+?)\b0x0\s+(\w+):\s+[12]\s+(__\w+)$", args)
        if m:
            # 1 or 2 byte reloc
            args = m.group(1) + m.group(3)
        olines.append([label, inst, args, None])
    #
    # pass 2
    for i in range(len(olines)):
        label, inst, args, args_label = olines[i]
        #
        if inst == "*DEL*":
            continue
        #
        if opts.call_rewrite and inst in ["call"]:
            k, v = parse_label(inst, args)
            if v[:2] == [1, 2]:     # external 2-byte
                if k == "__PIA":
                    inst1 = [
                        ["mov",  "bx,WORD PTR [bp-94]"],
                        ["or",   "bx,ax"],
                        ["mov",  "WORD PTR [bp-8],bx"],
                        ["mov",  "WORD PTR [bp-4],dx"],
                        ["mov",  "ax,WORD PTR [bp-12]"],
                        ["movw", "dx,ds"],
                        ["mov",  "bx,0x1"],
                        ["xor",  "cx,cx"],
                    ]
                    inst2 = [
                        ["mov",  "WORD PTR [bp-12],ax"],
                        ["movw", "ds,dx"],
                    ]
                    dpos1 = omatch(i - 8, inst1[-8:])
                    dpos2 = omatch(i + 1, inst2)
                    if dpos1 and dpos2:
                        orewrite(i, "__PIA_V04", v, dpos1 + dpos2)
                        continue
                    dpos1 = omatch(i - 4, inst1[-4:])
                    dpos2 = omatch(i + 1, inst2)
                    if dpos1 and dpos2:
                        orewrite(i, "__PIA_V03", v, dpos1 + dpos2)
                        continue
                    dpos = omatch(i - 3, inst1[-3:])
                    if dpos:
                        orewrite(i, "__PIA_V02", v, dpos)
                        continue
                    dpos = omatch(i - 2, inst1[-2:])
                    if dpos:
                        orewrite(i, "__PIA_V01", v, dpos)
                        continue
                if k == "__PTC":
                    inst1 = [
                        ["mov",  "ax,WORD PTR [bp-12]"],
                        ["movw", "dx,ds"],
                        ["mov",  "bx,WORD PTR [bp-26]"],
                        ["mov",  "cx,WORD PTR [bp-24]"],
                    ]
                    dpos = omatch(i - 4, inst1[-4:])
                    if dpos:
                        orewrite(i, "__PTC_V01", v, dpos)
                        continue
                if k == "__U4M":
                    inst1 = [
                        ["mov",  "bx,WORD PTR es:[bx]"],
                        ["mov",  "ax,WORD PTR [bp-102]"],
                        ["mov",  "dx,WORD PTR [bp-100]"],
                        ["xor",  "cx,cx"],
                    ]
                    inst2 = [
                        ["mov",  "WORD PTR [bp-10],ax"],
                        ["mov",  "WORD PTR [bp-6],dx"],
                    ]
                    dpos1 = omatch(i - 4, inst1[-4:])
                    dpos2 = omatch(i + 1, inst2)
                    if dpos1 and dpos2:
                        orewrite(i, "__U4M_V02", v, dpos1 + dpos2)
                        continue
                    dpos = omatch(i - 1, inst1[-1:])
                    if dpos:
                        orewrite(i, "__U4M_V01", v, dpos)
                        continue
        #
        if inst in [
            "call", "ja", "jae", "jb", "jbe", "jcxz", "je",
            "jg", "jge", "jl", "jle", "jmp", "jne", "loop",
        ]:
            k, v = parse_label(inst, args)
            olines[i][2] = None
            olines[i][3] = add_label(k, v)

    #
    # rewrite local labels
    digits, i = 1, len(labels)
    while i >= 10:
        digits += 1
        i /= 10
    format = "%s0%dd" % ("%", digits)
    counter = 0
    for i in range(len(olines)):
        label, inst, args, args_label = olines[i]
        # rewrite local labels
        v = labels.get(label)
        if v is not None:
            assert v[:3] == [0, 0, label], (label, v)
            v[2] = opts.label_prefix + format % counter
            counter += 1
        # handle inlining
        if opts.auto_inline and inst == "call":
            v = labels[args_label]
            if v[:2] == [1, 2]:     # external 2-byte
                if v[3] == 1:           # only one call
                    x = inline_map.get(v[2])
                    ##print "inline", v, x
                    if x:
                        olines[i][1] = x
                        olines[i][2] = "/* inlined */"
                        olines[i][2] = ""
                        olines[i][3] = None

    # write ofile
    ofp = open(ofile, "wb")
    for label, inst, args, args_label in olines:
        if labels.has_key(label):
            if opts.verbose:
                ofp.write("%s: /* %d */\n" % (labels[label][2], labels[label][3]))
            else:
                ofp.write("%s:\n" % (labels[label][2]))
        if inst == "*DEL*":
            continue
        if args_label:
            if opts.verbose:
                args = "%s /* %d */" % (labels[args_label][2], labels[args_label][3])
            else:
                args = labels[args_label][2]
        l = "%8s%-7s %s" % ("", inst, args)
        ofp.write(l.rstrip() + "\n")
    ofp.close()
    ##print olines


if __name__ == "__main__":
    sys.exit(main(sys.argv))

