#! /usr/bin/env bash
## vim:set ts=4 sw=4 et:
set -e; set -o pipefail
argv0=$0; argv0abs="$(readlink -fn "$argv0")"; argv0dir="$(dirname "$argv0abs")"

# run an interactive shell in the image
# using a rootless Podman container

image=upx-stubtools-20210104-v5

flags=( -ti --read-only --rm )
flags+=( --cap-drop=all )           # drop all capabilities
flags+=( --network=none )           # no network needed
flags+=( -e TERM="$TERM" )          # pass $TERM
if [[ 1 == 1 ]]; then
    # run as user upx 2000:2000
    flags+=( --user 2000 )
    # map container user 0 to subuid-user 1, and map container user 2000 to current host user
    flags+=( --uidmap=0:1:1 --uidmap=2000:0:1 )
    # map container group 0 to subgid-group 1, and map container group 2000 to current host group
    flags+=( --gidmap=0:1:1 --gidmap=2000:0:1 )
    # NOTE: we mount the upx top-level directory read-write under /home/upx/src/upx
    # INFO: SELinux users *may* have to add ":z" to the volume mount flags; check the docs!
    flags+=( -v "${argv0dir}/../..:/home/upx/src/upx" )
    flags+=( -w /home/upx/src/upx ) # working directory
else
    # run as user root 0:0
    # ONLY FOR DEBUGGING THE IMAGE
    # map container user/group 0 to current host user/group
    flags+=( --user 0 )
fi

podman run "${flags[@]}" "$image" bash -l

# now we can rebuild the UPX stubs:
#   cd /home/upx/src/upx/src/stub
#   # make sure that git is clean:
#   git status .
#   # remove stub files and make sure that they got deleted:
#   make maintainer-clean
#   git status .
#   # rebuild
#   make all
#   # make sure that the stub files did rebuild correctly:
#   git status .
#   git diff .
