#! /usr/bin/env python
## vim:set ts=4 sw=4 et: -*- coding: utf-8 -*-
#
#  bin2h.py --
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


import getopt, os, re, sys, zlib


class opts:
    dry_run = 0
    ident = None
    verbose = 0


def w_header(w, ifile, ofile, n):
    w("/* %s -- created from %s, %d (0x%x) bytes\n" % (os.path.basename(ofile), os.path.basename(ifile), n, n))
    w("""\n\
   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2006 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2006 Laszlo Molnar
   Copyright (C) 2000-2006 John F. Reiser
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
   <mfx@users.sourceforge.net>          <ml1050@users.sourceforge.net>
 */\n\n\n""")


def w_checksum(w, s, data):
    w("#define %s_SIZE    %d\n"     % (s, len(data)))
    w("#define %s_ADLER32 0x%08x\n" % (s, 0xffffffffL & zlib.adler32(data)))
    w("#define %s_CRC32   0x%08x\n" % (s, 0xffffffffL & zlib.crc32(data)))
    w("\n")


def w_data(w, data):
    def w_eol(w, i):
        if i > 0:
            w("   /* 0x%4x */" % (i - 16))
            w("\n")

    n = len(data)
    for i in range(n):
        if i % 16 == 0:
            w_eol(w, i)
        w("%3d" % ord(data[i]))
        w(", " [i == n - 1])
    i = n
    while i % 16 != 0:
        i += 1
        w("    ")
    w_eol(w, i)


def main(argv):
    shortopts, longopts = "qv", ["dry-run", "ident=", "quiet", "verbose"]
    xopts, args = getopt.gnu_getopt(argv[1:], shortopts, longopts)
    for opt, optarg in xopts:
        if 0: pass
        elif opt in ["-q", "--quiet"]: opts.verbose = opts.verbose - 1
        elif opt in ["-v", "--verbose"]: opts.verbose = opts.verbose + 1
        elif opt in ["--dry-run"]: opts.dry_run = opts.dry_run + 1
        elif opt in ["--ident"]: opts.ident = optarg
        else: assert 0, ("getopt problem:", opt, optarg, xopts, args)

    assert len(args) == 2
    ifile = args[0]
    ofile = args[1]

    # check file size
    st = os.stat(ifile)
    if 1 and st.st_size <= 0:
        print >> sys.stderr, "%s: ERROR: emtpy file" % (ifile)
        sys.exit(1)
    if 1 and st.st_size > 64*1024:
        print >> sys.stderr, "%s: ERROR: file is too big (%d bytes)" % (ifile, st.st_size)
        if re.search(r"^fold", ifile):
            print >> sys.stderr, "  (please upgrade your binutils to 2.12.90.0.15 or better)"
        sys.exit(1)

    # read ifile
    fp = open(ifile, "rb")
    data = fp.read()
    fp.close()
    assert len(data) == st.st_size

    # write ofile
    fp = open(ofile, "wb")
    w = fp.write
    if opts.verbose >= 0:
        w_header(w, ifile, ofile, len(data))
    if opts.ident:
        w_checksum(w, opts.ident.upper(), data)
        w("unsigned char %s[%d] = {\n" % (opts.ident, len(data)))
    w_data(w, data)
    if opts.ident:
        w("};\n")
    fp.close()



if __name__ == "__main__":
    sys.exit(main(sys.argv))

