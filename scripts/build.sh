#!/bin/bash

# Mixed Engine Build Script

set -e  # Exit on any error

echo "🔨 Building Mixed Engine..."

# Check if build directory exists
if [ ! -d "build" ]; then
    echo "❌ Build directory not found. Run setup.sh first."
    exit 1
fi

# Build the project
cd build
make -j$(nproc)

echo "✅ Build completed successfully!"
echo ""
echo "To run the application:"
echo "  ./MixedEngine" 