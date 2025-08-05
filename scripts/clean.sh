#!/bin/bash

# Mixed Engine Clean Script

echo "🧹 Cleaning Mixed Engine..."

# Remove build directory
if [ -d "build" ]; then
    echo "Removing build directory..."
    rm -rf build
fi

# Remove external dependencies
if [ -d "external/src" ]; then
    echo "Removing external dependencies..."
    rm -rf external/src
fi

if [ -d "external/archives" ]; then
    echo "Removing downloaded archives..."
    rm -rf external/archives
fi

# Remove bootstrap cache
if [ -f "deps/.bootstrap.json" ]; then
    echo "Removing bootstrap cache..."
    rm -f deps/.bootstrap.json
fi

echo "✅ Clean completed successfully!"
echo ""
echo "To rebuild the project:"
echo "  ./setup.sh" 