#!/bin/sh
set -e

# wrapper for the ASL cross-assembler (version 1.42bld9)
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

