#! /usr/bin/env python
## vim:set ts=4 sw=4 et: -*- coding: utf-8 -*-
#
#  gpp_inc.py -- Generic PreProcessor: include
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
    dry_run = 0
    verbose = 0
    fatal = 1
    includes = []
    mode = "c"


included_files = {}


def not_found(l, s, state):
    if opts.fatal:
        raise Exception, "%s:%d: include file %s not found" % (state[0], state[2], s)
    return l


def handle_inc_c(l, state, ofp):
    m = re.search(r"^\s*\#\s*include\s+([\"\<].+)", l)
    if not m:
        return l
    s = m.group(1).strip()
    if len(s) < 3 or s[0] != s[-1]:
        return not_found(l, s, state)
    if s[0] == '<':
        dirs = opts.includes
    elif s[0] == '"':
        dirs = [state[1]] + opts.includes
    else:
        assert 0
    inc = s[1:-1]
    for dir in dirs:
        fn = os.path.join(dir, inc)
        if os.path.isfile(fn):
            handle_file(fn, ofp, state)
            return None
    return not_found(l, s, state)


def handle_inc_nasm(l, state, ofp):
    m = re.search(r"^\s*\%\s*include\s+([\"\<].+)", l)
    if not m:
        return l
    s = m.group(1).strip()
    if len(s) < 3 or s[0] != s[-1]:
        return not_found(l, s, state)
    inc = s[1:-1]
    # info: nasm simply does concat the includes
    for prefix in opts.includes + [""]:
        fn = prefix + inc
        if os.path.isfile(fn):
            handle_file(fn, ofp, state)
            return None
    return not_found(l, s, state)


def handle_file(ifn, ofp, parent_state=None):
    state = [ifn, os.path.dirname(ifn) or ".", 0, parent_state]
    ifp = open(ifn, "rb")
    for l in ifp.readlines():
        state[2] += 1       # line counter
        l = l.rstrip("\n")
        if opts.mode == "c":
            l = handle_inc_c(l, state, ofp)
        elif opts.mode == "nasm":
            l = handle_inc_nasm(l, state, ofp)
        if l is not None:
            ofp.write(l + "\n")


def main(argv):
    ofile = None
    shortopts, longopts = "qvI::o::", ["dry-run", "mode=", "quiet", "verbose"]
    xopts, args = getopt.gnu_getopt(argv[1:], shortopts, longopts)
    for opt, optarg in xopts:
        if 0: pass
        elif opt in ["-q", "--quiet"]: opts.verbose = opts.verbose - 1
        elif opt in ["-v", "--verbose"]: opts.verbose = opts.verbose + 1
        elif opt in ["--dry-run"]: opts.dry_run = opts.dry_run + 1
        elif opt in ["-I"]: opts.includes.append(optarg)
        elif opt in ["-o"]: ofile = optarg
        elif opt in ["--mode"]: opts.mode = optarg.lower()
        else: assert 0, ("getopt problem:", opt, optarg, xopts, args)

    if ofile is None:
        assert len(args) == 2
        ifile = args[0]
        ofile = args[1]
    else:
        assert len(args) == 1
        ifile = args[0]

    assert os.path.isfile(ifile)
    ofp = open(ofile, "wb")
    handle_file(ifile, ofp)


if __name__ == "__main__":
    sys.exit(main(sys.argv))

