#!/bin/bash

set -x

for i in Makefile *.h *.c *.ash *.asm *.lds
do
	diff3 -m ./$i ../../../upx-ancestor/src/stub/$i ../../../upx-1.10/src/stub/$i  > tmp.$$
	mv tmp.$$ ./$i
	read junk
done
