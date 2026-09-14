# Ogre-Next build options

These are the CMake options this project sets when building its Ogre-Next
dependency, recorded while working out which features the engine needed. They
describe the dependency's configuration, not this engine's own build.

Yellow highlighted text indicates features likely to be toggled between builds.
Red highlighted text indicates deprecated features.
An asterisk before the variable name indicates a change from the Ogre-Next default.

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