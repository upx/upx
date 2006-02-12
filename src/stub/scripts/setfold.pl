#! /usr/bin/perl -w
#
#  setfold.pl -- set Elf32_Phdr[1].p_offset
#
#  This file is part of the UPX executable compressor.
#
#  Copyright (C) 2000-2006 John F. Reiser
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
#  John F. Reiser
#  <jreiser@users.sourceforge.net>
#


$fname = shift || die;
sysopen (FH,$fname,2) || die;
binmode FH;

$fsize = (stat($fname))[7];

$val = shift || die "$val";
###print STDERR "$val\n";
$val = oct($val);               # acutally hex()
$val = $val & 0xfff;
printf STDERR "setfold info: $fname: setting fold to 0x%x, file size 0x%x\n", $val, $fsize;
die unless $val > 0;
die unless $val < $fsize;
$num = pack("V", $val);

# 0x34 = sizeof(Elf32_Ehdr)
# 0x20 = sizeof(Elf32_Phdr)
# 4 = offset(p_offset)
sysseek (FH,0x34+0x20+4,0) || die;

syswrite (FH,$num,4) || die;

close(FH) || die;
exit 0;

# vi:ts=4:et
