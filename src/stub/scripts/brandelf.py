#! /usr/bin/env python
## vim:set ts=4 sw=4 et: -*- coding: utf-8 -*-
#
#  brandelf.py --
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


import getopt, os, re, sys


class opts:
    bfdname = None
    dry_run = 0
    elfosabi = None
    verbose = 0



# /***********************************************************************
# //
# ************************************************************************/

def do_file(fn):
    fp = open(fn, "r+b")
    fp.seek(0, 0)
    e_ident = fp.read(16)
    fp.seek(0, 0)
    if e_ident[:7] != "\x7f\x45\x4c\x46\x01\x01\x01":
        raise Exception, "%s is not ELF" % fn
    if opts.bfdname == "elf32-i386" and opts.elfosabi == "freebsd":
        fp.seek(7, 0)
        fp.write("\x09")
    elif opts.bfdname == "elf32-i386" and opts.elfosabi == "openbsd":
        fp.seek(7, 0)
        fp.write("\x0c")
    elif opts.bfdname == "elf32-i386" and opts.elfosabi == "linux":
        fp.seek(8, 0)
        fp.write("Linux\x00\x00\x00")
    else:
        raise Exception, ("error: invalid args", opts.__dict__)
    fp.close()


def main(argv):
    shortopts, longopts = "qv", [
        "bfdname=", "dry-run", "elfosabi=", "quiet", "verbose"
    ]
    xopts, args = getopt.gnu_getopt(argv[1:], shortopts, longopts)
    for opt, optarg in xopts:
        if 0: pass
        elif opt in ["-q", "--quiet"]: opts.verbose = opts.verbose - 1
        elif opt in ["-v", "--verbose"]: opts.verbose = opts.verbose + 1
        elif opt in ["--dry-run"]: opts.dry_run = opts.dry_run + 1
        elif opt in ["--bfdname"]: opts.bfdname = optarg.lower()
        elif opt in ["--elfosabi"]: opts.elfosabi = optarg.lower()
        else: assert 0, ("getopt problem:", opt, optarg, xopts, args)
    # process arguments
    if not args:
        raise Exception, "error: no arguments given"
    for arg in args:
        do_file(arg)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))

