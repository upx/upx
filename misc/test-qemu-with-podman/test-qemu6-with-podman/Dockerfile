FROM docker.io/library/alpine:3.15

# install qemu 6.1.1-r0 and some utils
RUN apk update && apk upgrade && apk add \
        bash-completion \
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
    && true

# create default user upx 2000:2000
RUN adduser upx -u 2000 -D \
    && cd /home/upx && chmod 00700 . \
    && mkdir -p .cache .local/bin src/upx \
    && chown -R upx:upx . \
    && true
USER upx
