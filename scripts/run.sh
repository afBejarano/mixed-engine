#!/bin/bash

# Mixed Engine Run Script

set -e  # Exit on any error

echo "🚀 Running Mixed Engine..."

# Check if executable exists
if [ ! -f "build/MixedEngine" ]; then
    echo "❌ Executable not found. Run build.sh first."
    exit 1
fi

# Run the application
cd build
./MixedEngine 