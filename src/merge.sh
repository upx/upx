#!/bin/bash

set -x

for i in *.h *.cpp
do
	diff3 -m ./$i ../../upx-ancestor/src/$i ../../upx-1.10/src/$i  > tmp.$$
	mv tmp.$$ ./$i
	read junk
done
