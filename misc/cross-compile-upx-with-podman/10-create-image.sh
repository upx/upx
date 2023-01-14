#! /usr/bin/env bash
## vim:set ts=4 sw=4 et:
set -e; set -o pipefail
argv0=$0; argv0abs="$(readlink -fn "$argv0")"; argv0dir="$(dirname "$argv0abs")"

# create the image from Dockerfile
# using a rootless Podman container

# NOTE: this image is based on rebuild-stubs-with-upx/upx-stubtools-20221212-v3,
#   so you have to create that image first
# WARNING: we install many packages, so the resulting image needs A LOT of disk space!
image=upx-cross-compile-20230115-v1

podman build -t "$image" -f "$argv0dir/Dockerfile" "$argv0dir"

podman image list "$image"
podman image tree "$image"
