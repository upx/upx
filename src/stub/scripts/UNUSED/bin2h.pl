#! /usr/bin/perl -w
#
#  bin2h.pl --
#
#  This file is part of the UPX executable compressor.
#
#  Copyright (C) 1996-2006 Markus Franz Xaver Johannes Oberhumer
#  Copyright (C) 1996-2006 Laszlo Molnar
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


use Compress::Zlib;

$delim = $/;
undef $/;       # undef input record separator - read file as a whole

$ifile = shift || die;
$ident = shift || die;
$ofile = shift || die;

$opt_q = "";
$opt_q = shift if ($#ARGV >= 0);

# open ifile
open(INFILE,$ifile) || die "open $ifile\n";
binmode(INFILE);

# check file size
@st = stat($ifile);
if (1 && $st[7] <= 0) {
    print STDERR "$ifile: ERROR: emtpy file\n";
    exit(1);
}
if (1 && $st[7] > 64*1024) {
    print STDERR "$ifile: ERROR: file is too big (${st[7]} bytes)\n";
    if ($ifile =~ /^fold/) {
        print STDERR "  (please upgrade your binutils to 2.12.90.0.15 or better)\n";
    }
    exit(1);
}

# read whole file
$data = <INFILE>;
close(INFILE) || die;
$n = length($data);
die if ($n != $st[7]);

# open ofile
open(OUTFILE,">$ofile") || die "open $ofile\n";
binmode(OUTFILE);
select(OUTFILE);

$if = $ifile;
$if =~ s/.*[\/\\]//;
$of = $ofile;
$of =~ s/.*[\/\\]//;

if ($opt_q ne "-q") {
printf ("/* %s -- created from %s, %d (0x%x) bytes\n", $of, $if, $n, $n);
print <<"EOF";

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
   <mfx\@users.sourceforge.net>          <ml1050\@users.sourceforge.net>
 */


EOF
}

$s = $ident;
$s =~ tr/a-z/A-Z/;
printf("#define %s_SIZE    %d\n", $s, $n);
printf("#define %s_ADLER32 0x%08x\n", $s, &adler32($data));
printf("#define %s_CRC32   0x%08x\n", $s, &crc32($data));
printf("\n");

printf("unsigned char %s[%d] = {", $ident, $n);
for ($i = 0; $i < $n; $i++) {
    if ($i % 16 == 0) {
        printf("   /* 0x%4x */", $i - 16) if $i > 0;
        print "\n";
    }
    printf("%3d", ord(substr($data, $i, 1)));
    print "," if ($i != $n - 1);
}

while (($i % 16) != 0) {
    $i++;
    print "    ";
}
printf("    /* 0x%4x */", $i - 16);

print "\n};\n";

close(OUTFILE) || die;
select(STDOUT);

undef $delim;
exit(0);

# vi:ts=4:et
