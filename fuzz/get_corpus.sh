#!/bin/bash
cd /tmp
mkdir upx_fuzzing_corpus
cd upx_fuzzing_corpus
wget https://github.com/upx/upx/files/2647056/POC.zip
unzip -o POC.zip -d 2647056
rm POC.zip

# Get these POCs: https://github.com/upx/upx/issues/315
wget https://github.com/gutiniao/afltest/archive/master.zip
unzip -o master.zip
rm master.zip

# Get some more fuzzing cases
wget https://github.com/liamjm/upx_fuzzing_corpus/archive/master.zip
unzip -o master.zip
rm master.zip


