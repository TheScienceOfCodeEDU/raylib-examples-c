#!/bin/bash

# Patch image adding libc
YAML_FILE="tmp/rel-linux/AppImageBuilder.yml"
# Check if the YAML file exists
if [ ! -f "$YAML_FILE" ]; then
    echo "Error: $YAML_FILE not found"
    exit 1
fi
# Add the - /lib64/libc.so.6 line under files include, if not already present
if ! grep -q "/lib64/libc.so.6" "$YAML_FILE"; then
    # Use sed to insert '- /lib64/libc.so.6' before '- lib64/ld-linux-x86-64.so.2 in the include section
    sed -i '/^  files:$/,/^  exclude:$/{/^[[:space:]]*include:/,/^[[:space:]]*- \/lib64\/ld-linux-x86-64\.so\.2/{s/^[[:space:]]*- \/lib64\/ld-linux-x86-64\.so\.2/    - \/lib64\/libc.so.6\n    - \/lib64\/ld-linux-x86-64.so.2/}}' "$YAML_FILE"
    echo "[SCRIPT]: Successfully added 'files include: - /lib64/libc.so.6' to $YAML_FILE"
fi

