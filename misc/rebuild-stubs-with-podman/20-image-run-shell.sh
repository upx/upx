#! /usr/bin/env bash
## vim:set ts=4 sw=4 et:
set -e; set -o pipefail
argv0=$0; argv0abs="$(readlink -fn "$argv0")"; argv0dir="$(dirname "$argv0abs")"

# run an interactive shell in the image
# using a rootless Podman container

image=upx-stubtools-20210104-v1

flags=( -ti --read-only --rm )
flags+=( --cap-drop=all )
flags+=( --network=none )
flags+=( --user 2000 )
# map container user 0 to 1, and map container user 2000 to current host user
flags+=( --uidmap=0:1:1 --uidmap=2000:0:1 )
flags+=( -w /home/upx )
flags+=( -e TERM="$TERM" )
# NOTE: we mount the upx top-level directory read-write under /home/upx/src/upx
flags+=( -v "${argv0dir}/../..:/home/upx/src/upx:rw" )

podman run "${flags[@]}" "$image" bash -l

# now we can rebuild the UPX stubs:
#   cd /home/upx/src/upx/src/stub
#   # make sure that git is clean:
#   git status .
#   # remove stub files and make sure that they got deleted:
#   make clean
#   git status .
#   # rebuild
#   make all
#   # make sure that the stub files did rebuild correctly:
#   git status .
#   git diff .
