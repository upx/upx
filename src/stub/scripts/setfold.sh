#!/bin/sh
set -e

file="$1"

# get directory of this script
bindir=`echo "$0" | sed -e 's|[^/][^/]*$||'`
bindir=`cd "$bindir" && pwd`

sstrip="$bindir/../util/sstrip/sstrip"

# get address of symbol "fold_begin"
fold=`nm -f bsd "$file" | grep fold_begin | sed 's/^0*\([0-9a-fA-F]*\).*/0x\1/'`

# strip
objcopy -S -R .comment -R .note "$file"
"$sstrip" "$file"

# patch address
perl -w "$bindir/setfold.pl" "$file" "$fold"

exit 0

