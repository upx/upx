#! /usr/bin/env bash
## vim:set ts=4 sw=4 et:
set -e; set -o pipefail
argv0=$0; argv0abs="$(readlink -fn "$argv0")"; argv0dir="$(dirname "$argv0abs")"

# run an interactive shell in the image
# using a rootless Podman container

image="$("$argv0dir/10-create-image.sh" --print-image)"

flags=( --read-only --rm --pull=never )
flags+=( --cap-drop=all )               # drop all capabilities
flags+=( --network=none )               # no network needed
flags+=( -ti -e TERM="$TERM" )          # allocate an interactive pseudo-tty and pass $TERM
if [[ 1 == 1 ]]; then
    # run as user upx 2000:2000
    flags+=( --user 2000 )
    # map container users 0..999 to subuid-users 1..1000, and map container user 2000 to current host user
    flags+=( --uidmap=0:1:1000 --uidmap=2000:0:1 )
    # map container groups 0..999 to subgid-groups 1..1000, and map container group 2000 to current host group
    flags+=( --gidmap=0:1:1000 --gidmap=2000:0:1 )
    # NOTE: we mount the upx top-level directory read-write under /home/upx/src/upx
    # INFO: SELinux users *may* have to add ":z" to the volume mount flags; check the docs!
    flags+=( -v "${argv0dir}/../../..:/home/upx/src/upx" )
    flags+=( -w /home/upx/src/upx )     # set working directory
    flags+=( --tmpfs /home/upx/.cache:rw,exec ) # mount a writeable tmpfs
    flags+=( --tmpfs /home/upx/.local:rw,exec ) # mount a writeable tmpfs
else
    # run as user root 0:0
    # ONLY FOR DEBUGGING THE IMAGE
    # map container user/group 0 to current host user/group
    flags+=( --user 0 )
fi

if [[ $# == 0 ]]; then
    podman run "${flags[@]}" "$image" bash -l
else
    podman run "${flags[@]}" "$image" "$@"
fi

# now we can cross-compile UPX for Windows:
#   cd /home/upx/src/upx
#   rm -rf ./build/extra/cross-windows-mingw64/release
#   make build/extra/cross-windows-mingw64/release

# and we can run the clang Static Analyzer (scan-build)
#   cd /home/upx/src/upx
#   rm -rf ./build/extra/scan-build/release
#   make build/extra/scan-build/release

# and lots of other cross-compilers are installed; see "ls /usr/bin/*g++*"

# and finally see
#   ./misc/podman/cross-compile-upx-ubuntu/build-all-inside-container.sh
# after running that script we can do cool things like:
#   cd /home/upx/src/upx/build/cross-compile-upx-ubuntu2204/alpha-linux-gnu/debug
#   qemu-alpha -L /usr/alpha-linux-gnu ./upx --version
#   cd /home/upx/src/upx/build/cross-compile-upx-ubuntu2204/hppa-linux-gnu/debug
#   qemu-hppa -L /usr/hppa-linux-gnu ./upx --version
# (similar for many other architectures/builds)
