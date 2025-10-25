## OpenGL engine
![time](https://hackatime-badge.hackclub.com/U08264TFUKV/opengl_engine)

An OpenGL rendering system to use for my projects, made largely to replace my old OpenGL wrapper.

 - [Installation instructions](https://github.com/snej55/opengl_engine/tree/master?tab=readme-ov-file#installation-instructions)
 - [To do](https://github.com/snej55/opengl_engine/tree/master?tab=readme-ov-file#to-do)

![screenshot](https://github.com/snej55/opengl_engine/blob/master/media/screenshots/Screenshot_20251012_131612.png)

---

## Installation instructions

>[!IMPORTANT]
> CMake and Ninja are required to be installed, as well as a working c++ compiler.

1. Clone the repo:

```
git clone --depth 1 https://github.com/snej55/opengl_engine
cd opengl_engine
```

2. Build:

```
# generate cmake files
cmake -S . -B build -G Ninja
# build it
cmake --build build/ -j4
```


>[!NOTE]
>Certain libraries will need to be installed via your distro's package manager
> in order to link the binary successfully (mainly glfw3 and assimp). Depending on 
> your distro, you may need to install additional opengl libs (e.g mesa-libGLU for fedora).


3. Run:

```
# either run it automatically via the build script:
chmod +x ./run.sh
./run.sh

# or do it manually (recommended)
cd build
./main
```

>[!NOTE]
>Please make sure to run the compiled binary from the build folder,
> so that it has access to the required assets (shaders, models, etc).


---

## To do:

### Target:

Make a basic OpenGL rendering framework that can handle PBR lighting & stuff, as well as basic game engine stuff (e.g. physics, audio, window management, level-editor, multi-platform exports)

### v2:
- [X] Set up engine object type
- [X] Set up engine manager
- [X] Window management
- [X] IOHandler
- [X] Main loop
- [X] Clock
- [X] Set up arena
- [X] Shaders
- [X] Shader manager
- [X] Automatic shader loading
- [X] Camera
- [ ] Set up G-Buffer (PBR data)
- [ ] Deffered shading (PBR)
- [ ] Integrate forward renderer alongside deffered shading
- [ ] Extend deffered shading to deffered lighting (MSAA & blending)
- [ ] Set up renderer class
- [ ] Set up render queue
- [ ] Config file loading?
- [X] README
- [X] Font rendering
- [ ] UI framebuffer
- [X] Texture manager
- [X] Texture loading and rendering
- [ ] Screen coordinate rendering
- [X] Rect rendering
- [X] Draw a cube
- [X] Add timers
- [X] Add fps counter
- [X] Default shaders
- [X] Post processor with framebuffers
- [X] Blinn-phong lighting
- [ ] Ambient lighting (SSAO)
- [X] PBR lighting
- [X] PBR material rendering
- [X] PBR materials on models
- [X] Add support for embedded materials in models
- [X] Model loading
- [X] Mesh class
- [X] Model manager
- [ ] Model animations (skeletal)
- [X] Colour functions (lerp, different blending functions)
- [ ] Box2D vs Chipmunk physics
- [ ] Tesselation
- [ ] Terrain generator
- [ ] Level editor
- [ ] Vim-style keybindings for level editor
- [ ] Particles
- [ ] ImGui ui
- [ ] Audio loading (SoLoud?)
- [ ] Asset manager singleton
- [ ] 2D rendering
- [ ] 2D tile map
- [ ] 2D particles
- [ ] 2D basic physics
- [ ] WebAssembly builds (emscripten)
- [ ] Separate Windowing api (SDL vs GLFW)
- [ ] Vulkan?

### v1:
- [X] Set up engine objects
- [X] Add app component
- [X] Add window component
- [X] Add IOHandler
- [X] Fix initialization and termination
- [X] Add logs
- [X] Give engine objects names
- [X] Tracking
- [X] Add clock object
- [X] Finish setting up main loop
- [X] Set up renderer class
- [X] Draw a cube
- [X] Add timers
- [ ] REFACTOR!
- [ ] Memory arena
- [ ] Uniform buffer object for render matrices
- [ ] Add camera ..
- [X] Shader uniforms
- [ ] Add line draw mode
- [ ] Geometry shader option
- [ ] Default shaders
- [ ] Shader templates
- [ ] Custom shaders
- [ ] Basic lighting shaders (Phong & Blinn-Phong)
- [ ] Point lights, directional lights, spot-lights
- [ ] Model loading with assimp
- [ ] Material system
- [X] Render queue
- [ ] Postprocessing (framebuffer system)
- [ ] Shadows
- [ ] PBR
- [ ] Model animations (Bones system)
- [ ] Physics system
- [ ] Physics mesh generation
- [ ] Texture loading
- [ ] Texture rendering
- [ ] Audio loading
- [ ] Audio playback
- [ ] Scene system
- [X] Objects
- [ ] Items
- [ ] Entities
- [ ] Item manager
- [ ] Entity manager
- [ ] Level editor
- [ ] Level loading
- [ ] World data object
- [ ] Octrees for world map physics data
- [ ] UI
- [ ] FPS Counter
- [ ] Debug popup
- [ ] Fonts
- [ ] Screen coordinates rendering
- [ ] 2D mode
- [ ] Loading assets splash screen
- [ ] Proper fullscreen
- [ ] Add README
- [ ] Add README to media
- [ ] Add vfx demos + screenshots

