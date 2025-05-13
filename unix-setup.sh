#!/usr/bin/env bash

set -e
VCPKG_DIR="vcpkg"
TOOLCHAIN_FILE="$VCPKG_DIR/scripts/buildsystems/vcpkg.cmake"

# Clone the vcpkg if not already
if [ ! -d "$VCPKG_DIR" ]; then
  git clone https://github.com/microsoft/vcpkg.git "$VCPKG_DIR"
  "$VCPKG_DIR/bootstrap-vcpkg.sh"
fi

#  Install dependencies (using manifest mode)
"$VCPKG_DIR/vcpkg" install

# Generate build files
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_FILE"

echo "Setup complete. Run 'cmake --build build' to compile."