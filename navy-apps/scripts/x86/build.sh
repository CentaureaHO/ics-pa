#!/bin/bash

# Function to normalize paths to Windows format
normalize_path() {
    local path="$1"
    if [[ "$path" =~ ^/([a-zA-Z])/ ]]; then
        path="${BASH_REMATCH[1]}:${path:2}"
    fi

    echo "$path" | sed 's|//|/|g'
}

DIR=$(dirname "$0")
DEST=$1
shift

ENTRY_S="$DIR/entry.S"
ENTRY_O="$DIR/entry.o"
gcc -m32 -I$(dirname "$DIR")/libs/libos/include -c "$ENTRY_S" -o "$ENTRY_O"

OBJS_NORM=""
LIBS_NORM=""

for arg in "$@"; do
    norm_arg=$(normalize_path "$arg")
    if [[ "$norm_arg" == *.o ]]; then
        OBJS_NORM="$OBJS_NORM $norm_arg"
    else
        LIBS_NORM="$LIBS_NORM $norm_arg"
    fi
done

DIR_NORM=$(normalize_path "$DIR")
DEST_NORM=$(normalize_path "$DEST")
ENTRY_O_NORM=$(normalize_path "$ENTRY_O")

# 1. Link to PE file (.exe)
ld -m i386pe --gc-sections -T "$DIR_NORM/loader.ld" -e _start -o "$DEST_NORM.exe" "$ENTRY_O_NORM" $OBJS_NORM --start-group $LIBS_NORM --end-group
# 2. Generate disassembly from the PE file.
objdump -d "$DEST_NORM.exe" > "$DEST_NORM.txt"
# 3. Generate a compact binary image.
objcopy -I pe-i386 -O binary "$DEST_NORM.exe" "$DEST_NORM"

rm -f "$DEST_NORM.exe" "$ENTRY_O"
