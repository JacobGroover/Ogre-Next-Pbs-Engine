#!/bin/bash
set -e

# ==========================================
# Detect OS & Hardware
# ==========================================
OS_NAME=$(uname -s)
ARCH=$(uname -m)

# Detect CPU Cores for parallel build
if [ "$OS_NAME" = "Darwin" ]; then
    # macOS command
    CORES=$(sysctl -n hw.logicalcpu)
    echo "Detected macOS ($ARCH) with $CORES cores."
else
    # Linux command
    CORES=$(nproc)
    echo "Detected Linux ($ARCH) with $CORES cores."
fi

# ==========================================
# Configuration
# ==========================================
ENGINE_BRANCH_NAME="main"
OGRE_NEXT_DEPS_REPO="https://github.com/JacobGroover/ogre-next-deps.git"
OGRE_NEXT_REPO="https://github.com/JacobGroover/ogre-next.git"
OGRE_BRANCH_NAME="master"

# ==========================================
# Check for CMake
# ==========================================
echo "CHECK FOR CMAKE"
if ! command -v cmake &> /dev/null; then
    echo "CMake not found. Please install CMake."
    exit 1
fi
echo "CMake detected at $(command -v cmake)"

# ==========================================
# Check for Vulkan
# ==========================================
echo "CHECK FOR VULKAN SDK"
if [ -z "$VULKAN_SDK" ]; then
    echo "[WARNING] VULKAN_SDK environment variable not found."
    
    # OS-Specific check for system libraries
    FOUND_VULKAN=0
    if [ "$OS_NAME" = "Linux" ]; then
        if ldconfig -p 2>/dev/null | grep -q libvulkan; then
            FOUND_VULKAN=1
        fi
    elif [ "$OS_NAME" = "Darwin" ]; then
        # On macOS, check common homebrew or framework locations
        if [ -f "/usr/local/lib/libvulkan.dylib" ] || [ -f "/opt/homebrew/lib/libvulkan.dylib" ]; then
            FOUND_VULKAN=1
        fi
    fi

    if [ $FOUND_VULKAN -eq 1 ]; then
        echo "Vulkan library found in system paths."
    else
        echo "Ogre-Next will likely skip building the Vulkan RenderSystem."
        # Use simple read for compatibility
        echo "Press ENTER to continue without Vulkan..."
        read ignore
    fi
else
    echo "Vulkan SDK detected at $VULKAN_SDK"
fi

# ==========================================
# Check for wxWidgets
# ==========================================
echo "CHECK FOR WXWIDGETS"
if [ -n "$WXWIDGETS_ROOT" ]; then
    echo "wxWidgets detected via Environment Variable at: $WXWIDGETS_ROOT"
elif command -v wx-config &> /dev/null; then
    echo "wxWidgets detected via wx-config at: $(command -v wx-config)"
else
    echo ""
    echo "[ERROR] wxWidgets not found."
    echo "The Editor requires wxWidgets."
    echo "  - Linux: sudo apt install libwxgtk3.0-gtk3-dev"
    echo "  - macOS: brew install wxwidgets"
    echo ""
    exit 1
fi

# ==========================================
# Engine Setup
# ==========================================
if [ ! -d "Engine" ]; then
    mkdir -p Engine
    echo "--- Cloning Engine ---"
    git clone --recurse-submodules --shallow-submodules https://github.com/JacobGroover/Ogre-Next-Pbs-Engine.git Engine
else
    echo "--- Engine repo detected. Cloning skipped ---"
fi

cd Engine/Dependencies/Ogre

# ==========================================
# Ogre-Next-Deps Setup & Build
# ==========================================
if [ ! -d "ogre-next-deps" ]; then
    mkdir -p ogre-next-deps
    echo "--- Cloning ogre-next-deps ---"
    git clone --recurse-submodules --shallow-submodules $OGRE_NEXT_DEPS_REPO ogre-next-deps
else
    echo "--- ogre-next-deps repo detected. Cloning skipped ---"
fi

cd ogre-next-deps
mkdir -p build
cd build

echo "--- Building ogre-next-deps ---"

# Build Debug
echo "--- Configuring & Building Deps (Debug) ---"
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX="./ogredeps" ..
cmake --build . --config Debug -- -j$CORES
cmake --build . --target install --config Debug

# Build Release
echo "--- Configuring & Building Deps (Release) ---"
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="./ogredeps" ..
cmake --build . --config Release -- -j$CORES
cmake --build . --target install --config Release

cd ../../

# ==========================================
# Ogre-Next Setup & Build
# ==========================================
if [ ! -d "ogre-next" ]; then
    echo "--- Cloning Ogre master ---"
    git clone --branch $OGRE_BRANCH_NAME $OGRE_NEXT_REPO ogre-next
fi

cd ogre-next

# Create Symlink
if [ ! -e "Dependencies" ]; then
    echo "Creating Dependencies symlink..."
    ln -s ../ogre-next-deps/build/ogredeps Dependencies
fi

mkdir -p build
cd build
echo "--- Running CMake configure for Ogre ---"

# Build Debug
echo "--- Building Ogre (Debug) ---"
cmake -DOGRE_DEPENDENCIES_DIR=../../ogre-next-deps/build/ogredeps \
      -DCMAKE_BUILD_TYPE=Debug \
      -DOGRE_BUILD_COMPONENT_SCENE_FORMAT=1 \
      -DOGRE_BUILD_SAMPLES2=1 \
      -DOGRE_BUILD_TESTS=1 \
      ..
cmake --build . --config Debug -- -j$CORES
cmake --build . --target install --config Debug

# Build Release
echo "--- Building Ogre (Release) ---"
cmake -DOGRE_DEPENDENCIES_DIR=../../ogre-next-deps/build/ogredeps \
      -DCMAKE_BUILD_TYPE=Release \
      -DOGRE_BUILD_COMPONENT_SCENE_FORMAT=1 \
      -DOGRE_BUILD_SAMPLES2=1 \
      -DOGRE_BUILD_TESTS=1 \
      ..
cmake --build . --config Release -- -j$CORES
cmake --build . --target install --config Release

cd ../../../../

# ==========================================
# Engine Build
# ==========================================
mkdir -p build
cd build
echo "--- Building Engine ---"

# Build Debug
echo "--- Building Engine (Debug) ---"
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build . --config Debug -- -j$CORES

# Build Release
echo "--- Building Engine (Release) ---"
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config Release -- -j$CORES

echo ""
echo "==================================="
echo "Build Complete."
echo "==================================="
