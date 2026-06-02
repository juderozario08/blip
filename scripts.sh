#!/bin/bash

# 1. Configure and Build
if ! cmake -S . -B build; then
    echo "CMake configuration failed!"
    exit 1
fi

if ! cmake --build build; then
    echo "Build failed!"
    exit 1
fi

# 2. Setup link
if [ ! -f ./compile_commands.json ]; then
    ln -s build/compile_commands.json .
fi

# 3. Execution logic
TARGET="./build/Blip"
if [ "$1" == "test" ]; then
    TARGET="./build/BlipTests"
elif [ "$1" == "dev" ]; then
    TARGET="./build/BlipDev"
fi

if [ -f "$TARGET" ]; then
    echo -e "\n--- Starting $TARGET ---"
    $TARGET
else
    echo "Target $TARGET not found!"
    exit 1
fi
