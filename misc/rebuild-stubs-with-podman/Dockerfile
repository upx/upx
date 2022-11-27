FROM docker.io/library/ubuntu:22.04
ENV UPX_CONTAINER_IMAGE_NAME=upx-stubtools-20210104-v10
ARG DEBIAN_FRONTEND=noninteractive
ENV LANG=C.UTF-8

# install system packages
RUN dpkg --add-architecture i386 \
    && apt-get update && apt-get upgrade -y \
    && apt-get install -y --no-install-recommends \
        aria2 ca-certificates git less libmpc3 libncurses5 make \
        ncurses-term perl-base python2-minimal wget xz-utils \
        libc6:i386 zlib1g:i386 \
        # the following packages are not required for rebuilding the stubs, but
        # they do make the image much more convenient and also allow building
        # the full UPX binary inside the container via CMake:
        7zip bzip2 cmake elfutils file g++ htop libzstd-dev lzip lzop ninja-build p7zip \
        patch patchelf pax-utils rsync screen unzip vim zip zlib1g-dev zsh zstd \
    && true

# manually install compat libs from Ubuntu 16.04; REQUIRED
RUN cd /root \
    && aria2c --checksum=sha-256=2605f43f8047fc972855bb909f1dd7af761bbfd35ae559ad689b0d55a4236d69 \
              'http://mirror.enzu.com/ubuntu/pool/main/g/gmp/libgmp10_6.1.0+dfsg-2_amd64.deb' \
    && aria2c --checksum=sha-256=3973a97387bbe0e8a775995c22861d8478edee2a594e8117970f635de25b2c8a \
              'http://mirror.enzu.com/ubuntu/pool/main/m/mpfr4/libmpfr4_3.1.4-1_amd64.deb' \
    && mkdir packages \
    && for f in ./*.deb; do dpkg -x $f ./packages; done \
    && mv -v -n ./packages/usr/lib/x86_64-linux-gnu/lib* /usr/lib/x86_64-linux-gnu/ \
    && rm -rf ./*.deb ./packages \
    && ldconfig \
    && true

# install upx-stubtools into /usr/local/bin/bin-upx-20210104; REQUIRED
RUN cd /root \
    && aria2c --checksum=sha-256=abcd8337cc656fe68d7bbb2ffe0f1e5ddce618688aa0e18c1ebcc40072843884 \
              'https://github.com/upx/upx-stubtools/releases/download/v20210104/bin-upx-20210104.tar.xz' \
    && cd /usr/local/bin \
    && tar -xoaf /root/bin-upx-20210104.tar.xz \
    && rm /root/bin-upx-20210104.tar.xz \
    && true

# install pre-built binary UPX versions into /usr/local/bin; not required but convenient for testing
RUN cd /root \
    && wget -q https://github.com/upx/upx/releases/download/v3.91/upx-3.91-amd64_linux.tar.bz2 \
    && for v in 3.92 3.93 3.94 3.95 3.96 4.0.0 4.0.1; do wget -q https://github.com/upx/upx/releases/download/v${v}/upx-${v}-amd64_linux.tar.xz; done \
    && for f in ./upx-*.tar.*; do tar -xoaf $f; done \
    && for v in 3.91 3.92 3.93 3.94 3.95 3.96 4.0.0 4.0.1; do d=upx-${v}-amd64_linux; ./$d/upx -qq -d $d/upx -o /usr/local/bin/upx-${v}; done \
    && rm -r ./upx-*.tar.* ./upx-*linux \
    && true

# create default user upx 2000:2000
RUN useradd upx -U --uid 2000 --shell /bin/bash -m \
    && mkdir -p /home/upx/.local/bin /home/upx/src/upx \
    && ln -s /usr/local/bin/bin-upx-20210104 /home/upx/.local/bin/bin-upx \
    && chown -R upx:upx /home/upx \
    && true
USER upx
