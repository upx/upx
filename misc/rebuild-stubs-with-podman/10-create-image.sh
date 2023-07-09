#! /usr/bin/env bash
## vim:set ts=4 sw=4 et:
set -e; set -o pipefail
argv0=$0; argv0abs="$(readlink -fn "$argv0")"; argv0dir="$(dirname "$argv0abs")"

# create the image from Dockerfile
# using a rootless Podman container

image=upx-stubtools-20221212-v6
[[ $1 == --print-image ]] && echo "$image" && exit 0

podman build -t "$image" -f "$argv0dir/Dockerfile" "$argv0dir"

podman image list "$image"
echo
podman image tree "$image"
