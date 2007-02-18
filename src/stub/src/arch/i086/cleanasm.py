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
    loop_rewrite = 1


inline_map = {
    "__aNNalshl":    ["M_aNNalshl", 1],
    "__aNahdiff":    ["M_aNahdiff", 1],
    "__PIA":         ["M_PIA", 999],
    "__PTS":         ["M_PTS", 999],
    "__PTC":         ["M_PTC", 999],
    "__U4M":         ["M_U4M", 999],
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
    assert opts.label_prefix
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
        k = v = None
        m = re.search(r"^(.*?)\b(2|R_386_PC16)\s+(__\w+)$", args)
        if m and k is None:
            # external 2-byte label
            k, v = m.group(3).strip(), [1, 2, None, 0]
        m = re.search("^0x([0-9a-z]+)$", args)
        if m and k is None:
            # local label
            k, v = m.group(1).strip(), [0, 0, None, 0]
        m = re.search("^([0-9a-z]+)\s+<", args)
        if m and k is None:
            # local label
            k, v = m.group(1).strip(), [0, 0, None, 0]
        assert k and v, (inst, args)
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
        def match(a, b):
            if b is None:
                return False
            if "^" in a or "*" in a:
                # regexp
                return re.search(a, b.lower())
            else:
                return a.lower() == b.lower()
        i = 0
        dpos = []
        while i < len(m):
            if pos < 0 or pos >= len(olines):
                return []
            o = olines[pos][1:3]
            assert len(m[i]) == 2, (i, m)
            if o[1] == "*DEL*":
                pos += 1
                continue
            m0 = match(m[i][0], o[0])
            m1 = match(m[i][1], o[1])
            if not m0 or not m1:
                return []
            dpos.append([pos, m0, m1])
            pos += 1
            i += 1
        return dpos
    def orewrite_inst(i, inst, args, dpos):
        for pos, m0, m1 in dpos:
            olines[pos][1] = "*DEL*"
        olines[i][1] = inst
        olines[i][2] = args
        olines[i][3] = None
    def orewrite_call(i, k, v, dpos):
        for pos, m0, m1 in dpos:
            olines[pos][1] = "*DEL*"
        v[2] = k
        olines[i][2] = None
        olines[i][3] = add_label(k, v)

    #
    # pass 1
    func = None
    for i in range(len(lines)):
        l = lines[i]
        m = re.search(r"^0{8,16}\s*<(\.text\.)?(\w+)>:", l)
        if m:
            func = re.sub(r"^_+|_+$", "", m.group(2))
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
        m = re.search(r"^(.+?)\b0x0\s+(\w+):\s+(1|2|R_386_16|R_386_PC16)\s+(__\w+)$", args)
        if m:
            # 1 or 2 byte reloc
            args = m.group(1) + m.group(4)
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
                if k in ["__LMUL", "__U4M",]:
                    s = [
                        ["mov",  "bx,word ptr [bx]"],
                        ["xor",  "cx,cx"],
                    ]
                    dpos = omatch(i - 2, s[-2:])
                    if 0 and dpos:
                        orewrite_inst(i, "M_LMUL_dxax_00bx_ptr", "", dpos)
                        continue
                    dpos = omatch(i - 1, s[-1:])
                    if dpos:
                        orewrite_inst(i, "M_LMUL_dxax_00bx", "", dpos)
                        continue
                if k == "__aNahdiff":
                    s = [
                        ["push", "word ptr [bp+8]"],
                        ["push", "word ptr [bp+6]"],
                        ["push", "word ptr [bp-66]"],
                        ["push", "word ptr [bp-68]"],
                    ]
                    dpos = omatch(i - 4, s[-4:])
                    if dpos:
                        orewrite_inst(i, "*DEL*", "", dpos)
                        continue
                if k == "__PIA":
                    s = [
                        ["mov",  "bx,0x1"],
                        ["xor",  "cx,cx"],
                    ]
                    dpos = omatch(i - 2, s[-2:])
                    if dpos:
                        orewrite_inst(i, "M_PIA1", "", dpos)
                        continue
        if opts.loop_rewrite and inst in ["loop"]:
            s = [
                ["mov",  r"^c[lx],0xb$"],
                ["shr",  "dx,1"],
                ["rcr",  "ax,1"],
            ]
            dpos = omatch(i - 3, s[-3:])
            if dpos:
                orewrite_inst(i, "M_shrd_11", "", dpos)
                continue
            s = [
                ["mov",  r"^c[lx],0x8$"],
                ["shl",  "ax,1"],
                ["rcl",  "dx,1"],
            ]
            dpos = omatch(i - 3, s[-3:])
            if dpos:
                orewrite_inst(i, "M_shld_8", "", dpos)
                continue
            s1 = [
                ["mov",  r"^c[lx],0x8$"],
                ["shl",  r"^word ptr \[bp([+-]\d+)\],1$"],
                ["rcl",  r"^word ptr \[bp([+-]\d+)\],1$"],
            ]
            s2 = [
                ["mov",  r"^dx,word ptr"],
                ["mov",  r"^ax,word ptr"],
            ]
            s3 = [
                ["mov",  r"^ax,word ptr"],
                ["mov",  r"^dx,word ptr"],
            ]
            dpos1 = omatch(i - 3, s1[-3:])
            dpos2 = omatch(i + 1, s2)
            dpos3 = omatch(i + 1, s3)
            if dpos1 and (dpos2 or dos3):
                bp_dx, bp_ax = dpos1[-1][2].group(1), dpos1[-2][2].group(1)
                m = "M_shld_8_bp %s %s" % (bp_dx, bp_ax)
                orewrite_inst(i, m, "", dpos1)
                continue
            s1 = [
                ["mov",  r"^word ptr \[bp([+-]\d+)\],si$"],
                ["mov",  r"^word ptr \[bp([+-]\d+)\],di$"],
                ["mov",  r"^c[lx],0xb$"],
                ["shr",  r"^word ptr \[bp([+-]\d+)\],1$"],
                ["rcr",  r"^word ptr \[bp([+-]\d+)\],1$"],
            ]
            s2 = [
                ["mov",  r"^bx,word ptr"],
                ["mov",  r"^bx,word ptr"],
                ["mov",  r"^ax,word ptr \[bp([+-]\d+)\]$"],
                ["mov",  r"^dx,word ptr \[bp([+-]\d+)\]$"],
            ]
            dpos1 = omatch(i - 5, s1[-5:])
            dpos2 = omatch(i + 1, s2)
            if dpos1 and dpos2:
                bp_dx, bp_ax = dpos1[-2][2].group(1), dpos1[-1][2].group(1)
                bp_di, bp_si = dpos1[-4][2].group(1), dpos1[-5][2].group(1)
                assert bp_dx == dpos2[-1][2].group(1)
                assert bp_ax == dpos2[-2][2].group(1)
                assert bp_dx == bp_di
                assert bp_ax == bp_si
                m = "M_shrd_11_disi_bp %s %s" % (bp_dx, bp_ax)
                orewrite_inst(i, m, "", dpos1 + dpos2[-2:])
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
    # pass 3
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
                x = inline_map.get(v[2])
                if x and v[3] <= x[1]:       # max. number of calls
                    ##print "inline", v, x
                    if x:
                        olines[i][1] = x[0]
                        olines[i][2] = "/* inlined */"
                        olines[i][2] = ""
                        olines[i][3] = None
    #
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

