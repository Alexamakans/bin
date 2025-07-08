#!/usr/bin/env bash

# Usage: ./setup-bin.sh projectname
set -e

if [ -z "$1" ]; then
    echo "Usage: $0 <projectname>"
    exit 1
fi

PROJ_NAME="$1"
DIR="src/$PROJ_NAME"

mkdir -p "$DIR"

cat > "$DIR/main.c" <<EOF
#include <stdio.h>

int main(void) {
    printf("Hello from $PROJ_NAME!\\n");
    return 0;
}
EOF

cat > "$DIR/CMakeLists.txt" <<EOF
# Build executable: $PROJ_NAME

add_executable($PROJ_NAME
    main.c
)

# === Linking custom libraries (uncomment and modify as needed) ===
# target_link_libraries($PROJ_NAME PRIVATE libA libB)

# === If you need include directories (rare in simple apps) ===
# target_include_directories($PROJ_NAME PRIVATE \${CMAKE_SOURCE_DIR}/libs/libA)
EOF

echo "Binary project '$PROJ_NAME' created in src/$PROJ_NAME/"

