#!/bin/bash

dirs=("include" "src")
patterns=("*.cpp" "*.h" "*.hpp" "*.c" "*.cxx" "*.hxx" "*.cmake" "*.make" "*.json")

for dir in "${dirs[@]}"; do
    if [ -d "$dir" ]; then
        while IFS= read -r -d '' file; do
            echo "$file:"
            cat "$file"
            echo ""
        done < <(find "$dir" -type f \( \
            -name "*.cpp" -o \
            -name "*.h" -o \
            -name "*.hpp" -o \
            -name "*.c" -o \
            -name "*.cxx" -o \
            -name "*.hxx" -o \
            -name "*.cmake" -o \
            -name "*.make" -o \
            -name "*.json" \) -print0)
    fi
done