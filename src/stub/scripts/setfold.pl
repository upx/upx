#! /usr/bin/perl -w
#
#  setfold.pl -- set Elf32_Phdr[1].p_offset
#
#  This file is part of the UPX executable compressor.
#
#  Copyright (C) 2000 John F. Reiser.  All rights reserved.
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
#  John Reiser
#  jreiser@BitWagon.com
#

$fname = shift || die;
sysopen (FH,$fname,2) || die;
binmode FH;

$val = oct shift || die;
$num = pack("V", $val);

# 0x34 = sizeof(Elf32_Ehdr)
# 0x20 = sizeof(Elf32_Phdr)
# 4 = offset(p_offset)
sysseek (FH,0x34+0x20+4,0) || die;

syswrite (FH,$num,4) || die;

close(FH) || die;
exit 0;

# vi:ts=4:et
