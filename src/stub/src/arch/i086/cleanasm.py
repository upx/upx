#! /usr/bin/env python
## vim:set ts=4 sw=4 et: -*- coding: utf-8 -*-
#
#  cleanasm.py --
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
    label_prefix = ".L"
    verbose = 0
    # optimizer flags
    auto_inline = 1
    call_rewrite = 1
    loop_rewrite = 1
    mov_rewrite = 1


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
    def inst_has_label(inst):
        return inst in [
            "call", "ja", "jae", "jb", "jbe", "jcxz", "je",
            "jg", "jge", "jl", "jle", "jmp", "jne", "loop",
        ]
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
    def omatch(pos, mlen, m, debug=0):
        assert len(m) >= abs(mlen)
        def sgn(x):
            if x < 0: return -1
            if x > 0: return  1
            return 0
        def match(a, b):
            if b is None:
                return False
            if "^" in a or "*" in a or "$" in a:
                # regexp
                return re.search(a, b.lower())
            else:
                return a.lower() == b.lower()
        mpos = []
        while len(mpos) != abs(mlen):
            if pos < 0 or pos >= len(olines):
                return []
            o = olines[pos]
            if o[1] != "*DEL*":
                mpos.append(pos)
            pos += sgn(mlen)
        if mlen < 0:
            mpos.reverse()
        if debug and 1: print mlen, m, [olines[x] for x in mpos]
        dpos = []
        i = -abs(mlen)
        while i < 0:
            pos = mpos[i]
            o = olines[pos]
            assert o[1] != "*DEL*"
            assert len(m[i]) == 2, (i, m)
            m0 = match(m[i][0], o[1])
            m1 = match(m[i][1], o[2])
            if not m0 or not m1:
                return []
            dpos.append([pos, m0, m1])
            i += 1
        assert len(dpos) == abs(mlen)
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
        if m.group(3):
            args = m.group(3).strip()
        if not inst_has_label(inst):
            def hex2int(m): return str(int(m.group(0), 16))
            args = re.sub(r"\b0x[0-9a-fA-F]+\b", hex2int, args)
        #
        if 1 and inst in ["movl",] and re.search(r"\b[de]s\b", args):
            # work around a bug in objdump 2.17 (fixed in binutils 2.18)
            inst = "mov"
        m = re.search(r"^(.+?)\b(0|0x0)\s+(\w+):\s+(1|2|R_386_16|R_386_PC16)\s+(__\w+)$", args)
        if m:
            # 1 or 2 byte reloc
            args = m.group(1) + m.group(5)
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
                if k == "__aNahdiff":
                    s = [
                        ["push", "word ptr [bp+8]"],
                        ["push", "word ptr [bp+6]"],
                        ["push", r"word ptr \[bp([+-](\d+))\]$"],
                        ["push", r"word ptr \[bp([+-](\d+))\]$"],
                    ]
                    dpos = omatch(i-1, -4, s)
                    if dpos:
                        orewrite_inst(i, "*DEL*", "", dpos)
                        continue
                if k in ["__LMUL", "__U4M",]:
                    s1 = [
                        ["mov",  "bx,768"],     # 0x300
                        ["xor",  "cx,cx"],
                    ]
                    s2 = [
                        ["shl",  "ax,1"],
                        ["rcl",  "dx,1"],
                    ]
                    dpos1 = omatch(i-1, -2, s1)
                    dpos2 = omatch(i+1,  2, s2)
                    if dpos1 and dpos2:
                        orewrite_inst(i, "M_U4M_dxax_0x0600", "", dpos1 + dpos2)
                        continue
                    s = [
                        ["mov",  "bx,word ptr [bx]"],
                        ["xor",  "cx,cx"],
                    ]
                    dpos = omatch(i-1, -2, s, debug=0)
                    if 0 and dpos:
                        orewrite_inst(i, "M_U4M_dxax_00bx_ptr", "", dpos)
                        continue
                    dpos = omatch(i-1, -1, s)
                    if dpos:
                        orewrite_inst(i, "M_U4M_dxax_00bx", "", dpos)
                        continue
                if k == "__PIA":
                    s = [
                        ["mov",  "bx,1"],
                        ["xor",  "cx,cx"],
                    ]
                    dpos = omatch(i-1, -2, s)
                    if dpos:
                        orewrite_inst(i, "M_PIA1", "", dpos)
                        continue
                if k == "__PTC":
                    s = [
                        ["jne",  "(.*)"],
                    ]
                    dpos = omatch(i+1, 1, s)
                    if dpos:
                        olines[i][1] = "M_PTC_JNE"
                        k, v = parse_label("jne", dpos[0][2].group(1))
                        orewrite_call(i, k, v, dpos)
                        continue
        if opts.loop_rewrite and inst in ["loop"]:
            s = [
                ["mov",  r"^c[lx],11$"],
                ["shr",  "dx,1"],
                ["rcr",  "ax,1"],
            ]
            dpos = omatch(i-1, -3, s)
            if dpos:
                orewrite_inst(i, "M_shrd_11", "", dpos)
                continue
            s = [
                ["mov",  r"^c[lx],8$"],
                ["shl",  "ax,1"],
                ["rcl",  "dx,1"],
            ]
            dpos = omatch(i-1, -3, s)
            if dpos:
                orewrite_inst(i, "M_shld_8", "", dpos)
                continue
            s1 = [
                ["mov",  r"^c[lx],8$"],
                ["shl",  "si,1"],
                ["rcl",  "di,1"],
            ]
            s2 = [
                ["les",  r"^bx,dword ptr \[bp([+-](\d+))\]$"],
            ]
            dpos1 = omatch(i-1, -3, s1)
            dpos2 = omatch(i+1,  1, s2)
            if 1 and dpos1 and dpos2:
                # bx and cx are free for use
                orewrite_inst(i, "M_shld_disi_8_bxcx", "", dpos1)
                continue
            s1 = [
                ["mov",  "ax,si"],
                ["mov",  r"^c[lx],8$"],
                ["shl",  "ax,1"],
                ["rcl",  "di,1"],
            ]
            s2 = [
                ["mov",  "si,ax"],
                ["les",  r"^bx,dword ptr \[bp([+-](\d+))\]$"],
            ]
            dpos1 = omatch(i-1, -4, s1)
            dpos2 = omatch(i+1,  2, s2)
            if 1 and dpos1 and dpos2:
                # bx and cx are free for use
                orewrite_inst(i, "M_shld_diax_8_bxcx", "", dpos1[-3:])
                continue
            s1 = [
                ["mov",  r"^c[lx],8$"],
                ["shl",  r"^word ptr \[bp([+-](\d+))\],1$"],
                ["rcl",  r"^word ptr \[bp([+-](\d+))\],1$"],
            ]
            s2 = [
                ["mov",  r"^dx,word ptr"],
                ["mov",  r"^ax,word ptr"],
            ]
            s3 = [
                ["mov",  r"^ax,word ptr"],
                ["mov",  r"^dx,word ptr"],
            ]
            dpos1 = omatch(i-1, -3, s1)
            dpos2 = omatch(i+1,  2, s2)
            dpos3 = omatch(i+1,  2, s3)
            if dpos1 and (dpos2 or dos3):
                bp_dx, bp_ax = dpos1[-1][2].group(1), dpos1[-2][2].group(1)
                m = "M_shld_8_bp %s %s" % (bp_dx, bp_ax)
                orewrite_inst(i, m, "", dpos1)
                continue
            s1 = [
                ["mov",  r"^word ptr \[bp([+-](\d+))\],si$"],
                ["mov",  r"^word ptr \[bp([+-](\d+))\],di$"],
                ["mov",  r"^c[lx],11$"],
                ["shr",  r"^word ptr \[bp([+-](\d+))\],1$"],
                ["rcr",  r"^word ptr \[bp([+-](\d+))\],1$"],
            ]
            s2 = [
                ["mov",  r"^bx,word ptr"],
                ["mov",  r"^bx,word ptr"],
                ["mov",  r"^ax,word ptr \[bp([+-](\d+))\]$"],
                ["mov",  r"^dx,word ptr \[bp([+-](\d+))\]$"],
            ]
            dpos1 = omatch(i-1, -5, s1)
            dpos2 = omatch(i+1,  4, s2)
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
        if opts.mov_rewrite and inst in ["mov"]:
            s = [
                ["mov",  r"^al,byte ptr \[(di|si)\]$"],
                ["xor",  r"^ah,ah$"],
                ["mov",  r"^word ptr \[bp([+-](\d+))\],ax$"],
                ["mov",  r"^word ptr \[bp([+-](\d+))\],(0|1)$"],
                ["mov",  r"^word ptr \[bp([+-](\d+))\],(0|1)$"],
                ["mov",  r"^word ptr \[bp([+-](\d+))\],(0|1)$"],
                ["mov",  r"^word ptr \[bp([+-](\d+))\],(0|1)$"],
                ["mov",  r"^word ptr \[bp([+-](\d+))\],(0|1)$"],
                ["mov",  r"^word ptr \[bp([+-](\d+))\],(0|1)$"],
                ["mov",  r"^word ptr \[bp([+-](\d+))\],(0|1)$"],
                ["mov",  r"^word ptr \[bp([+-](\d+))\],(0|1)$"],
                ["mov",  r"^word ptr \[bp([+-](\d+))\],(0|1)$"],
                ["mov",  r"^bx,word ptr \[bp([+-](\d+))\]$"],
                ["mov",  r"^word ptr \[bx\],(0)$"],
                ["mov",  r"^word ptr \[bx([+-](\d+))\],(0)$"],
                ["mov",  r"^bx,word ptr \[bp([+-](\d+))\]$"],
                ["mov",  r"^word ptr \[bx\],(0)$"],
                ["mov",  r"^word ptr \[bx([+-](\d+))\],(0)$"],
                ["mov",  r"^dl,byte ptr \[(di|si)([+-](\d+))\]$"],
                ["xor",  r"^dh,dh$"],
                ["mov",  r"^cx,ax$"],
            ]
            dpos = omatch(i, -len(s), s)
            if dpos:
                ipos, n_del = 16, 0
                pos0 = dpos[0][0]
                r = []
                for pos, m0, m1 in dpos:
                    assert pos == pos0 + len(r)
                    r.append([olines[pos][1], olines[pos][2]])
                z0 = r[0]; z1 = r[2]; del r[:3]
                r.insert(0, ["xor", "ax,ax"])
                r.insert(ipos, z0); r.insert(ipos + 1, z1)
                i = 0
                while i < len(r):
                    inst, args = r[i]
                    if inst == "mov" and args.endswith(",0"):
                        r[i] = [inst, args[:-1] + "ax"]
                    elif inst == "mov" and args.endswith(",1"):
                        assert i < ipos
                        r.insert(ipos, [inst, args[:-1] + "ax"])
                        del r[i]; i -= 1; n_del += 1
                    i += 1
                assert len(r) == len(dpos)
                pos = pos0
                for inst, args in r:
                    ##print pos-pos0, inst, args
                    olines[pos][1] = inst
                    olines[pos][2] = args
                    pos += 1
                if n_del:
                    olines.insert(pos0 + ipos - n_del, [None, "inc", "ax", None])
                continue
        #
        if inst_has_label(inst):
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
    current_label = None
    for label, inst, args, args_label in olines:
        if labels.has_key(label):
            current_label = labels[label][2]
            if opts.verbose:
                ofp.write("%s: /* %d */\n" % (labels[label][2], labels[label][3]))
            else:
                ofp.write("%s:\n" % (labels[label][2]))
        if inst == "*DEL*":
            continue
        if 1 and current_label in [".Lf122", ".Lf123", ".Lf124", ".Ls122", ".Ls123", ".Ls124"]:
            continue
        if args_label:
            if opts.verbose:
                args = "%s /* %d */" % (labels[args_label][2], labels[args_label][3])
            else:
                args = labels[args_label][2]
        if 0:
            # remove unneeded "byte/word/dword ptr"
            # [this works, but disabled for now as we gain nothing]
            if re.search(r"\bbyte ptr ", args):
                if re.search(r"^[abcd][hl],", args): args = args.replace("byte ptr ", "")
                if re.search(r",[abcd][hl]$", args): args = args.replace("byte ptr ", "")
            if re.search(r"\bword ptr ", args):
                if re.search(r"^[abcds][ix],", args): args = args.replace("word ptr ", "")
                if re.search(r",[abcds][ix]$", args): args = args.replace("word ptr ", "")
            if re.search(r"\bdword ptr ", args):
                if re.search(r"^[abcd][x],",  args): args = args.replace("dword ptr ", "")
        l = "%8s%-7s %s" % ("", inst, args)
        ofp.write(l.rstrip() + "\n")
    ofp.close()
    ##print olines


if __name__ == "__main__":
    sys.exit(main(sys.argv))

