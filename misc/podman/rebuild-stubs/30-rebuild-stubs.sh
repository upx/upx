#! /usr/bin/env bash
## vim:set ts=4 sw=4 et:
set -e; set -o pipefail
argv0=$0; argv0abs="$(readlink -fn "$argv0")"; argv0dir="$(dirname "$argv0abs")"

# rebuild stubs from source code
#   (also see 20-image-run-shell.sh for more container usage examples)
# using a rootless Podman container
# Copyright (C) Markus Franz Xaver Johannes Oberhumer

image="$("$argv0dir/10-create-image.sh" --print-image)"

flags=( --read-only --rm --pull=never )
flags+=( --cap-drop=all )               # drop all capabilities
flags+=( --network=none )               # no network needed
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
fi

podman run "${flags[@]}" "$image" bash -c $'
set -ex; set -o pipefail
cd /home/upx/src/upx
# check whitespace
[[ -d .git ]] && bash ./misc/scripts/check_whitespace_git.sh
# clang-format
make -C src clang-format
# rebuild docs
make -C doc clean all
# rebuild stubs
cd /home/upx/src/upx/src/stub
make maintainer-clean extra-maintainer-clean
ls -ld ./*.h || true
rm -rf ./*.h
rm -rf ./*.[!em]*
ls -la
git status . || true # make sure the stub files got deleted
make extra-all all
echo "===== Rebuilt stubs. All done. ====="
exit 0
'
