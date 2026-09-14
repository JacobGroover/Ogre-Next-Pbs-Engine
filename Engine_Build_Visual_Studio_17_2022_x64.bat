
@echo off
SETLOCAL

set ENGINE_REPO=https://github.com/JacobGroover/Ogre-Next-Pbs-Engine.git
set OGRE_NEXT_DEPS_REPO=https://github.com/JacobGroover/ogre-next-deps.git
set OGRE_NEXT_REPO=https://github.com/JacobGroover/ogre-next.git
set OGRE_BRANCH_NAME=master

set GENERATOR="Visual Studio 17 2022"
set PLATFORM=x64

echo CHECK FOR CMAKE
set CMAKE_BIN_x86="C:\Program Files (x86)\CMake\bin\cmake.exe"
set CMAKE_BIN_x64="C:\Program Files\CMake\bin\cmake.exe"
IF EXIST %CMAKE_BIN_x64% (
	echo CMake 64-bit detected
	set CMAKE_BIN=%CMAKE_BIN_x64%
) ELSE (
	IF EXIST %CMAKE_BIN_x86% (
		echo CMake 32-bit detected
		set CMAKE_BIN=%CMAKE_BIN_x86%
	) ELSE (
		echo Cannot detect either %CMAKE_BIN_x86% or
		echo %CMAKE_BIN_x64% make sure CMake is installed
		pause
		EXIT /B 1
	)
)
echo Using CMake at %CMAKE_BIN%

echo CHECK FOR VULKAN SDK
IF NOT DEFINED VULKAN_SDK (
	echo [WARNING] VULKAN_SDK environment variable not found.
	echo Ogre-Next will likely skip building the Vulkan RenderSystem.
	echo Press any key to continue without Vulkan.
	pause
) ELSE (
	echo Vulkan SDK detected at %VULKAN_SDK%
)


IF NOT EXIST Engine (
	mkdir Engine
	echo --- Cloning Engine ---
	call git clone --recurse-submodules --shallow-submodules %ENGINE_REPO% Engine
) ELSE (
	echo --- Engine repo detected. Cloning skipped ---
)
cd Engine/Dependencies/Ogre

IF NOT EXIST ogre-next-deps (
	mkdir ogre-next-deps
	echo --- Cloning ogre-next-deps ---
	call git clone --recurse-submodules --shallow-submodules %OGRE_NEXT_DEPS_REPO%
) ELSE (
	echo --- ogre-next-deps repo detected. Cloning skipped ---
)
cd ogre-next-deps

mkdir build
cd build
echo --- Building ogre-next-deps ---
%CMAKE_BIN% -G %GENERATOR% -A %PLATFORM% ..
%CMAKE_BIN% --build . --config Debug
%CMAKE_BIN% --build . --target install --config Debug
%CMAKE_BIN% --build . --config Release
%CMAKE_BIN% --build . --target install --config Release

cd ../../
IF NOT EXIST ogre-next (
	echo --- Cloning Ogre master ---
	call git clone --branch %OGRE_BRANCH_NAME% %OGRE_NEXT_REPO%
)
cd ogre-next
IF NOT EXIST Dependencies (
	mklink /D Dependencies ..\ogre-next-deps\build\ogredeps
	IF ERRORLEVEL 1 (
		echo Failed to create Dependency directory symlink. Run the script as Administrator.
		EXIT /B 1
	)
)
mkdir build
cd build
echo --- Running CMake configure ---
REM All %CMAKE_BIN% -D OGRE_CONFIG_THREAD_PROVIDER=0 -D OGRE_CONFIG_THREADS=0 -D OGRE_BUILD_COMPONENT_SCENE_FORMAT=1 -D OGRE_BUILD_SAMPLES2=1 -D OGRE_BUILD_TESTS=1 -D OGRE_DEPENDENCIES_DIR=..\..\ogre-next-deps\build\ogredeps -G %GENERATOR% -A %PLATFORM% ..
%CMAKE_BIN% -D OGRE_DEPENDENCIES_DIR=..\..\ogre-next-deps\build\ogredeps -G %GENERATOR% -A %PLATFORM% ..
echo --- Building Ogre (Debug) ---
%CMAKE_BIN% --build . --config Debug
%CMAKE_BIN% --build . --target install --config Debug
echo --- Building Ogre (Release) ---
%CMAKE_BIN% --build . --config Release
%CMAKE_BIN% --build . --target install --config Release

cd ../../../../

mkdir build
cd build
echo --- Building Engine ---
%CMAKE_BIN% -G %GENERATOR% -A %PLATFORM% ..
%CMAKE_BIN% --build . --config Debug

REM (Add the following line back when ready to package the game engine for distribution)
REM %CMAKE_BIN% --build . --target install --config Debug

%CMAKE_BIN% --build . --config Release

REM (Add the following line back when ready to package the game engine for distribution)
REM %CMAKE_BIN% --build . --target install --config Release


echo.
echo ===================================
echo Build Complete.
echo ===================================
ENDLOCAL
pause