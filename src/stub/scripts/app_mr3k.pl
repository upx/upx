#! /usr/bin/perl -w
#
#  app_mr3k.pl -- assembly preprocessor for upx
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
# usage: app_mr3k.pl infile outfile
#

$in = shift || die;
$ou = shift || die;

open (IN,"<$in") or die;
open (OU,">$ou") or die;
binmode IN;
binmode OU;

@lines = <IN>;

%labels = ();
$i = 0;
$cs = "";

($ilabel = $in) =~ s,^.*[\/\\],,;   # get basename
$ilabel =~ s/\W//g;

# 1st pass
for $line (@lines)
{
    $labels{$1} = "$cs" if ($line =~ /^(\w+):/ && $cs);
    if ($line =~ /__([A-Z0-9]{8})__/) {
        $cs = $1;
        # verify the line
        if ($line =~ /^[\%\;]ifdef/) {
            # ok
        } elsif ($line =~ /^([\%\;]\w+)?\s*;/) {
            # ok
        } else {
            print STDERR "$in:$i:warning 1:$line"
        }
    }

    if ($line =~ /^[\%\;](if|el|endi)/)
    {
        if ($line =~ /__([A-Z0-9]{8})__/)
        {
            $line=";$line" unless ($line =~ /^\;/);
        }
        else
        {
            print STDERR "$in:$i:warning 2:$line";
        }
    }
    $line =~ s/\.ash/\.asy/ if ($line =~ /^\s*\%include/);
    $i++;
}

$cs = "";
$i = 0;
$test = "";

# 2nd pass
for $line (@lines)
{
    $line = ";$line" if ($line =~ /^\s+align\s/);

    print OU $line;

    if ($line =~ /__([A-Z0-9]{8})__/)
    {
        print OU "S$1$ilabel:\n";
        push @{ $test[++$#test] }, "\t\tDB\t\t\"$1\",0\n\t\tDW_UNALIGNED\tS$1$ilabel\n";

        $cs = $1;
    }
    $i++;

    if ($line =~ /section .data/)
    {
        for $i ( 0 .. $#test )
        {
            print OU "@{$test[$i]}";
        }
    }
}

# vi:ts=4:et
