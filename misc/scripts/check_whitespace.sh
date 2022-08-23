#! /usr/bin/env bash
## vim:set ts=4 sw=4 et:
set -e; set -o pipefail

# Copyright (C) Markus Franz Xaver Johannes Oberhumer

[[ -z $1 ]] || cd "$1" || exit 1

find . \
    -type d -name '.git' -prune -o \
    -type d -name '.hg' -prune -o \
    -type d -name 'build*' -prune -o \
    -type d -name 'tmp*' -prune -o \
    -type d -name 'vendor' -prune -o \
    -type f -iname '*.bat' -prune -o \
    -type f -iname '*.exe' -prune -o \
    -type f -iname '*.o' -prune -o \
    -type f -iname '*.obj' -prune -o \
    -type f -iname '*.out' -prune -o \
    -type f -iname '*.pdf' -prune -o \
    -type f -iname '*.swp' -prune -o \
    -type f -print0 | \
LC_ALL=C sort -z | xargs -0r perl -n -e '
    #print("$ARGV\n");
    if (m,[\x00\x01\x02\x7f\xfe\xff],) { print "ERROR: binary file detected $ARGV: $_"; exit(1); }
    if (m,[\r\x1a],) { print "ERROR: DOS EOL detected $ARGV: $_"; exit(1); }
    if (m,([ \t]+)$,) {
        # allow exactly two trailing spaces for GitHub flavoured Markdown in .md files
        if ($1 ne "  " || $ARGV !~ m,\.md$,) {
            print "ERROR: trailing whitespace detected $ARGV: $_"; exit(1);
        }
    }
    if (m,\t,) {
       if ($ARGV =~ m,(^|/)(gnu|m)?make(file|vars),i) { }
       elsif ($ARGV =~ m,/tmp/.*\.(disasm|dump)$,) { }
       elsif ($ARGV =~ m,/src/stub/src/arch/.*/lzma\w+\.S$,) { }
       else { print "ERROR: hard TAB detected $ARGV: $_"; exit(1); }
    }
' || exit 1
