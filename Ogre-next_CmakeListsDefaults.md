
# Ogre-next CmakeLists.txt Components

### The following are the default settings in the CMakeLists.txt for Ogre-next and the developers' reasoning as to why (very partial list).

| Component                 | CMake Option                                  | Default | Purpose                                                                   | Reason for Default                                                                 |
| ------------------------- | --------------------------------------------- | ------- | ------------------------------------------------------------------------- | ---------------------------------------------------------------------------------- |
| **Bites**                 | `OGRE_BUILD_COMPONENT_BITES`                  | ON      | Small utilities like sample frameworks, console, and application helpers. | Useful for most sample/demo builds; low overhead.                                  |
| **OgreMain/Core**         | `OGRE_BUILD_COMPONENT_OGRE`                   | ON      | Core rendering engine.                                                    | Always required.                                                                   |
| **RTShaderSystem**        | `OGRE_BUILD_COMPONENT_RTSHADERSYSTEM`         | ON      | Real-time shader generator system.                                        | Core for modern rendering; most users need it.                                     |
| **Overlay**               | `OGRE_BUILD_COMPONENT_OVERLAY`                | ON      | UI overlay system for HUDs, menus, etc.                                   | Lightweight; widely used.                                                          |
| **BitesGUI**              | `OGRE_BUILD_COMPONENT_BITES_GUI`              | OFF     | Deprecated GUI helpers (mostly legacy).                                   | Optional; new projects often use custom UI.                                        |
| **Terrain**               | `OGRE_BUILD_COMPONENT_TERRAIN`                | OFF     | Terrain rendering component.                                              | Optional; adds dependencies and complexity; not needed for indoor/standard scenes. |
| **Paging**                | `OGRE_BUILD_COMPONENT_PAGING`                 | OFF     | Paged geometry system for large terrains/forests.                         | Optional, niche feature; adds dependencies on terrain and scene management.        |
| **Volume**                | `OGRE_BUILD_COMPONENT_VOLUME`                 | OFF     | Volumetric fog or other volume-based effects.                             | Specialized; rarely required; keeps default build light.                           |
| **RTShaderSystemPlugins** | `OGRE_BUILD_COMPONENT_RTSHADERSYSTEM_PLUGINS` | OFF     | Plugins for RTShaderSystem.                                               | Optional; enables extra shaders but not needed for minimal build.                  |
| **PropertyBindings**      | `OGRE_BUILD_COMPONENT_PROPERTYBINDINGS`       | OFF     | Data-driven property bindings for scripting or external tools.            | Optional; mostly for editor/scripting integration.                                 |
| **Samples**               | `OGRE_BUILD_SAMPLES`                          | OFF     | Sample programs demonstrating Ogre features.                              | Optional; increases build time; users may download separately.                     |
| **Tests**                 | `OGRE_BUILD_TESTS`                            | OFF     | Unit and integration tests.                                               | Optional; for developers only; not needed for runtime builds.                      |
| **Jumbo Builds**                 | `OGRE_UNITY_BUILD`                            | OFF     | Combine .cpp files into large translation units, compile them together.  | Optional; for developers only; compiler-specific, not needed for runtime builds. May reduce compile times.                      |


### The following are disabled CMake Options in Ogre-Next. The ones marked TRUE have been enabled in Engine


