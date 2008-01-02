#! /usr/bin/env python
## vim:set ts=4 sw=4 et: -*- coding: utf-8 -*-
#
#  gpp_inc.py -- Generic PreProcessor: include
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


import getopt, os, re, sys


class opts:
    dry_run = 0
    verbose = 0
    fatal = 1
    includes = []
    mode = "c"
    target_mf = None
    target_mmd = None


files_md = []
files_mmd = []
files_st = {}

def add_dep(state, fn, mode):
    if mode:
        files = files_md
    else:
        files = files_mmd
    fn = os.path.normpath(fn)
    fn = os.path.normcase(fn)
    if fn in files:
        return
    # FIXME: could use os.path.samestat() etc.
    files.append(fn)
    files_st[fn] = os.stat(fn)


def not_found(state, l, inc, fatal=None):
    if fatal is None:
        fatal = opts.fatal
    if fatal:
        raise Exception, "%s:%d: include file %s not found" % (state[0], state[2], inc)
    return l


def parse_comment(state, l, comment):
    cf = {}
    if not comment: return cf
    m = re.search(r"gpp_inc:(.+?):", comment.strip())
    if not m: return cf
    flags = re.split(r"\s+", m.group(1).strip())
    for f in flags:
        assert f, (f, flags, m.groups(), comment)
        if   f == "ignore=0": cf["fatal"] = 1
        elif f == "ignore=1": cf["fatal"] = 0
        else:
            raise Exception, "%s:%d: bad flags %s %s" % (state[0], state[2], f, str(flags))
    return cf


def handle_inc_c(state, l, ofp):
    m = re.search(r"^\s*\#\s*include\s+([\"\<])(.+?)([\"\>])(.*)$", l)
    if not m:
        return l
    q1, inc, q2, comment = m.groups()
    cf = parse_comment(state, l, comment)
    if q1 == '<' and q2 == '>':
        dirs = opts.includes
    elif q1 == '"' and q2 == '"':
        dirs = [state[1]] + opts.includes
    else:
        raise Exception, "syntax error: include line " + l
    for dir in dirs:
        fn = os.path.join(dir, inc)
        if os.path.isfile(fn):
            add_dep(state, fn, q1 == '<')
            handle_file(fn, ofp, state)
            return None
    return not_found(state, l, inc, cf.get("fatal"))


def handle_inc_nasm(state, l, ofp):
    m = re.search(r"^\s*\%\s*include\s+([\"\<])(.+?)([\"\>])(.*)$", l)
    if not m:
        return l
    q1, inc, q2, comment = m.groups()
    cf = parse_comment(state, l, comment)
    if q1 == '<' and q2 == '>':
        pass
    elif q1 == '"' and q2 == '"':
        pass
    else:
        raise Exception, "syntax error: include line " + l
    # info: nasm simply does concat the includes
    for prefix in opts.includes + [""]:
        fn = prefix + inc
        if os.path.isfile(fn):
            add_dep(state, fn, False)
            handle_file(fn, ofp, state)
            return None
    return not_found(state, l, inc, cf.get("fatal"))


def handle_file(ifn, ofp, parent_state=None):
    state = [ifn, os.path.dirname(ifn) or ".", 0, parent_state]
    ifp = open(ifn, "rb")
    for l in ifp.readlines():
        state[2] += 1       # line counter
        l = l.rstrip("\n")
        if opts.mode == "c":
            l = handle_inc_c(state, l, ofp)
        elif opts.mode == "nasm":
            l = handle_inc_nasm(state, l, ofp)
        if l is not None:
            ofp.write(l + "\n")


def main(argv):
    ofile = None
    shortopts, longopts = "qvI:o:", ["dry-run", "MF=", "MMD=", "mode=", "quiet", "verbose"]
    xopts, args = getopt.gnu_getopt(argv[1:], shortopts, longopts)
    for opt, optarg in xopts:
        if 0: pass
        elif opt in ["-q", "--quiet"]: opts.verbose = opts.verbose - 1
        elif opt in ["-v", "--verbose"]: opts.verbose = opts.verbose + 1
        elif opt in ["--dry-run"]: opts.dry_run = opts.dry_run + 1
        elif opt in ["-I"]: opts.includes.append(optarg)
        elif opt in ["-o"]: ofile = optarg
        elif opt in ["--mode"]: opts.mode = optarg.lower()
        elif opt in ["--MF"]: opts.target_mf = optarg
        elif opt in ["--MMD"]: opts.target_mmd = optarg
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
    ofp.close()

    if opts.target_mmd:
        fn = ofile + ".d"
        if opts.target_mf:
            fn = opts.target_mf
        if os.path.isfile(fn):
            os.unlink(fn)
        if files_mmd:
            fp = open(fn, "wb")
            fp.write("%s : \\\n" % opts.target_mmd)
            for i, f in enumerate(files_mmd):
                if i < len(files_mmd) - 1:
                    fp.write("  %s \\\n" % f)
                else:
                    fp.write("  %s\n" % f)
            fp.close()


if __name__ == "__main__":
    sys.exit(main(sys.argv))

