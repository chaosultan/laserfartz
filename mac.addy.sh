#!/bin/bash

format_mac() {
    local mac=$(echo "$1" | tr -d ':-.' | tr '[:lower:]' '[:upper:]')
    local result="{"
    
    for ((i=0; i<${#mac}; i+=2)); do
        [ $i -gt 0 ] && result+=", "
        result+="0x${mac:$i:2}"
    done
    
    result+="}"
    echo "$result"
}

if [ $# -eq 0 ]; then
    echo "Usage: $0 <mac-address> [mac-address2] ..."
    echo "Example: $0 e8:3d:c1:8e:3e:a4 00:11:22:33:44:55"
    exit 1
fi

# Process all arguments
for mac in "$@"; do
    format_mac "$mac"
done
