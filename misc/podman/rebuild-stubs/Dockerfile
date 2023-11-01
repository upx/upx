FROM docker.io/library/ubuntu:22.04
ENV UPX_CONTAINER_IMAGE_NAME=upx-stubtools-20221212-v6
ARG DEBIAN_FRONTEND=noninteractive
ENV LANG=C.UTF-8

# install system packages
RUN dpkg --add-architecture i386 \
    && apt-get update && apt-get upgrade -y \
    && apt-get install -y --no-install-recommends \
        aria2 bash bash-completion ca-certificates dash git less libmpc3 libncurses5 \
        make ncurses-term perl-base python2-minimal time wget xz-utils \
        libc6:i386 zlib1g:i386 \
        # the following packages are not required for rebuilding the stubs, but
        # they do make the image *much* more convenient and also allow building
        # the full UPX binary inside the container via CMake:
        7zip bfs busybox bzip2 cabextract ccache chrpath cmake cpio curl elfutils fd-find file fzf \
        g++ gawk gdb gojq ht htop hyperfine jq libzstd-dev lsb-release lz4 lzip lzop \
        mksh moreutils ninja-build p7zip parallel patch patchelf pax-utils paxctl \
        python3 python3-pyasn1 python3-pycryptodome python3-zstd \
        re2c ripgrep rsync screen universal-ctags unzip vim yash zip zlib1g-dev zsh zstd \
        # extra packages for compiling with "gcc -m32" and and "gcc -mx32":
        g++-multilib gcc-multilib \
    && true

# manually unpack and install compat libs from Ubuntu-16.04; REQUIRED
RUN cd /root \
    && aria2c --checksum=sha-256=de22baf3dd851a10e16fbf66a243e70149ca46e06b2939fdc79429196cefc090 \
              'https://archive.kernel.org/ubuntu-archive/ubuntu/pool/main/m/mpfr4/libmpfr4_3.1.6-1_amd64.deb' \
    && mkdir packages \
    && for f in ./*.deb; do dpkg -x $f ./packages; done \
    && mv -v -n ./packages/usr/lib/x86_64-linux-gnu/lib* /usr/lib/x86_64-linux-gnu/ \
    && rm -rf ./*.deb ./packages \
    && ldconfig \
    && true

# install upx-stubtools into /usr/local/bin/bin-upx-20221212; REQUIRED
RUN cd /root \
    && aria2c --checksum=sha-256=509e06639118a79d8e79489a400e134c6d3ca36bad2c6ec29648d7c1e5b81afa \
              'https://github.com/upx/upx-stubtools/releases/download/v20221212/bin-upx-20221212.tar.xz' \
    && cd /usr/local/bin \
    && tar -xoaf /root/bin-upx-20221212.tar.xz \
    && rm /root/bin-upx-20221212.tar.xz \
    && true

# install official UPX release binaries into /usr/local/bin; not required but convenient for testing
RUN cd /root \
    && wget -q https://github.com/upx/upx/releases/download/v3.91/upx-3.91-amd64_linux.tar.bz2 \
    && xzlist="3.92 3.93 3.94 3.95 3.96 4.0.0 4.0.1 4.0.2 4.1.0 4.2.0 4.2.1" \
    && for v in $xzlist; do wget -q https://github.com/upx/upx/releases/download/v${v}/upx-${v}-amd64_linux.tar.xz; done \
    && for f in ./upx-*.tar.*; do tar -xoaf $f; done \
    && for v in 3.91 $xzlist; do d=upx-${v}-amd64_linux; ./$d/upx -qq -d $d/upx -o /usr/local/bin/upx-${v}; done \
    && rm -r ./upx-*.tar.* ./upx-*linux \
    && true

# create default user upx:upx 2000:2000
RUN useradd upx -U --uid 2000 --shell /bin/bash -m \
    && cd /home/upx && chmod 00700 . \
    # prepare ~/.cache and ~/.local for possible tmpfs mounts
    && mkdir -p .cache/tmp .local/bin src/upx \
    && for d in ccache fontconfig go-build mesa_shader_cache tmp wine zig; do mkdir -p .cache/$d; done \
    && for d in bin include lib share state; do mkdir -p .local/$d; done \
    && ln -s .cache/wine .wine && ln -s .cache/tmp tmp \
    && ln -s /usr/local/bin/bin-upx-20221212 .local/bin/bin-upx \
    && chown -R upx:upx . \
    && true
USER upx
