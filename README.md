
# Engine

Engine is a custom game engine built on top of [Ogre-next](https://github.com/OGRECave/ogre-next).  
It provides a modular framework for rendering, input, and game state management, with a focus on clarity and extensibility.

---

## Setup (For Windows)

**=== To build Ogre-next ===**

- Install python 3.11 https://www.python.org/downloads/ (or use chocolatey or whatever method you prefer, but newer than 3.11 is not recommended, as some packages may have additional dependency issues later in development)

- Install CMake (preferably with CMake-GUI) Version 3.29.3 from https://cmake.org/files/v3.29/

- Install Vulkan SDK https://vulkan.lunarg.com/ 

- Navigate to the root directory of a drive (close to root helps with shaderc file path length limitations later, I can't guarantee good results if you place the project elsewhere)

- Clone this repository with submodules and run cmake to build the project: 
  - git clone --recurse-submodules https://github.com/youruser/Engine.git
  - cd Engine
  - cmake -S . -B build
  - cmake --build build


## Updating

**=== To pull updates to Ogre-next ===**

- As of the time of this writing, the instructions for setting up ogre-next did not work without significant modification, so there are likely to be many merge conflicts when attempting the process below:

- cd Dependencies/Ogre/ogre-next
- git pull origin master
- cd ../..
- git add Dependencies/Ogre/ogre-next
- git commit -m "Update ogre-next"

---

- Run the ogre build script (build_ogre_Visual_Studio_17_2022_x64.bat) to save time and insure proper cloning of subdirectories, etc. (There are some errors in their script and files, which I will rectify later. This leads to... all of the rest of this setup guide)

- Replace the CmakeLists.txt in the ogre-next directory with the CMakeLists.txt found in this repository under Engine\Dependencies\Ogre\ogre-next\CMakeLists.txt (see Ogre-next_CmakeListsDefaults.md for some context, located in \Engine)

- Move or copy paste the Ogre directory with its contained ogre-next and ogre-next-deps directories to wherever you plan to keep Engine

- Open CMake-gui
  - Point source directory at .../Engine/Ogre/ogre-next-deps directory
  
  - Point binary build directory at .../Engine/Ogre/ogre-next-deps/build directory (make it if it is not there yet)
  
  - Click Configure button, choose Visual Studio 17 2022

  - Click Configure button again until options are not highlighted red (red indicates new options, some setups do this in steps)
  
  - Choose desired build options (the defaults should be fine here since they are set in the CMakeLists.txt in .../Engine/Ogre/ogre-next-deps)

  - Generate

- Navigate to ...\Project\Ogre\ogre-next-deps\build and open OGRE-Next.sln in Visual Studio (or open it from CMake-gui)
  - Choose Debug and x64 at the top

    - Right click on the Solution in the Solution explorer
      - Select the build option to build all of the projects in the Solution
      - Should see 9 succeeded, 1 skipped. These 9 are dependencies used by ogre-next. The 1 skipped is the INSTALL project (See below)

    - Still inside of the Solution explorer, right click on the INSTALL project inside CMakePredefinedTargets folder
      - Select the build option to build the INSTALL project. This will move built binaries and headers from the other projects to the appropriate places for them to be found by ogre-next


- Open CMake-gui
  - Point source directory at .../Engine/Ogre/ogre-next directory
  
  - Point binary build directory at .../Engine/Ogre/ogre-next/build directory
  
  - Click Configure button, choose Visual Studio 17 2022

   - Click Configure button again until options are not highlighted red (red indicates new options, some setups do this in steps)
  
  - Choose desired build options (the defaults should be fine here since they are set in the CMakeLists.txt in .../Engine/Ogre/ogre-next)

  - Generate

- Navigate to ...\Project\Ogre\ogre-next\build and open OGRE-Next.sln in Visual Studio (or open it from CMake-gui)
  - Choose Debug and x64 at the top

  - Right click on the Solution in the Solution explorer
    - Select the build option to build all of the projects in the Solution

 **=== To build EmptyProject into Engine ===**

- Copy the EmptyProject directory in ...\Project\Dependencies\Ogre\Samples\2.0\Tutorials\EmptyProject and paste it to ...\Project

- Rename the newly pasted EmptyProject directory to EngineProject

- Copy ogre-next and ogre-next-deps directorys from ...\Project\Ogre and paste them in ...\Project\Engine\Dependencies (You will need to make the Dependencies directory here)

- Rename ogre-next directory you just pasted to Ogre

- Copy the contents of ogre-next-deps to the Dependencies directory in the newly-renamed Ogre directory ...\Project\Engine\Dependencies\Ogre\Dependencies

- Create a build directory at ...\Projects\Engine\build

- Add ...\Project\Engine\Dependencies\ogre-next-deps\build\ogredeps\bin\Debug to path environment (for SDL2.dll runtime binary and some other runtime binaries)

- Add ...\Project\Engine\Dependencies\ogre-next-deps\build\ogredeps\lib to path environment for the rest of the SDL2 libraries

- Open CMake-gui
  - Point source directory at ...\Projects\Engine directory
  
  - Point binary build directory at ...\Projects\Engine\build directory
  
  - Click Configure button, choose Visual Studio 17 2022
  
  - Choose desired build options

  - Generate (first click configure until no options show red, to ensure you have covered them all)

- Navigate to ...\Project\Engine\build and open Engine.sln in Visual Studio
  - Choose Debug and x64 at the top

  - Right click on the Engine solution in the Solution explorer
    - Select the build option to build the projects in the Solution

- The .exe will be in ...\Project\Engine\bin\Debug

- pull repo from GitHub, rebuild as needed

---

## Project Structure


- Engine/
  - Assets/
    - Hlms/
    - Materials/
    - Shaders/
    - Models/
    - Textures/
    - Fonts/
  - Config/
    - resources2.cfg.in
    - plugins.cfg.in
  - Scripts/
    - build_win.bat
    - deploy_assets.py
  - Source/
    - include/
    - src/
  - CMake/
  - bin/
    - Debug/
    - Release/
    - Data/
      - Hlms/
      - Materials/
      - Textures/
      - resources2.cfg
  - build/
  - Dependencies/
  - CMakeLists.txt



### Source Code
- **Source/**  
  Contains all engine and game code (`include/` headers and `src/` implementation files).

- **CMake/**  
  Custom CMake modules and toolchain files used to build the engine.  

---

### Assets
All *source* game and engine assets live here. These files are tracked in Git, unlike the runtime `bin/Data/` folder which only holds **deployment copies**.  

Currently empty, but added for structure since it will likely be needed later.  

Subfolders:
- **Assets/Hlms/** -> HLMS shader template sources (`.glsl`, `.hlsl`, `.metal`).  
- **Assets/Materials/** -> `.material` script sources.  
- **Assets/Shaders/** -> Custom shaders.  
- **Assets/Models/** -> Source meshes (`.mesh.xml`, `.fbx`, `.blend`).  
- **Assets/Textures/** -> Textures in editable formats (`.png`, `.tga`, `.dds`).  
- **Assets/Fonts/** -> Font sources (`.ttf`, `.otf`).  

During the build or packaging step, these assets will be copied into `bin/Data/` for runtime use.  
Keep the *sources* here. Git ignores the runtime copies, keeping repo size smaller if eventually moving to LFS.  

---

### Config
Configuration files and templates:
- `resources2.cfg.in` and `plugins.cfg.in` are CMake templates, found in Engine\CMake\Templates.  
- Additional engine/game config JSON or INI files can go here.  
- Developers customize these locally by generating or copying into `bin/Data/`.

---

### Scripts
Helper scripts for build, deployment, and packaging:
- `build_win.bat` (Windows builds)  
- `build_mac.sh` (macOS builds)  
- `deploy_assets.py` (copies source assets into `bin/Data/`)  

These scripts should be cross-platform where possible, to keep onboarding easy.

---

## Build Outputs

- **build/** -> CMake build directory (ignored by Git).  
- **bin/** -> Output executables and runtime data (ignored by Git).  

---

## Git LFS (Planned)

Currently, all assets are tracked normally in Git.  
When the project grows (large textures, models, audio), we will migrate heavy files in `Assets/` to [Git LFS](https://git-lfs.com/) or other asset management solutions.

Typical future rules are already in the .gitattributes
