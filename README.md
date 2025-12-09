
# Engine

Engine is a custom game engine built on top of [Ogre-next](https://github.com/OGRECave/ogre-next).  
It provides a modular framework for rendering, input, and game state management, with a focus on clarity and extensibility.

---

## Setup (For Windows)

- Install CMake (preferably with CMake-GUI) Version 3.29.3 from https://cmake.org/files/v3.29/

- Install wxWidgets Version 3.2.8 from https://wxwidgets.org/downloads/ (Anywhere you set the environment variable should work after it is installed, but the project expects it to be installed directly in the default location at C:\wxWidgets)

- Install python 3.11 (For shaderc needed to compile shaders for ogre-next engine and for ogre-next scripts if you want to play around with them) https://www.python.org/downloads/ (or use chocolatey or whatever method you prefer, but newer than 3.11 is not recommended, as some packages may have additional dependency issues with later versions)

- (OPTIONAL: To allow the option to run the rendering engine using the Vulkan RenderSystem. You will still be able to run it with D3D11 and/or OpenGL 3+ on Windows if you do not install this) Install Vulkan SDK from the LunarG website https://vulkan.lunarg.com/ 

- Navigate to the root directory of a drive (close to root helps with shaderc file path length limitations later, good results cannot be guaranteed if you place the project elsewhere)

- Download the Engine_Build_Visual_Studio_17_2022_x64.bat in the root directory on this GitHub page and place it at the root directory where you want the Engine directory to be created

- Run the Engine_Build_Visual_Studio_17_2022_x64.bat file

- After the project is done downloading and building, navigate to ...\Engine\bin\Debug and run editor.exe to make your own scene or to create an editor project out of the SampleProject located at Engine\Media\SampleProject. You can also run Engine.exe directly if you want to just immediately see the sample project scene. (NOTE: When looking at the SampleProject in the editor, removing the Albedo/Diffuse texture from the material and leaving the rest of the textures will make it much easier to notice the Ambient Occlusion feature that was implemented, especially when then toggling the Ambient Occlusion texture on/off and trying the Playtest button again. The AO_Diff1.png - AO_Diff4.png files in SampleProject show the scene with Ambient Occlusion removed, then added, then a albedo/diffuse pixel color difference mapping showing only the differences between the two images resulting from ambient occlusion, and then another difference mapping in grayscale with the differences magnified to contrast the areas where Ambient Occlusion effects were more pronounced on the material)

- From the editor you can create a scene or use the prebuilt scene, and click Playtest to launch the Engine.exe to playtest the scene!

---

## Setup (For MacOS)

- Install XCode Command Line Tools: xcode-select --

- Install Homebrew https://brew.sh/

- Install CMake (preferably with CMake-GUI) Version 3.29.3 from https://cmake.org/files/v3.29/ or brew install cmake

- Install wxWidgets Version 3.2.8 from https://wxwidgets.org/downloads/ or brew install wxwidgets

- Install python 3.11 (For shaderc needed to compile shaders for ogre-next engine and for ogre-next scripts if you want to play around with them) https://www.python.org/downloads/ (or use chocolatey or whatever method you prefer, but newer than 3.11 is not recommended, as some packages may have additional dependency issues with later versions)

- (OPTIONAL: To allow the option to run the rendering engine using the Vulkan RenderSystem. You will still be able to run it with Metal on MacOS if you do not install this) Install Vulkan SDK from the LunarG website https://vulkan.lunarg.com/ or use brew install molten-vk (though the official SDK from LunarG is preferable for development headers if you ever want to modify vulkan shaders). Apple does not natively support Vulkan. To use Vulkan, you need MoltenVK, which translates Vulkan calls to Metal.

- Navigate to the root directory of a drive (close to root helps with shaderc file path length limitations later, good results cannot be guaranteed if you place the project elsewhere, though this should only matter on Windows)

- Download the Engine_Build_Unix.sh in the root directory on this GitHub page and place it at the root directory where you want the Engine directory to be created

- Run the Engine_Build_Unix.sh file

- After the project is done downloading and building, navigate to ...\Engine\bin\Debug and run editor to make your own scene or to create an editor project out of the SampleProject located at Engine\Media\SampleProject. You can also run Engine directly if you want to just immediately see the sample project scene. (NOTE: When looking at the SampleProject in the editor, removing the Albedo/Diffuse texture from the material and leaving the rest of the textures will make it much easier to notice the Ambient Occlusion feature that was implemented, especially when then toggling the Ambient Occlusion texture on/off and trying the Playtest button again. The AO_Diff1.png - AO_Diff4.png files in SampleProject show the scene with Ambient Occlusion removed, then added, then a albedo/diffuse pixel color difference mapping showing only the differences between the two images resulting from ambient occlusion, and then another difference mapping in grayscale with the differences magnified to contrast the areas where Ambient Occlusion effects were more pronounced on the material)

- From the editor you can create a scene or use the prebuilt scene, and click Playtest to launch the Engine to playtest the scene!

---

## Setup (For Linux)

- sudo apt update

- sudo apt install build-essential git

- Install CMake (preferably with CMake-GUI) Version 3.29.3 from https://cmake.org/files/v3.29/ or:
    - sudo apt install build-essential libssl-dev
    - wget https://github.com/Kitware/CMake/releases/download/v3.29.3/cmake-3.29.3.tar.gz
    - tar -xf cmake-3.29.3.tar.gz
    - cd cmake-3.29.3
    - ./bootstrap
    - make
    - sudo make install

- Install wxWidgets Version 3.2.8 from https://wxwidgets.org/downloads/ or sudo apt install libwxgtk3.2-dev

- Install python 3.11 (For shaderc needed to compile shaders for ogre-next engine and for ogre-next scripts if you want to play around with them) https://www.python.org/downloads/ (newer than 3.11 is not recommended, as some packages may have additional dependency issues with later versions) or: 
    - sudo add-apt-repository ppa:deadsnakes/ppa
    - sudo apt install python3.11

- (OPTIONAL: To allow the option to run the rendering engine using the Vulkan RenderSystem) Install Vulkan SDK from the LunarG website https://vulkan.lunarg.com/ or use sudo apt install libvulkan-dev vulkan-tools

- Install X11 and OpenGL Headers: sudo apt install libx11-dev libxrandr-dev libglu1-mesa-dev freeglut3-dev mesa-common-dev (Note: If you have an NVIDIA GPU, ensure your proprietary drivers are installed via "Additional Drivers" settings)

- Install X11 Athena Widget Set (Xaw) headers for Linux windowing implementation dependencies and pkg-config: sudo apt install libxaw7-dev libxt-dev libx11-xcb-dev pkg-config
    - sudo apt update

- Install SDL2 (for window creation and mouse/keyboard input) and XCB RandR headers (to allow Vulkan to communicate with X11 to resize/rotate screen resolution & detect monitor changes): sudo apt install libsdl2-dev libxcb-randr0-dev libxcb-xtest0-dev libxcb-xinerama0-dev libxcb-shape0-dev libxcb-xfixes0-dev

- Navigate to the root directory of a drive (close to root helps with shaderc file path length limitations later, good results cannot be guaranteed if you place the project elsewhere, though this should only matter on Windows)

- Download the Engine_Build_Unix.sh in the root directory on this GitHub page and place it at the root directory where you want the Engine directory to be created

- Run the Engine_Build_Unix.sh file

- After the project is done downloading and building, navigate to ...\Engine\bin\Debug and run editor to make your own scene or to create an editor project out of the SampleProject located at Engine\Media\SampleProject. You can also run Engine directly if you want to just immediately see the sample project scene. (NOTE: When looking at the SampleProject in the editor, removing the Albedo/Diffuse texture from the material and leaving the rest of the textures will make it much easier to notice the Ambient Occlusion feature that was implemented, especially when then toggling the Ambient Occlusion texture on/off and trying the Playtest button again. The AO_Diff1.png - AO_Diff4.png files in SampleProject show the scene with Ambient Occlusion removed, then added, then a albedo/diffuse pixel color difference mapping showing only the differences between the two images resulting from ambient occlusion, and then another difference mapping in grayscale with the differences magnified to contrast the areas where Ambient Occlusion effects were more pronounced on the material)

- From the editor you can create a scene or use the prebuilt scene, and click Playtest to launch the Engine to playtest the scene!

---

### \Samples

If you want to look at some samples, there are .exe files in **E:\Engine\Dependencies\Ogre\ogre-next\build\bin\debug** and their related c++ files are located in the various subfolders under **E:\Engine\Dependencies\Ogre\ogre-next\Samples**.

---

### \Config
Configuration files and templates:
- `resources2.cfg.in` and `plugins.cfg.in` are CMake templates, found in Engine\CMake\Templates.  
- Additional engine/game config JSON or INI files can go here.  
- Developers usually customize these locally by generating or copying into `bin/Data/`.

---

### \Scripts
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


# Ogre-next CmakeLists.txt Build Options
(Yellow highlighted text indicates features likely to be toggled between builds)

(Red highlighted text indicates deprecated features)

(Asterisk before the variable name indicates a new change)

## Debug/General
| **Variable**                             | **Type / Values**            | **Set Value** | **Description**                                                                              |
| ---------------------------------------- | ---------------------------- | ---------------------------- | -------------------------------------------------------------------------------------------- |
| `OGRE_ASSERT_MODE`                       | STRING (0, 1, 2)             | `0`                          | Runtime assert handling: `0=off`, `1=abort`, `2=throw exception`.                            |
| `OGRE_DEBUG_LEVEL_DEBUG`                 | STRING (0-3)                 | `3`                          | Debug build logging level (verbosity).                                                       |
| `OGRE_DEBUG_LEVEL_RELEASE`               | STRING (0-3)                 | `0`                          | Release build logging level.                                                                 |
| `OGRE_EMBED_DEBUG_MODE`                  | STRING (`auto`, `on`, `off`) | `auto`                       | Whether debug symbols/resources are embedded in executables.                                 |
| `OGRE_PROFILING_EXHAUSTIVE`              | BOOL                         | `OFF`                        | <mark>Enables more detailed but slower CPU/GPU profiling.</mark>                             |
| `OGRE_PROFILING_TEXTURES`                | BOOL                         | `OFF`                        | <mark>Tracks GPU texture memory usage in profiles.</mark>                                    |
| *`OGRE_SHADER_COMPILATION_THREADING_MODE` | STRING (0, 1, 2)             | `2`                          | Shader compilation threading: `0=single-threaded`, `1=background async`, `2=multi-threaded`. |
| `OGRE_RESTRICT_ALIASING`                 | BOOL                         | `ON`                         | Adds compiler flags to assume strict aliasing rules (better optimization).                   |

## Components
| **Variable**                              | **Type / Values** | **Set Value** | **Description**                                                    |
| ----------------------------------------- | -------- | ----------- | ------------------------------------------------------------------ |
| `OGRE_BUILD_COMPONENT_ATMOSPHERE`         | BOOL     | `ON`        | Builds atmosphere & sky scattering component.                      |
| `OGRE_BUILD_COMPONENT_HLMS_PBS`           | BOOL     | `ON`        | Builds HLMS (High Level Material System) Physically Based Shading. |
| `OGRE_BUILD_COMPONENT_HLMS_UNLIT`         | BOOL     | `ON`        | Builds HLMS Unlit shading system.                                  |
| `OGRE_BUILD_COMPONENT_MESHLODGENERATOR`   | BOOL     | `ON`        | Builds mesh LOD (level-of-detail) generator.                       |
| `OGRE_BUILD_COMPONENT_OVERLAY`            | BOOL     | `ON`        | Builds overlay system (2D HUD/UI).                                 |
| `OGRE_BUILD_COMPONENT_PAGING`             | BOOL    | `OFF`        | <span style="color: red;">DEPRECATED. Builds paging/streaming terrain system.</span>                            |
| `OGRE_BUILD_COMPONENT_TERRAIN`             | BOOL   | `OFF`        | <span style="color: red;">DEPRECATED. Builds Terrain component within the Ogre-Next engine (Heightmap-based terrain, Level of Detail (LOD), Layered texturing, Paging system).</span>                            |
| *`OGRE_BUILD_COMPONENT_PLANAR_REFLECTIONS` | BOOL    | `ON`        | Builds planar reflections component.                               |
| `OGRE_BUILD_COMPONENT_PROPERTY`           | BOOL     | `ON`        | Builds property system (serialization/config helpers).             |
| `OGRE_BUILD_COMPONENT_SCENE_FORMAT`       | BOOL     | `ON`        | Enables scene format import/export component.                      |
| `OGRE_BUILD_COMPONENT_VOLUME`             | BOOL     | `OFF`       | Builds volume rendering support.                                   |

## Plugins
| **Variable**             | **Type / Values** | **Set Value** | **Description**                              |
| ------------------------ | -------- | ----------- | -------------------------------------------- |
| `OGRE_BUILD_PLUGIN_PFX`  | BOOL     | `ON`        | Builds legacy particle effects plugin.       |
| `OGRE_BUILD_PLUGIN_PFX2` | BOOL     | `ON`        | Builds newer particle effects plugin (PFX2). |

## Render Systems
| **Variable**                      | **Type / Values**      | **Set Value** | **Description**                                    |
| --------------------------------- | ------------- | ----------- | -------------------------------------------------- |
| `OGRE_BUILD_RENDERSYSTEM_D3D11`   | BOOL          | `ON`        | Build Direct3D11 render system (Windows only).     |
| `OGRE_BUILD_RENDERSYSTEM_GL3PLUS` | BOOL          | `ON`        | Build modern OpenGL 3+ render system.              |
| `OGRE_BUILD_RENDERSYSTEM_GLES2`   | BOOL          | `OFF`       | Build OpenGL ES 2 render system.                   |
| `OGRE_BUILD_RENDERSYSTEM_VULKAN`  | BOOL          | `ON`        | Build Vulkan render system.                        |
| `OGRE_VULKAN_SDK`                 | STRING (path) | *(empty)*   | Path to Vulkan SDK (if not autodetected).          |
| `OGRE_VULKAN_WINDOW_NULL`         | BOOL          | `OFF`       | Enables Vulkan offscreen/null window backend.      |
| `OGRE_VULKAN_WINDOW_WIN32`        | BOOL          | `ON`        | Enables Vulkan Win32 window backend.               |
| `OGRE_GLSUPPORT_USE_WGL`          | BOOL          | `ON`        | Use Windows WGL instead of EGL for OpenGL context. |

## Build Systems
| **Variable**                | **Type / Values**     | **Set Value** | **Description**                                                |
| --------------------------- | ------------ | ----------- | -------------------------------------------------------------- |
| `OGRE_BUILD_MSVC_MP`        | BOOL         | `ON`        | Enable `/MP` (multi-processor compilation) in MSVC.            |
| `OGRE_BUILD_MSVC_ZM`        | BOOL         | `ON`        | Enable `/Zm` MSVC option (increase precompiled header memory). |
| `OGRE_BUILD_SAMPLES2`       | BOOL         | `ON`        | Builds Ogre 2.x sample browser.                                |
| `OGRE_BUILD_TESTS`          | BOOL         | `OFF`       | Builds unit tests.                                             |
| `OGRE_BUILD_TOOLS`          | BOOL         | `ON`        | Builds tools (e.g., MeshUpgrader, HlmsJson).                   |
| `OGRE_UNITY_BUILD`          | BOOL         | `OFF`       | Enable unity builds (compile multiple source files together).  |
| `OGRE_UNITY_FILES_PER_UNIT` | STRING (int) | `50`        | Number of files per unity build unit.                          |
| `OGRE_USE_NEW_PROJECT_NAME` | BOOL         | `ON`        | Use `OgreNext` as project name instead of `OGRE`.              |
| `OGRE_STATIC`               | BOOL         | `OFF`       | Build static instead of shared libraries.                      |

## Config/Features
| **Variable**                            | **Type / Values**                        | **Set Value** | **Description**                                                   |
| --------------------------------------- | ------------------------------- | ----------- | ----------------------------------------------------------------- |
| `OGRE_CONFIG_AMD_AGS`                   | BOOL                            | `ON`        | Enable AMD AGS (GPU extensions library) support.                  |
| `OGRE_CONFIG_ENABLE_JSON`               | BOOL                            | `ON`        | Enable JSON scene/material support.                               |
| `OGRE_CONFIG_ENABLE_QUAD_BUFFER_STEREO` | BOOL                            | `OFF`       | Enable quad-buffered stereo rendering (3D displays).              |
| `OGRE_CONFIG_THREADS`                   | STRING (0,1,2,3)                | `0`         | <span style="color: red;">Threading model: `0=none`, `1=std::thread`, `2=TBB`, `3=pthread`. In Ogre 2.x and beyond, a different, more scalable multithreading system is used. This newer system is enabled by default and does not require CMake configuration. Instead, you control it directly in your code when creating a SceneManager by specifying the number of worker threads. </span> |
| `OGRE_CONFIG_THREAD_PROVIDER`           | STRING (`none`, `tbb`, `boost`) | `none`      | <span style="color: red;">Choose threading backend. This configuration option only applies if you enable threading through the older OGRE_CONFIG_THREADS setting. It tells Ogre which external library to use to manage the background threads for resource loading and preparation. In contemporary Ogre-Next versions (2.1+), threading is automatically managed and does not rely on OGRE_CONFIG_THREAD_PROVIDER. The engine's high-performance, Data-Oriented Design uses multithreading for tasks like frustum culling, batch processing, and other performance-critical operations. As a user, you get full control over the number of worker threads by specifying it when you create a SceneManager instance. This approach is faster, more scalable, and simplifies the build process.</span>                                         |
| `OGRE_IDSTRING_ALWAYS_READABLE`         | BOOL                            | `OFF`       | <mark>Force ID strings to be readable instead of hashed. </mark>               |
| `OGRE_IDSTRING_USE_128`                 | BOOL                            | `OFF`       | <mark>Use 128-bit IDs for object identifiers. </mark>                          |
| `OGRE_SIMD_NEON`                        | BOOL                            | `ON`        | Enable ARM NEON SIMD intrinsics. Used on mobile phones and single-board computers.                                 |
| `OGRE_SIMD_SSE2`                        | BOOL                            | `ON`        | Enable SSE2 SIMD intrinsics.                                      |

## Install/Deployment
| **Variable**                | **Type / Values** | **Set Value**        | **Description**                                   |
| --------------------------- | -------- | ------------------ | ------------------------------------------------- |
| `OGRE_COPY_DEPENDENCIES`    | BOOL     | `ON`               | Copy dependency binaries into build/install tree. |
| `OGRE_DEPENDENCIES_DIR`     | PATH     | *(your deps path)* | Root directory of dependencies.                   |
| `OGRE_INSTALL_DEPENDENCIES` | BOOL     | `ON`               | Install dependency binaries alongside Ogre.                      |
| `OGRE_INSTALL_DOCS`         | BOOL     | `ON`               | Install documentation.                            |
| `OGRE_INSTALL_PDB`          | BOOL     | `ON`               | Install MSVC `.pdb` debug symbol files.           |
| `OGRE_INSTALL_SAMPLES`      | BOOL     | `ON`               | Install sample data and binaries.                 |
| `OGRE_INSTALL_TOOLS`        | BOOL     | `ON`               | Install tools.                                    |
| *`OGRE_INSTALL_VSPROPS`      | BOOL     | `ON`              | <mark>Install Visual Studio property sheets. </mark>           |