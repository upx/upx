#! /usr/bin/env bash
## vim:set ts=4 sw=4 et:
set -e; set -o pipefail
argv0=$0; argv0abs="$(readlink -fn "$argv0")"; argv0dir="$(dirname "$argv0abs")"

# list all system packages that are installed in the image
# using a rootless Podman container

image="$("$argv0dir/10-create-image.sh" --print-image)"

podman image list "$image"
echo
podman image tree "$image"

echo 'Packages:'
flags=( --read-only --rm --pull=never )
flags+=( --cap-drop=all )               # drop all capabilities
flags+=( --network=none )               # no network needed
podman run "${flags[@]}" "$image" bash -c $'apk info -v | sed \'s/ *$//\' | LC_ALL=C sort'