| Option | Type | Value | Description |
|--------|------|-------|-------------|
| `OGRE_ADDRESS_SANITIZER_ASAN` | BOOL | TRUE | Enable AddressSanitizer ASAN support |
| `OGRE_BUILD_ANDROID_JNI_SAMPLE` | BOOL | FALSE | Builds the Android JNI sample app (not needed for normal builds). |
| `OGRE_BUILD_COMPONENT_ATMOSPHERE` | BOOL | TRUE | Build the Atmosphere component |
| `OGRE_BUILD_COMPONENT_HLMS_PBS` | BOOL | TRUE | Build HLMS PBS component |
| `OGRE_BUILD_COMPONENT_HLMS_UNLIT` | BOOL | TRUE | Build HLMS Unlit component |
| `OGRE_BUILD_COMPONENT_MESHLODGENERATOR` | BOOL | TRUE | Build Mesh LOD Generator component |
| `OGRE_BUILD_COMPONENT_OVERLAY` | BOOL | TRUE | Build Overlay component |
| `OGRE_BUILD_COMPONENT_PAGING` | BOOL | TRUE | Build the Paging component |
| `OGRE_BUILD_COMPONENT_PLANAR_REFLECTIONS` | BOOL | TRUE | Build planar reflections component |
| `OGRE_BUILD_COMPONENT_PROPERTY` | BOOL | TRUE | Build Property component |
| `OGRE_BUILD_COMPONENT_VOLUME` | BOOL | TRUE | Build volume rendering component |
| `OGRE_BUILD_PLUGIN_PFX` | BOOL | TRUE | Build PFX plugin |
| `OGRE_BUILD_PLUGIN_PFX2` | BOOL | TRUE | Build PFX2 plugin |
| `OGRE_BUILD_RENDERSYSTEM_D3D11` | BOOL | TRUE | Build Direct3D11 render system |
| `OGRE_BUILD_RENDERSYSTEM_GL3PLUS` | BOOL | TRUE | Build OpenGL3+ render system |
| `OGRE_BUILD_RENDERSYSTEM_METAL` | BOOL | FALSE | Build Metal render system |
| `OGRE_BUILD_RENDERSYSTEM_VULKAN` | BOOL | TRUE | Build Vulkan render system |
| `OGRE_BUILD_SAMPLES2` | BOOL | TRUE | Build the new Samples framework |
| `OGRE_BUILD_TESTS` | BOOL | FALSE | Builds unit tests and the PlayPen test app. |
| `OGRE_BUILD_TOOLS` | BOOL | TRUE | Build Ogre tools like MaterialEditor |
| `OGRE_BUILD_XSIEXPORTER` | BOOL | FALSE | Builds the legacy **Softimage XSI exporter**. |
| `OGRE_CONFIG_ALLOCATOR` | STRING | 0 | Allocator type (0=none, 1=nedmalloc, 2=custom) |
| `OGRE_CONFIG_CONTAINERS_USE_CUSTOM_ALLOCATOR` | BOOL | FALSE | Use custom allocator for containers |
| `OGRE_CONFIG_DOUBLE` | BOOL | FALSE | Use double precision floating point |
| `OGRE_CONFIG_ENABLE_ASTC` | BOOL | TRUE | Enable ASTC texture support |
| `OGRE_CONFIG_ENABLE_DDS` | BOOL | TRUE | Enable DDS support |
| `OGRE_CONFIG_ENABLE_ETC` | BOOL | TRUE | Enable ETC texture support |
| `OGRE_CONFIG_ENABLE_FINE_LIGHT_MASK_GRANULARITY` | BOOL | FALSE | Enable fine light mask granularity |
| `OGRE_CONFIG_ENABLE_FREEIMAGE` | BOOL | FALSE | Enable FreeImage support. This is deprecated in newer ogre-next, replaced by STBI. |
| `OGRE_CONFIG_ENABLE_GLES2_GLSL_OPTIMISER` | BOOL | FALSE | Enable GLES2 GLSL optimization |
| `OGRE_CONFIG_ENABLE_GLES2_VAO_SUPPORT` | BOOL | FALSE | Enables Vertex Array Objects (VAO) for OpenGL ES 2 |
| `OGRE_CONFIG_ENABLE_GLES3_SUPPORT` | BOOL | TRUE | Enable GLES3 support |
| `OGRE_CONFIG_ENABLE_GL_STATE_CACHE_SUPPORT` | BOOL | TRUE | Enable OpenGL state cache |
| `OGRE_CONFIG_ENABLE_JSON` | BOOL | TRUE | Enable JSON support |
| `OGRE_CONFIG_ENABLE_LIGHT_OBB_RESTRAINT` | BOOL | TRUE | Enable light OBB restraint |
| `OGRE_CONFIG_ENABLE_MESHLOD` | BOOL | FALSE | Enable Mesh LOD support |
| `OGRE_CONFIG_ENABLE_PVRTC` | BOOL | TRUE | Enable PVRTC texture support. PowerVR Texture Compression (lossy texture compression format, especially for all iOS devices, predecessor to ASTC. May still be needed for iOS portability, not sure) |
| `OGRE_CONFIG_ENABLE_STBI` | BOOL | TRUE | Enable STBI image support. Single header library replacement for FreeImage with no vulnerabilities or dependency headaches. |
| `OGRE_CONFIG_ENABLE_TBB_SCHEDULER` | BOOL | FALSE | Enable Intel TBB scheduler |
| `OGRE_CONFIG_ENABLE_VIEWPORT_ORIENTATIONMODE` | BOOL | FALSE | Enable viewport orientation mode |
| `OGRE_CONFIG_ENABLE_ZIP` | BOOL | TRUE | Enable ZIP file support |
| `OGRE_CONFIG_MEMTRACK_DEBUG` | BOOL | FALSE | Enable memory tracking in debug builds |
| `OGRE_CONFIG_MEMTRACK_RELEASE` | BOOL | FALSE | Enable memory tracking in release builds |
| `OGRE_CONFIG_NODE_INHERIT_TRANSFORM` | BOOL | FALSE | Node transform inheritance |
| `OGRE_CONFIG_RENDERDOC_INTEGRATION` | BOOL | TRUE | Integrate RenderDoc for debugging |
| `OGRE_CONFIG_STATIC_LINK_CRT` | BOOL | FALSE | Link static C runtime library |
| `OGRE_CONFIG_STRING_USE_CUSTOM_ALLOCATOR` | BOOL | FALSE | Use custom allocator for strings |
| `OGRE_FULL_RPATH` | BOOL | FALSE | Full RPATH for libraries |
| `OGRE_INSTALL_SAMPLES_SOURCE` | BOOL | TRUE | Install sample source files |
| `OGRE_LIB_DIRECTORY` | STRING | "lib" | Directory for Ogre libraries |
| `OGRE_PLUGIN_LIB_PREFIX` | STRING | "" | Custom prefix for plugin libraries |
| `OGRE_PROFILING_PROVIDER` | STRING | 1 | Profiling provider (0=None, 1=Remotery, 2=Tracy) |
| `OGRE_SHADER_COMPILATION_THREADING_MODE` | STRING | 2 | Shader compilation threading mode |
| `OGRE_CONFIG_THREAD_PROVIDER` | STRING | std | Threading provider (std or TBB) |
