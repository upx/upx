#! /usr/bin/perl -w
#
#  brandelf.pl -- brand an ELF binary as Linux or FreeBSD
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


$fname = shift || die;

$sig = shift || "Linux";
die if length($sig) > 7;

sysopen (FH,$fname,2) || die;
binmode FH;

sysread (FH,$header,8) || die;
die if (substr($header, 0, 7) ne "\x7f\x45\x4c\x46\x01\x01\x01");

syswrite (FH,$sig,length($sig)) || die;
syswrite (FH,"\0\0\0\0\0\0\0\0",8-length($sig)) || die;
close (FH) || die;

exit (0);

# vi:ts=4:et
