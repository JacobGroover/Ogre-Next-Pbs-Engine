# Ogre-Next-Pbs-Engine

A custom 3D rendering engine built on [Ogre-Next](https://github.com/OGRECave/ogre-next),
with an EnTT entity component system, a JSON scene format, and an ambient
occlusion texture slot added to Ogre-Next's PBS material system.

Developed September to December 2025 as a capstone project.

![Without ambient occlusion](docs/images/AO_Diff1.png)
![With ambient occlusion](docs/images/AO_Diff2.png)

The same scene without and with the ambient occlusion map applied. Ogre-Next's
PBS material system has no AO texture slot, so adding one meant working inside
HLMS, its runtime shader generator. That work lives in a
[fork of Ogre-Next](https://github.com/JacobGroover/ogre-next) and is written up
in [docs/ambient-occlusion.md](docs/ambient-occlusion.md), including the enum
width bug and the const buffer offset bug it surfaced.

---

## What is here

- **Ambient occlusion in HLMS PBS.** A new `PBSM_AO` texture slot threaded
  through the texture type enum, the datablock, the GPU const buffer upload,
  and the generated shader pieces. About 215 lines across 12 files in the
  Ogre-Next fork. See [docs/ambient-occlusion.md](docs/ambient-occlusion.md).
- **EnTT entity component system.** Scene entities are EnTT registry entries
  carrying `TransformComponent`, `SceneNodeComponent` and renderable or light
  components, with a render sync step pushing transforms to Ogre scene nodes.
- **JSON scene format.** Scenes, materials and texture resources are described
  in a single JSON document and deserialized at startup, including full PBR
  material setup (albedo, normal, roughness, metallic, AO, and packed ORM).
- **Cross platform build.** One build script per platform bootstraps the
  dependency tree, builds Ogre-Next and its dependencies, and then the engine.
  D3D11, OpenGL 3+, Vulkan and Metal render systems.

## Architecture

```
src/Engine.cpp             entry point, graphics system and resource setup
src/EngineGameState.cpp    scene loading, JSON deserialization, ECS wiring
include/Components/        EnTT components (Transform, OgreRenderable, Spin)
src/OgreCommon/            Ogre-Next sample framework (upstream, unmodified)
CMake/                     dependency and template configuration
Media/SampleProject/       sample scene, PBR textures, AO comparison renders
docs/                      ambient occlusion writeup, Ogre-Next build options
```

The engine loads a scene by parsing its JSON document in two passes: first
mapping texture resource names to files, then building `HlmsPbsDatablock`
materials from those maps, then walking the node tree to create EnTT entities
and their matching Ogre scene nodes. Node types dispatch to items, lights or
cameras.

## Scene format

A scene is one JSON document holding a node tree and a resource list. Nodes
carry a transform and a type; materials reference textures by resource name
rather than by path, so the same scene can be retargeted without rewriting node
data.

```json
{
  "nodes": [
    { "type": "SceneNode", "name": "Scene", "uid": 1, "ambientLight": 0.5 },
    { "type": "LightNode", "name": "Light", "uid": 2, "parent": 1,
      "lightType": 1, "intensity": 1.0, "rotationX": -60.0, "rotationY": -75.0 },
    { "type": "PrimitiveNode", "name": "Primitive", "uid": 4, "parent": 1,
      "material": "res://Material.7", "meshType": 0, "positionX": 15.0 }
  ],
  "resources": [
    { "type": "MaterialResource", "name": "Material.7", "uid": 7,
      "albedoTexture": "Texture.8", "ambientOcclusionMap": "Texture.9",
      "normalMap": "Texture.10", "roughnessMap": "Texture.11" },
    { "type": "TextureResource", "name": "Texture.9", "uid": 9,
      "path": "A23DTEX_Ambient_Occlusion.jpg" }
  ]
}
```

Pass a scene with `--scene <path>`. With no argument the engine loads
`Media/SampleProject`.

---

## Building

The build scripts clone the engine and its dependencies, build Ogre-Next, and
then build the engine. Expect a long first build, since Ogre-Next is compiled
from source.

### Windows

- Install Visual Studio 17 2022 (or current version)

- Install CMake (preferably with CMake-GUI) Version 3.29.3 from https://cmake.org/files/v3.29/

- Install python 3.11 (For shaderc needed to compile shaders for ogre-next engine and for ogre-next scripts if you want to play around with them) https://www.python.org/downloads/ (or use chocolatey or whatever method you prefer, but newer than 3.11 is not recommended, as some packages may have additional dependency issues with later versions)

- (OPTIONAL: To allow the option to run the rendering engine using the Vulkan RenderSystem. You will still be able to run it with D3D11 and/or OpenGL 3+ on Windows if you do not install this) Install Vulkan SDK from the LunarG website https://vulkan.lunarg.com/

- Navigate to the root directory of a drive (close to root helps with shaderc file path length limitations later, good results cannot be guaranteed if you place the project elsewhere)

- Download the Engine_Build_Visual_Studio_17_2022_x64.bat in the root directory on this GitHub page and place it at the root directory where you want the Engine directory to be created

- Run the Engine_Build_Visual_Studio_17_2022_x64.bat file

- After the project is done downloading and building, navigate to `...\Engine\bin\Debug` and run `Engine.exe` to load the sample scene.

### MacOS

- Install XCode Command Line Tools: xcode-select --

- Install Homebrew https://brew.sh/

- Install CMake (preferably with CMake-GUI) Version 3.29.3 from https://cmake.org/files/v3.29/ or brew install cmake

- Install python 3.11 (For shaderc needed to compile shaders for ogre-next engine and for ogre-next scripts if you want to play around with them) https://www.python.org/downloads/ (or use chocolatey or whatever method you prefer, but newer than 3.11 is not recommended, as some packages may have additional dependency issues with later versions)

- (OPTIONAL: To allow the option to run the rendering engine using the Vulkan RenderSystem. You will still be able to run it with Metal on MacOS if you do not install this) Install Vulkan SDK from the LunarG website https://vulkan.lunarg.com/ or use brew install molten-vk (though the official SDK from LunarG is preferable for development headers if you ever want to modify vulkan shaders). Apple does not natively support Vulkan. To use Vulkan, you need MoltenVK, which translates Vulkan calls to Metal.

- Navigate to the root directory of a drive (close to root helps with shaderc file path length limitations later, good results cannot be guaranteed if you place the project elsewhere, though this should only matter on Windows)

- Download the Engine_Build_Unix.sh in the root directory on this GitHub page and place it at the root directory where you want the Engine directory to be created

- Run the Engine_Build_Unix.sh file

- After the project is done downloading and building, navigate to `.../Engine/bin/Debug` and run `Engine` to load the sample scene.

### Linux

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

- After the project is done downloading and building, navigate to `.../Engine/bin/Debug` and run `Engine` to load the sample scene.

---

## Dependencies

| Dependency | Role |
|---|---|
| [Ogre-Next](https://github.com/JacobGroover/ogre-next) | Rendering. Forked to add the AO texture slot to HLMS PBS. |
| [ogre-next-deps](https://github.com/JacobGroover/ogre-next-deps) | Ogre-Next's own dependency bundle. |
| [EnTT](https://github.com/skypjack/entt) | Entity component system. |

## Notes

- `src/OgreCommon/` and `include/OgreCommon/` are Ogre-Next's sample framework,
  included upstream and unmodified, under Ogre-Next's MIT license.
- `docs/ogre-next-build-options.md` records the CMake options used to configure
  the Ogre-Next dependency.
- `build/` and `bin/` are build outputs and are not tracked.
