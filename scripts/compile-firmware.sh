#!/bin/sh

set -eu

project_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
build_name=${1:-current}
build_dir="$project_dir/build/$build_name"
stage_root=$(mktemp -d /private/tmp/linnstrument-build.XXXXXX)
stage_dir="$stage_root/linnstrument-firmware"

cleanup() {
  rm -rf "$stage_root"
}
trap cleanup EXIT INT TERM

mkdir -p "$stage_dir" "$build_dir"
cp "$project_dir"/*.ino "$project_dir"/*.h "$stage_dir"/

env \
  ARDUINO_DIRECTORIES_DATA="$project_dir/.arduino-data" \
  ARDUINO_DIRECTORIES_DOWNLOADS="$project_dir/.arduino-data/staging" \
  ARDUINO_DIRECTORIES_USER="$project_dir" \
  "$project_dir/.tools/arduino-cli/arduino-cli" compile \
    --warnings all \
    --fqbn arduino:sam:arduino_due_x \
    --build-path "$build_dir" \
    --export-binaries \
    "$stage_dir"
