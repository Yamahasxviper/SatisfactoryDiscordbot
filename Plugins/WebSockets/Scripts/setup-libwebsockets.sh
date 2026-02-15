#!/bin/bash
# Setup script for libwebsockets on Linux/Mac
# This script downloads and builds libwebsockets for the WebSockets plugin

set -e  # Exit on error

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PLUGIN_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
THIRDPARTY_DIR="$PLUGIN_DIR/ThirdParty/libwebsockets"
BUILD_DIR="/tmp/libwebsockets-build-$$"

LIBWEBSOCKETS_VERSION="v4.3.2"
LIBWEBSOCKETS_REPO="https://github.com/warmcat/libwebsockets.git"

echo "========================================="
echo "libwebsockets Auto-Setup Script"
echo "========================================="
echo "Plugin Directory: $PLUGIN_DIR"
echo "ThirdParty Directory: $THIRDPARTY_DIR"
echo "Build Directory: $BUILD_DIR"
echo ""

# Check if already set up
if [ -f "$THIRDPARTY_DIR/include/libwebsockets.h" ]; then
    echo "✓ libwebsockets already set up (found libwebsockets.h)"
    
    # Check for library files
    if [ "$(uname)" == "Linux" ]; then
        if [ -f "$THIRDPARTY_DIR/lib/Linux/x86_64-unknown-linux-gnu/libwebsockets.a" ]; then
            echo "✓ Linux library found"
            exit 0
        fi
    elif [ "$(uname)" == "Darwin" ]; then
        if [ -f "$THIRDPARTY_DIR/lib/Mac/libwebsockets.a" ]; then
            echo "✓ macOS library found"
            exit 0
        fi
    fi
fi

echo "⚠ libwebsockets not found or incomplete, building from source..."
echo ""

# Check for required tools
if ! command -v git &> /dev/null; then
    echo "Error: git is required but not installed"
    exit 1
fi

if ! command -v cmake &> /dev/null; then
    echo "Error: cmake is required but not installed"
    exit 1
fi

if ! command -v make &> /dev/null; then
    echo "Error: make is required but not installed"
    exit 1
fi

# Create build directory
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Clone libwebsockets
echo "→ Cloning libwebsockets $LIBWEBSOCKETS_VERSION..."
git clone --depth 1 --branch "$LIBWEBSOCKETS_VERSION" "$LIBWEBSOCKETS_REPO" libwebsockets
cd libwebsockets

# Create build directory
mkdir build
cd build

# Configure CMake
echo "→ Configuring build with CMake..."
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DLWS_WITH_SHARED=OFF \
    -DLWS_WITH_STATIC=ON \
    -DLWS_WITH_SSL=ON \
    -DLWS_WITH_ZLIB=ON \
    -DLWS_IPV6=ON \
    -DLWS_WITHOUT_TESTAPPS=ON \
    -DLWS_WITHOUT_TEST_SERVER=ON \
    -DLWS_WITHOUT_TEST_CLIENT=ON

# Build
echo "→ Building libwebsockets..."
make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

# Create target directories
echo "→ Creating target directories..."
mkdir -p "$THIRDPARTY_DIR/include"

if [ "$(uname)" == "Linux" ]; then
    mkdir -p "$THIRDPARTY_DIR/lib/Linux/x86_64-unknown-linux-gnu"
    LIB_TARGET="$THIRDPARTY_DIR/lib/Linux/x86_64-unknown-linux-gnu"
elif [ "$(uname)" == "Darwin" ]; then
    mkdir -p "$THIRDPARTY_DIR/lib/Mac"
    LIB_TARGET="$THIRDPARTY_DIR/lib/Mac"
else
    echo "Error: Unsupported platform $(uname)"
    exit 1
fi

# Copy headers
echo "→ Copying headers..."
cp -r ../include/*.h "$THIRDPARTY_DIR/include/" 2>/dev/null || true
cp ../include/libwebsockets.h "$THIRDPARTY_DIR/include/"

# Copy library
echo "→ Copying library..."
cp lib/libwebsockets.a "$LIB_TARGET/"

# Clean up
echo "→ Cleaning up build directory..."
cd /
rm -rf "$BUILD_DIR"

echo ""
echo "========================================="
echo "✓ libwebsockets setup complete!"
echo "========================================="
echo "Headers: $THIRDPARTY_DIR/include/"
echo "Library: $LIB_TARGET/"
echo ""

exit 0
