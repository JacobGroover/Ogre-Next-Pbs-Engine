
# Engine

Engine is a custom game engine built on top of [Ogre-next](https://github.com/OGRECave/ogre-next).  
It provides a modular framework for rendering, input, and game state management, with a focus on clarity and extensibility.

---

## Setup (For Windows)

- Install python 3.11 https://www.python.org/downloads/ (or use chocolatey or whatever method you prefer, but newer than 3.11 is not recommended, as some packages may have additional dependency issues later in development)

- Install CMake (preferably with CMake-GUI) Version 3.29.3 from https://cmake.org/files/v3.29/

- Install Vulkan SDK https://vulkan.lunarg.com/ 

- Navigate to the root directory of a drive (close to root helps with shaderc file path length limitations later, good results cannot be guaranteed if you place the project elsewhere)

- Download the Engine_build_ogre_next_Visual_Studio_17_2022_x64.bat in the root directory on this GitHub page and place it at the root directory where you want the Engine directory to be created

- Run the Engine_build_ogre_next_Visual_Studio_17_2022_x64.bat file

---

### Samples

If you want to look at some samples, there are .exe files in **E:\Engine\Dependencies\Ogre\ogre-next\build\bin\debug** and their related c++ files are located in the various subfolders under **E:\Engine\Dependencies\Ogre\ogre-next\Samples**.

---

### Config
Configuration files and templates:
- `resources2.cfg.in` and `plugins.cfg.in` are CMake templates, found in Engine\CMake\Templates.  
- Additional engine/game config JSON or INI files can go here.  
- Developers usually customize these locally by generating or copying into `bin/Data/`.

---

### Scripts
Helper scripts for build, deployment, and packaging, will probably include the main build script later.
---

## Build Outputs

- **build/** -> CMake build directory (ignored by Git).  
- **bin/** -> Output executables and runtime data (ignored by Git).  

---

## Git LFS (Planned)

Currently, all assets are tracked normally in Git.  
When the project grows (large textures, models, audio), we will migrate heavy files in `Assets/` to [Git LFS](https://git-lfs.com/) or other asset management solutions.

Typical future rules are already in the .gitattributes
