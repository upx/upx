# Fuzzing

NOTE: Work in progress

## Overview

The focus of this fuzzing work is to uncover issues in the decompression mode of
UPX. This mode is of greater interest as this is the time when someone is more
likely to be dealing with untrusted input.

The fuzzing will make use of libFuzzer. The goal is to integrate this with
[OSS-Fuzz](https://github.com/google/oss-fuzz).

## Building

1.  Build ucl

```
cd /tmp/
wget http://www.oberhumer.com/opensource/ucl/download/ucl-1.03.tar.gz
tar -zxf ucl-1.03.tar.gz
cd ucl-1.03/
./configure CC="gcc -std=gnu89"
make all
```

1.  Build UPX

```
export UPX_UCLDIR=/tmp/ucl-1.03
export BUILD_TYPE_SANITIZE=1
make all
```

1.  Build fuzzer

NOTE: requires changes to src/main.cpp to remove the main() function and allow
the fuzzer to link as a library. See this branch for the changes.

```./fuzz/build.sh```

1. Get a corpus of samples.

```./fuzzer/get_corpus.sh```

1. Fuzz

```./fuzz/unpacker_fuzzer /tmp/upx_fuzzing_corpus```

