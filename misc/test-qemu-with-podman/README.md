test-qemu-with-podman
=====================

This directory provides some simple scripts for creating and running
quite small Alpine Linux container images, intended for testing
statically-linked Linux executables with Podman and qemu-user.

Very short usage instructions follow.

### Where do I get statically-linked Linux binaries:
  - all recent official UPX linux release binaries are statically linked
  - the `zigcc linux-musl` artifacts as created by our GitHub Actions CI
  - many other `linux-musl` binaries are statically linked
  - many `Go` and some `Rust` programs are statically linked

### PREPARATION OUTSIDE THE CONTAINER:

```sh
    cd your-upx-top-level-directory
    mkdir -p tmp
    cd tmp

    # download official UPX release binaries
    wget https://github.com/upx/upx/releases/download/v4.0.2/upx-4.0.2-amd64_linux.tar.xz
    wget https://github.com/upx/upx/releases/download/v4.0.2/upx-4.0.2-arm64_linux.tar.xz
    wget https://github.com/upx/upx/releases/download/v4.0.2/upx-4.0.2-armeb_linux.tar.xz
    wget https://github.com/upx/upx/releases/download/v4.0.2/upx-4.0.2-arm_linux.tar.xz
    wget https://github.com/upx/upx/releases/download/v4.0.2/upx-4.0.2-i386_linux.tar.xz
    wget https://github.com/upx/upx/releases/download/v4.0.2/upx-4.0.2-mipsel_linux.tar.xz
    wget https://github.com/upx/upx/releases/download/v4.0.2/upx-4.0.2-mips_linux.tar.xz
    wget https://github.com/upx/upx/releases/download/v4.0.2/upx-4.0.2-powerpc64le_linux.tar.xz
    wget https://github.com/upx/upx/releases/download/v4.0.2/upx-4.0.2-powerpc_linux.tar.xz

    # and unpack all .tar.xz files
    for f in ./upx*.tar.xz; do tar -xJf $f; done
```

### INSIDE THE CONTAINER:

```sh
    cd /home/upx/src/upx/tmp

    # check that the official UPX release binaries do work
    qemu-i386 ./upx-4.0.2-i386_linux/upx --version
    qemu-mips ./upx-4.0.2-mips_linux/upx --version
    # ...same for more architectures

    # use qemu-mips to unpack the arm64 binary, and then run the unpacked arm64 binary:
    qemu-mips ./upx-4.0.2-mips_linux/upx -d upx-4.0.2-arm64_linux/upx -o upx-arm64-unpacked
    qemu-aarch64 ./upx-arm64-unpacked --version
    # ...same for more architectures
```
