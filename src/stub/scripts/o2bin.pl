#! /usr/bin/perl -w
#
#  o2bin.pl --
#
#  This file is part of the UPX executable compressor.
#
#  Copyright (C) 1996-2001 Markus Franz Xaver Johannes Oberhumer
#  Copyright (C) 1996-2001 Laszlo Molnar
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
#  Markus F.X.J. Oberhumer   Laszlo Molnar
#  markus@oberhumer.com      ml1050@cdata.tvnet.hu
#


$delim = $/;
undef $/;       # undef input record separator - read file as a whole

$ifile = shift || die;
$ofile = shift || die;
$x_start = shift || die;
$x_end = shift || die;

# read whole file
open(INFILE,$ifile) || die "$ifile\n";
binmode(INFILE);
$data = <INFILE>;
close(INFILE) || die;

# delete everything up to 'UPX1'
die if ($data =~ s/^.*${x_start}//s) != 1;

# delete everything from 'UPX9'
die if ($data =~ s/${x_end}.*$//s) != 1;

# write file
open(OUTFILE,">$ofile") || die "$ofile\n";
binmode(OUTFILE);
if ($ofile =~ /\.(db)$/i) {
    # asm "db xx" output
    $n = length($data);
    $l = 16;
    for ($i = 0; $i < $n; ) {
        print OUTFILE "db " if ($i % $l == 0);
        printf OUTFILE ("%d", ord(substr($data, $i, 1)));
        ++$i;
        if ($i == $n || $i % $l == 0) {
            print OUTFILE "\n";
        } else {
            print OUTFILE ",";
        }
    }
} else {
    print OUTFILE $data;
}
close(OUTFILE) || die;

undef $delim;
exit(0);

# vi:ts=4:et
