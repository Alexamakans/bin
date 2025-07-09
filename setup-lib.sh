#!/usr/bin/env bash

# Usage: ./setup-lib.sh libname
set -e

if [ -z "$1" ]; then
    echo "Usage: $0 <libname>"
    exit 1
fi

LIB_NAME="$1"
DIR="libs/$LIB_NAME"

mkdir -p "$DIR"

cat > "$DIR/$LIB_NAME.c" <<EOF
#include "$LIB_NAME.h"

// TODO: Implement functions
EOF

cat > "$DIR/$LIB_NAME.h" <<EOF
#ifndef ${LIB_NAME^^}_H
#define ${LIB_NAME^^}_H

// TODO: Declare your library functions here

#endif // ${LIB_NAME^^}_H
EOF

cat > "$DIR/CMakeLists.txt" <<EOF
# Build static library: $LIB_NAME


file(GLOB_RECURSE SRC_FILES "*.c")

add_library($LIB_NAME STATIC \${SRC_FILES})

# Make headers accessible to projects that link this library
target_include_directories($LIB_NAME PUBLIC \${CMAKE_CURRENT_SOURCE_DIR})
EOF

echo "Library '$LIB_NAME' created in libs/$LIB_NAME/"
