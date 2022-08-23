FROM docker.io/library/ubuntu:22.04
ARG DEBIAN_FRONTEND=noninteractive
ENV LANG=C.UTF-8

# install system packages
RUN dpkg --add-architecture i386 \
    && apt-get update && apt-get upgrade -y \
    && apt-get install -y --no-install-recommends \
        aria2 ca-certificates git less libmpc3 libncurses5 make \
        ncurses-term perl-base python2-minimal xz-utils \
        libc6:i386 zlib1g:i386 \
    && true

# manually install compat libs from Ubuntu 16.04
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

# install upx-stubtools into /usr/local/bin/bin-upx-20210104
RUN cd /root \
    && aria2c --checksum=sha-256=abcd8337cc656fe68d7bbb2ffe0f1e5ddce618688aa0e18c1ebcc40072843884 \
              'https://github.com/upx/upx-stubtools/releases/download/v20210104/bin-upx-20210104.tar.xz' \
    && cd /usr/local/bin \
    && tar -xJf /root/bin-upx-20210104.tar.xz \
    && rm /root/bin-upx-20210104.tar.xz \
    && true

# create default user upx 2000:2000
RUN useradd upx -U --uid 2000 --shell /bin/bash -m \
    && mkdir -p /home/upx/.local/bin /home/upx/src/upx \
    && ln -s /usr/local/bin/bin-upx-20210104 /home/upx/.local/bin/bin-upx \
    && chown -R upx:upx /home/upx \
    && true
USER upx
