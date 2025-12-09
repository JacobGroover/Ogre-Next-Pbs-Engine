#!/bin/bash
set -e

# ==========================================
# Detect OS & Hardware
# ==========================================
OS_NAME=$(uname -s)
ARCH=$(uname -m)

if [ "$OS_NAME" = "Darwin" ]; then
    CORES=$(sysctl -n hw.logicalcpu)
    echo "Detected macOS ($ARCH) with $CORES cores."
else
    CORES=$(nproc)
    echo "Detected Linux ($ARCH) with $CORES cores."
fi

# ==========================================
# Repos / Branches
# ==========================================
#ENGINE_REPO="https://github.com/JacobGroover/Ogre-Next-Pbs-Engine.git"
#OGRE_NEXT_DEPS_REPO="https://github.com/JacobGroover/ogre-next-deps.git"
#OGRE_NEXT_REPO="https://github.com/JacobGroover/ogre-next.git"

ENGINE_REPO="https://github.com/JacobGroover/Ogre-Next-Pbs-Engine.git"
OGRE_NEXT_DEPS_REPO="https://github.com/JacobGroover/ogre-next-deps.git"
OGRE_NEXT_REPO="https://github.com/JacobGroover/ogre-next.git"

ENGINE_BRANCH="main"
OGRE_BRANCH="master"

# ==========================================
# Check CMake
# ==========================================
echo "CHECK FOR CMAKE"
if ! command -v cmake >/dev/null 2>&1; then
    echo "CMake not found. Install it and re-run."
    exit 1
fi

echo "CMake detected: $(command -v cmake)"

# Detect whether generator is multi-config
GEN=$(cmake -G "" -LA 2>/dev/null | grep "CMAKE_GENERATOR" || true)
MULTI_CONFIG=0

case "$GEN" in
    *"Xcode"*|*"Visual Studio"*|*"Multi-Config"*)
        MULTI_CONFIG=1
        ;;
esac

# ==========================================
# Vulkan Check
# ==========================================
echo "CHECK FOR VULKAN"
FOUND_VULKAN=0

if [ -n "$VULKAN_SDK" ]; then
    echo "Vulkan SDK detected at $VULKAN_SDK"
    FOUND_VULKAN=1
else
    if [ "$OS_NAME" = "Linux" ]; then
        if ldconfig -p 2>/dev/null | grep -q "libvulkan"; then
            FOUND_VULKAN=1
        fi
    elif [ "$OS_NAME" = "Darwin" ]; then
        # MoltenVK locations
        if [ -f "/usr/local/lib/libMoltenVK.dylib" ] || \
           [ -f "/opt/homebrew/lib/libMoltenVK.dylib" ]; then
            FOUND_VULKAN=1
        fi
    fi
fi

if [ $FOUND_VULKAN -eq 0 ]; then
    echo "[WARNING] Vulkan not detected. Ogre-Next Vulkan RS will be skipped."
    echo "Press ENTER to continue..."
    read _
fi

# ==========================================
# wxWidgets Check
# ==========================================
echo "CHECK FOR WXWIDGETS"
if [ -n "$WXWIDGETS_ROOT" ]; then
    echo "wxWidgets detected at: $WXWIDGETS_ROOT"
elif command -v wx-config >/dev/null 2>&1; then
    echo "wxWidgets detected via wx-config: $(command -v wx-config)"
else
    echo "[ERROR] wxWidgets not found."
    exit 1
fi

# ==========================================
# Clone Engine
# ==========================================
if [ ! -d "Engine" ]; then
    echo "--- Cloning Engine ---"
    git clone --recurse-submodules --shallow-submodules "$ENGINE_REPO" Engine
else
    echo "--- Engine already exists ---"
fi

pushd Engine/Dependencies/Ogre >/dev/null

# ==========================================
# Clone ogre-next-deps
# ==========================================
if [ ! -d "ogre-next-deps" ]; then
    echo "--- Cloning ogre-next-deps ---"
    git clone --recurse-submodules --shallow-submodules "$OGRE_NEXT_DEPS_REPO" ogre-next-deps
else
    echo "--- ogre-next-deps already exists ---"
fi

# ==========================================
# Build ogre-next-deps (Debug + Release)
# ==========================================
pushd ogre-next-deps >/dev/null

