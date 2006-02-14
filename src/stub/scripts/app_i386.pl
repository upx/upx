#! /usr/bin/perl -w
#
#  app_i386.pl -- assembly preprocessor for upx
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
# usage: app.pl infile outfile
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
    if ($line =~ /__([A-Z][A-Z0-9_]+)__/) {
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
        if ($line =~ /__([A-Z][A-Z0-9_]+)__/)
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

# 2nd pass
for $line (@lines)
{
    if ($line =~ /^\s+(j\w+|loop|call)\s+(\w*)/)
    {
        $label = $2;
        die "$line" if ($label =~ /(\bnear\b|\bshort\b)/);
        if (defined $labels{$label})
        {
            $ts = $labels{$label};
            if ($ts ne $cs)
            {
                $line =~ s/$label/J$i$ilabel/;
                print OU $line;
                print OU "J$i$ilabel:\n";
                print OU "\t\tsection\t.data\n\t\tdd\t";
                print OU "0,J$i$ilabel\n\t\tdb\t\'$ts\',0\n\t\tdd\t$label - S$ts$ilabel\n";
                print OU "\t\tsection\t.text\n\n";
                $line = "";
            }
        }
    }

    $line = ";$line" if ($line =~ /^\s+align\s/);

    print OU $line;
    if ($line =~ /__([A-Z][A-Z0-9_]+)__/)
    {
        print OU "S$1$ilabel:\n";
        print OU "\t\tsection\t.data\n\t\tdb\t\'$1\',0\n\t\tdd\tS$1$ilabel\n";
        print OU "\t\tsection\t.text\n\n";
        $cs = $1;
    }
    $i++;
}

# vi:ts=4:et
