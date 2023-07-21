FROM docker.io/library/alpine:3.15

# install qemu-6.1.1-r0 and some utils
RUN apk update && apk upgrade && apk add \
        bash-completion \
        musl-dbg \
        qemu-aarch64 \
        qemu-aarch64_be \
        qemu-arm \
        qemu-armeb \
        qemu-i386 \
        qemu-m68k \
        qemu-mips \
        qemu-mipsel \
        qemu-ppc \
        qemu-ppc64 \
        qemu-ppc64le \
        qemu-riscv32 \
        qemu-riscv64 \
        qemu-s390x \
        qemu-x86_64 \
        strace \
    && true

# create default user upx:upx 2000:2000
RUN adduser upx -u 2000 -D \
    && cd /home/upx && chmod 00700 . \
    && mkdir -p .cache/tmp .local/bin src/upx \
    && chown -R upx:upx . \
    && true
USER upx