for CONFIG in Debug Release; do
    mkdir -p "build-$CONFIG"
    pushd "build-$CONFIG" >/dev/null

    echo "--- Building ogre-next-deps ($CONFIG) ---"

    cmake -DCMAKE_BUILD_TYPE=$CONFIG \
          -DCMAKE_INSTALL_PREFIX="./ogredeps-$CONFIG" \
          ..

    if [ $MULTI_CONFIG -eq 1 ]; then
        cmake --build . --config $CONFIG -- -j$CORES
        cmake --build . --target install --config $CONFIG
    else
        cmake --build . -- -j$CORES
        cmake --install .
    fi

    popd >/dev/null
done

popd >/dev/null

# ==========================================
# Clone Ogre-Next
# ==========================================
if [ ! -d "ogre-next" ]; then
    echo "--- Cloning Ogre-Next master ---"
    git clone --branch "$OGRE_BRANCH" "$OGRE_NEXT_REPO" ogre-next
fi

pushd ogre-next >/dev/null

# ==========================================
# Build Ogre-Next (Debug + Release)
# ==========================================
mkdir -p build

for CONFIG in Debug Release; do
    # FIX 1: Update the Dependencies symlink for the current CONFIG
    # This ensures Release builds link against Release deps, and Debug against Debug deps.
    rm -f Dependencies
    if [ "$CONFIG" = "Debug" ]; then
        ln -s ../ogre-next-deps/build-Debug/ogredeps-Debug Dependencies
    else
        ln -s ../ogre-next-deps/build-Release/ogredeps-Release Dependencies
    fi

    # Create build directory
    mkdir -p "build/$CONFIG"
    pushd "build/$CONFIG" >/dev/null

    echo "--- Building Ogre-Next ($CONFIG) ---"

    # FIX 2: Install to './sdk' to avoid file locking, then copy back.
    # We use 'cp -a' to preserve symlinks (critical for shared libraries on Linux).
    cmake -DCMAKE_BUILD_TYPE=$CONFIG \
          -DOGRE_BUILD_COMPONENT_SCENE_FORMAT=1 \
          -DOGRE_BUILD_SAMPLES2=1 \
          -DOGRE_BUILD_TESTS=1 \
          -DOGRE_DEPENDENCIES_DIR="../../../ogre-next-deps/build-$CONFIG/ogredeps-$CONFIG" \
          -DCMAKE_INSTALL_PREFIX="./sdk" \
          ../../

    if [ $MULTI_CONFIG -eq 1 ]; then
        cmake --build . --config $CONFIG -- -j$CORES
        cmake --build . --target install --config $CONFIG
    else
        cmake --build . -- -j$CORES
        cmake --install .
    fi
    
    echo "--- Adjusting header/lib layout for Engine ($CONFIG) ---"
    
    # 1. Copy headers
    if [ -d "sdk/include/OGRE-Next" ]; then
        mkdir -p include
        cp -a sdk/include/OGRE-Next/* include/
    fi
    
    # 2. Copy libraries (libs) to the folder Engine expects
    if [ -d "sdk/lib" ]; then
         mkdir -p lib
         cp -a sdk/lib/* lib/
    fi

    # 3. Cleanup temp sdk folder
    rm -rf sdk

    popd >/dev/null # Exit build/$CONFIG
done

popd >/dev/null  # Exit ogre-next
popd >/dev/null  # Exit Dependencies/Ogre

# ==========================================
# Build Engine
# ==========================================
pushd Engine >/dev/null
mkdir -p build

for CONFIG in Debug Release; do
    echo "--- Building Engine ($CONFIG) ---"
    
    # Create separate build folders for each config to prevent
    # CMakeCache.txt variable collision (specifically OGRE_BINARIES)
    mkdir -p "build/$CONFIG"
    pushd "build/$CONFIG" >/dev/null

    cmake -DCMAKE_BUILD_TYPE=$CONFIG ../../
    
    if [ $MULTI_CONFIG -eq 1 ]; then
        cmake --build . --config $CONFIG -- -j$CORES
    else
        cmake --build . -- -j$CORES
    fi
    
    popd >/dev/null # Exit build/$CONFIG
done

echo ""
echo "==================================="
echo "Build Complete."
echo "==================================="
