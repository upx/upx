#! /bin/sh

#
#  bin2w.sh
#
#  This file is part of the UPX executable compressor.
#
#  Copyright (C) 2008-2013 John F. Reiser
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
#  John F. Reiser
#  <jreiser@users.sourceforge.net>
#

# Convert binary 4-byte integers to assembler input.
# Endian-ness will be handled by the assembler.

od -Ax -tx4 |
sed -e '
  /^\([^ ]*\) \([^ ]*\) \([^ ]*\) \([^ ]*\) \([^ ]*\)$/ {
    s//\/*0x\1*\/ .long 0x\2,0x\3,0x\4,0x\5/p
    d
  }
  /^\([^ ]*\) \([^ ]*\) \([^ ]*\) \([^ ]*\)$/ {
    s//\/*0x\1*\/ .long 0x\2,0x\3,0x\4/p
    d
  }
  /^\([^ ]*\) \([^ ]*\) \([^ ]*\)$/ {
    s//\/*0x\1*\/ .long 0x\2,0x\3/p
    d
  }
  /^\([^ ]*\) \([^ ]*\)$/ {
    s//\/*0x\1*\/ .long 0x\2/p
    d
  }
  /^\([^ ]*\)$/ {
    s//\/*0x\1*\//p
    d
  }
'
