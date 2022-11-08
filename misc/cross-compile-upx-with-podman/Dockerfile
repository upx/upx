# NOTE: this image is based on rebuild-stubs-with-upx/upx-stubtools-20210104-vX,
#   so you have to create that image first
# WARNING: we install many packages, so the resulting image needs A LOT of disk space!
FROM localhost/upx-stubtools-20210104-v7
ARG DEBIAN_FRONTEND=noninteractive

USER root

# Ubuntu 22.04
RUN apt-get update && apt-get upgrade -y \
    && apt-get install -y \
        # Linux cross compilers
        g++-aarch64-linux-gnu \
        g++-alpha-linux-gnu \
        g++-arm-linux-gnueabi \
        g++-arm-linux-gnueabihf \
        g++-hppa-linux-gnu \
        g++-i686-linux-gnu \
        g++-m68k-linux-gnu \
        g++-mips-linux-gnu \
        g++-mipsel-linux-gnu \
        g++-mips64-linux-gnuabi64 \
        g++-mips64el-linux-gnuabi64 \
        g++-powerpc-linux-gnu \
        g++-powerpc64-linux-gnu \
        g++-powerpc64le-linux-gnu \
        g++-riscv64-linux-gnu \
        g++-s390x-linux-gnu \
        g++-sh4-linux-gnu \
        g++-sparc64-linux-gnu \
        # Linux cross compilers - ILP32 on 64-bit CPUs
        g++-x86-64-linux-gnux32 \
        # Windows cross compilers
        g++-mingw-w64-i686 \
        g++-mingw-w64-x86-64 \
    && true
RUN apt-get install -y \
        # clang-14 and tools
        clang-14 clang-format-14 clang-tidy-14 clang-tools-14 lldb-14 llvm-14 \
        # QEMU and Wine
        qemu-system qemu-user wine wine32 \
        # misc
        gdb lsb-release valgrind \
    && true

# switch back to default user upx 2000:2000
USER upx
