#! /usr/bin/perl -w
#
#  stripelf.pl -- strip section headers from an ELF executable
#
#  This file is part of the UPX executable compressor.
#
#  Copyright (C) 1996-2006 Markus Franz Xaver Johannes Oberhumer
#  Copyright (C) 1996-2006 Laszlo Molnar
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

#
# Strip section headers from the Linux stub. Section headers are
# optional for executables, but nevertheless binutils (2.9.1.0.25)
# complain with a "File format not recognized" error.
# Looks like a bug in binutils to me.
#
# A positive side effect of this is that `strip' cannot ruin an UPX
# compressed file any longer.
#

$fname = shift || die;
sysopen (FH,$fname,2) || die;
binmode FH;

sysseek (FH,0x20,0) || die;
sysread (FH,$num,4) || die;
$shpos = unpack ("V",$num);     # e_shoff

sysseek (FH,0x2e,0) || die;
sysread (FH,$num,2) || die;
$ssize = unpack ("v",$num);     # e_shentsize

sysseek (FH,0x32,0) || die;
sysread (FH,$num,2) || die;
$idx = unpack ("v",$num);       # e_shstrndx

sysseek (FH,$shpos + $idx * $ssize + 16,0) || die;
sysread (FH,$num,4) || die;
$neweof = unpack ("V",$num);    # sh_offset of the e_shstrndx section

$num = pack ("x6");
sysseek (FH,0x20,0) || die;
syswrite (FH,$num,4) || die;    # clear e_shoff

if (1) {
  sysseek (FH,0x2e,0) || die;
  syswrite (FH,$num,6) || die;  # clear e_shentsize, e_shnum & e_shstrndx
} else {
  sysseek (FH,0x30,0) || die;
  syswrite (FH,$num,4) || die;  # clear e_shnum & e_shstrndx
}

truncate (FH,$neweof) || die;
close(FH) || die;

print STDOUT "$0: truncated $fname to $neweof bytes.\n";
exit 0;

# vi:ts=4:et
