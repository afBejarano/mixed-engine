#!/bin/bash

# Mixed Engine Setup Script
# This script sets up the project dependencies and builds the project

set -e  # Exit on any error

echo "🚀 Setting up Mixed Engine..."

# Check if Python 3 is available
if ! command -v python3 &> /dev/null; then
    echo "❌ Python 3 is required but not installed"
    exit 1
fi

# Check if Git is available
if ! command -v git &> /dev/null; then
    echo "❌ Git is required but not installed"
    exit 1
fi

# Check if CMake is available
if ! command -v cmake &> /dev/null; then
    echo "❌ CMake is required but not installed"
    exit 1
fi

echo "✅ Prerequisites check passed"

# Run bootstrap script
echo "📦 Downloading dependencies..."
cd deps
python3 bootstrap.py
cd ..

# Copy header-only libraries
echo "📋 Copying header files..."
python3 scripts/copy_headers.py

# Create build directory
echo "🔨 Setting up build directory..."
mkdir -p build
cd build

# Configure with CMake
echo "⚙️  Configuring with CMake..."
cmake .. -G "Unix Makefiles"

echo "✅ Setup completed successfully!"
echo ""
echo "To build the project, run:"
echo "  cd build"
echo "  make"
echo ""
echo "To run the application:"
echo "  ./MixedEngine" 