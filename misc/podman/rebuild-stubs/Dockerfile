# Copyright (C) Markus Franz Xaver Johannes Oberhumer
FROM docker.io/library/ubuntu:24.04
ENV UPX_CONTAINER_IMAGE_NAME=upx-stubtools-20240405-v1
ARG DEBIAN_FRONTEND=noninteractive
ENV LANG=C.UTF-8

# install system packages
RUN dpkg --add-architecture i386 \
    && apt-get update && apt-get upgrade -y \
    && apt-get install -y --no-install-recommends \
        aria2 bash bash-completion ca-certificates coreutils curl diffutils findutils git grep gzip \
        less libmpc3 make ncurses-term perl-base python3 sed sudo tar time util-linux wget xz-utils \
        libc6:i386 zlib1g:i386 \
        # the following packages are not required for rebuilding the stubs, but
        # they do make the image *much* more convenient and also allow building
        # the full UPX binary inside the container via CMake:
        7zip bat bfs btop bubblewrap busybox bzip2 bzip3 cabextract ccache chrpath cmake cpio \
        dash diffstat direnv dmidecode elfutils execstack eza fd-find file fish fzf \
        g++ gawk gdb gh gojq ht htop hyperfine jq just ksh \
        lftp libzstd-dev lsb-release lsd lz4 lzip lzop man minify mksh moreutils musl neovim \
        ninja-build p7zip parallel patch patchelf patchutils pax-utils proot \
        python3-pyasn1 python3-pycryptodome python3-pycurl python3-tomli python3-tomli-w \
        python3-yaml python3-zstd \
        re2c ripgrep rsync screen socat strace tree universal-ctags unzip valgrind yash yq \
        zip zlib1g-dev zoxide zsh zstd \
        # extra packages for compiling with "gcc -m32" and "gcc -mx32":
        g++-multilib gcc-multilib \
    && apt-get install -y \
        gcc-riscv64-linux-gnu \
    && true

# install python2-minimal packages from Debian-11; REQUIRED
RUN cd /root \
    && aria2c --checksum=sha-256=85833c92314011b89bbead6f9a5a7a360b5ee4f9b95e001225afead4ebf9f7d3 \
              'https://ftp.debian.org/debian/pool/main/p/python2.7/libpython2.7-minimal_2.7.18-8+deb11u1_amd64.deb' \
    && aria2c --checksum=sha-256=44f0cdc1a343303d31c55fcb3a6402cfe326adf749ec30e375e6e0c0c5f7c1f7 \
              'https://ftp.debian.org/debian/pool/main/p/python2.7/python2.7-minimal_2.7.18-8+deb11u1_amd64.deb' \
    && dpkg -i ./*.deb \
    && rm ./*.deb \
    && if ! test -f /usr/bin/python2; then ln -s -v python2.7 /usr/bin/python2; fi \
    && ldconfig \
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
    && curl -sS -L -O https://github.com/upx/upx/releases/download/v3.91/upx-3.91-amd64_linux.tar.bz2 \
    && xzversions="3.92 3.93 3.94 3.95 3.96 4.0.0 4.0.1 4.0.2 4.1.0 4.2.0 4.2.1 4.2.2 4.2.3 4.2.4 5.0.0 5.0.1 5.0.2 5.1.0 5.1.1 5.2.0 5.2.1" \
    && for v in $xzversions; do curl -sS -L -O https://github.com/upx/upx/releases/download/v${v}/upx-${v}-amd64_linux.tar.xz; done \
    && for f in ./upx-*.tar.*; do tar -xoaf $f; done \
    && for v in 3.91 $xzversions; do d=upx-${v}-amd64_linux; ./$d/upx -qq -d $d/upx -o /usr/local/bin/upx-${v}; done \
    && rm -r ./upx-*.tar.* ./upx-*linux \
    && true

# create default container user upx:upx 2000:2000
RUN useradd upx -U --uid 2000 --shell /bin/bash -m \
    && cd /home/upx && chmod 00700 . \
    # prepare ~/.cache, ~/.config and ~/.local for possible tmpfs mounts
    && mkdir -p .cache/tmp .local/bin src/upx \
    && for d in ccache fontconfig go-build mesa_shader_cache parallel tmp wine zig; do mkdir -p .cache/$d; done \
    && for d in fish git helix kak nvim; do mkdir -p .config/$d; done \
    && for d in bin include lib share state; do mkdir -p .local/$d; done \
    && ln -s .cache/wine .wine && ln -s .cache/tmp tmp \
    # symlink upx-stubtools
    && ln -s /usr/local/bin/bin-upx-20221212 .local/bin/bin-upx \
    # some convenience settings according to upx developer preferences
    && echo "alias path='echo PATH=\"\$PATH\"'" >> .bashrc \
    && echo "alias md=mkdir" >> .bashrc \
    && echo "alias rd=rmdir" >> .bashrc \
    && echo 'mdd() { mkdir "$1" && cd "$1"; }' >> .bashrc \
    && echo 'm() { make "$@"; }' >> .bashrc \
    && echo 'mm() { make -j8 "$@"; }' >> .bashrc \
    && echo 'tn() { time nice "$@"; }' >> .bashrc \
    && echo 'tnm() { time nice make "$@"; }' >> .bashrc \
    && echo 'tnmm() { time nice make -j8 "$@"; }' >> .bashrc \
    && echo 'source /usr/share/bash-completion/completions/make' >> .bashrc \
    && echo 'complete -F _make m mm tnm tnmm' >> .bashrc \
    && echo 'eval "$(zoxide init bash)"' >> .bashrc \
    && echo "alias zz=zi\nalias y=z\nalias yy=zi" >> .bashrc \
    && echo "alias cd..='cd ..'" >> .bashrc \
    && echo "alias ..='cd ..'" >> .bashrc \
    && echo "alias .='cd ..'" >> .bashrc \
    && chown -R upx:upx . \
    && true
USER upx
