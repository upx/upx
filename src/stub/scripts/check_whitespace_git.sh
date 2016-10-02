#! /usr/bin/env bash
## vim:set ts=4 sw=4 et:
set -e; set -o pipefail

# Copyright (C) Markus Franz Xaver Johannes Oberhumer

[[ -z $1 ]] || cd "$1" || exit 1
[[ -d .git ]] || exit 1

# info: cannot use "-z" as that needs GNU sed 4.2.2 which is not installed on Ubuntu 12.04
git ls-files --full-name | sed 's/^/\.\//' |\
sed -e '/lzma-sdk$/d' -e '/\.exe$/d' |\
LC_ALL=C sort | \
xargs -r perl -n -e '
    #print("$ARGV\n");
    if (m,[\r\x1a],) { print "ERROR: DOS EOL detected $ARGV: $_"; exit(1); }
    if (m,([ \t]+)$,) {
        # allow exactly two trailing spaces for GitHub flavoured Markdown in .md files
        if ($1 ne "  " || $ARGV !~ m,\.md$,) {
            print "ERROR: trailing whitespace detected $ARGV: $_"; exit(1);
        }
    }
    if (m,\t,) {
       if ($ARGV =~ m,(^|/)\.gitmodules$,) { }
       elsif ($ARGV =~ m,(^|/)(gnu|m)?make(file|vars),i) { }
       elsif ($ARGV =~ m,/tmp/.*\.(disasm|dump)$,) { }
       elsif ($ARGV =~ m,/src/stub/src/arch/.*\.S$,) { }
       else { print "ERROR: hard TAB detected $ARGV: $_"; exit(1); }
    }
' || exit 1
