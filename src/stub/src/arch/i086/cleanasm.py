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
    olines = []
    labels = {}
    for i in range(len(lines)):
        l = lines[i]
        m = re.search("^(\s*[0-9a-z]+):\s+(\w+)(.*)", l)
        if not m:
            continue
        label = m.group(1).strip()
        inst = m.group(2).strip()
        args = ""
        if m.group(3): args = m.group(3).strip()
        args_label = None

        if label == "0" and olines: # start of next function
            break

        if inst in ["call",]:
            args = re.sub(r"^(.*?)2\s+(__\w+)$", "\g<2>", args)
        elif inst in [
            "ja", "jae", "jb", "jbe", "jcxz", "je", "jge", "jl", "jmp", "jne", "loop",
        ]:
            m = re.search("^([0-9a-z]+)\s+<", args)
            assert m, l
            if m:
                labels[m.group(1)] = ""
                args_label = m.group(1)
                args = None
        elif inst in ["movl",]:
            assert re.search(r"\b[de]s\b", args), args
            inst = "movw"
        olines.append((label, inst, args, args_label))
    #
    digits, i = 1, len(labels)
    while i >= 10:
        digits += 1
        i /= 10
    format = "%s0%dd" % ("%", digits)
    i = 0
    for label, inst, args, args_label in olines:
        if labels.has_key(label):
            labels[label] = opts.label_prefix + format % i
            i += 1
    # write ofile
    ofp = open(ofile, "wb")
    for label, inst, args, args_label in olines:
        if labels.has_key(label):
            ofp.write(labels[label] + ":\n")
        if args_label: args = labels[args_label]
        l = "%8s%-7s %s" % ("", inst, args)
        ofp.write(l.rstrip() + "\n")
    ofp.close()
    ##print olines


if __name__ == "__main__":
    sys.exit(main(sys.argv))

