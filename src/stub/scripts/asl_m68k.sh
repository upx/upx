#! /bin/sh
#
#  asl_m68k.sh --
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

set -e

# wrapper for the ASL cross-assembler (version 1.42bld43)
#   http://john.ccac.rwth-aachen.de:8000/as/
#   http://john.ccac.rwth-aachen.de:8000/ftp/as/source/c_version/

file="$1"
test -f "$file" || exit 1

ofile=`echo "$file" | sed 's/\.[a-z]*$/.o/'`

# convert ' to " in dc.x statements
perl -p -i -e '
  s,\x27,",g if m,^\s*dc\.,;
' "$file"

echo asl -q -xC -U -cpu 68000 -o "$ofile" -L "$file"
     asl -q -xC -U -cpu 68000 -o "$ofile" -L "$file"

exit 0

