#!/bin/sh
set -e

# wrapper for the ASL cross-assembler
# http://john.ccac.rwth-aachen.de:8000/as/

file="$1"

perl -p -i -e '
  s,\x27,",g if m,^\s*dc,;
' "$file"

asl -cpu 68000 "$file"

exit 0

