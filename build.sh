#!/usr/bin/env sh
set -eu

ROOT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
IMAGE=${LINNSTRUMENT_DOCKER_IMAGE:-linnstrument-firmware-build}
# Arduino CLI names artifacts after the sketch file (linnstrument-firmware.ino.bin);
# override build.project_name so they are named linnstrument-firmware.bin/.elf/.map.
SKETCH_NAME=$(basename -- "$ROOT_DIR")

docker build --tag "$IMAGE" "$ROOT_DIR"
docker run --rm \
    --user "$(id -u):$(id -g)" \
    --env HOME=/tmp \
    --env SKETCH_NAME="$SKETCH_NAME" \
    --volume "$ROOT_DIR:/workspace/linnstrument-firmware" \
    --workdir /workspace/linnstrument-firmware \
    "$IMAGE" /bin/sh -c '
        # Arduino CLI requires the sketch directory to match its main .ino file.
        exec arduino-cli --config-file /etc/arduino-cli.yaml compile \
            --fqbn arduino:sam:arduino_due_x \
            --build-path /workspace/linnstrument-firmware/build \
            --libraries /workspace/linnstrument-firmware/libraries \
            --output-dir /workspace/linnstrument-firmware/build \
            --build-property "build.project_name=$SKETCH_NAME" \
            /workspace/linnstrument-firmware
    '
