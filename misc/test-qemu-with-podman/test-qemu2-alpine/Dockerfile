FROM docker.io/library/alpine:3.8

# install qemu-2.12.0-r3 and some utils
RUN apk update && apk upgrade && apk add \
        bash-completion \
        musl-dbg \
        qemu-aarch64 \
        qemu-arm \
        qemu-armeb \
        qemu-i386 \
        qemu-mips \
        qemu-mipsel \
        qemu-ppc \
        qemu-ppc64 \
        qemu-ppc64le \
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
