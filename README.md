![https://hackatime-badge.hackclub.com/U08264TFUKV/mix_simulator](https://hackatime-badge.hackclub.com/U08264TFUKV/mix_simulator)
# Duck Bowling

A physics based game where you play as a duck destroying various ceramic items. It is intended mainly as a tech demo for my OpenGL renderer, which most of the development time was spent on. 

![screenshot](https://github.com/snej55/mix_simulator/blob/master/media/screenshots/Screenshot_20260314_192138.png)

## Building:

This should work on Linux. You will most likely also have to install some additional dependencies if you get linking errors (e.g. on fedora: `sudo dnf install mesa-libGLU-devel alsa-lib-devel`).
Building:
```
git clone  https://github.com/snej55/mix_simulator.git
cd mix_simulator

# build it
cmake -S . -B build
cmake --build build/ -j$(nproc)

# to run
./build/main
```

## Renderer Features:
- Physically based rendering
- IBL (Image Based Lighting)
- Skeletal animation
- GLTF Model loading
- Hybrid renderer (deferred shading / forward rendering)
- HDR (ACES & Khronos PBR Neutral)
- Bloom (PBR)
- SSAO (Screen Space Ambient Occlusion)
- Anti-aliasing (FXAA)
- JSON Scene loading
- Frustum Culling (AABB bounding volumes)
- Freetype2 font rendering
- Physics engine ([Jolt](https://github.com/jrouwe/JoltPhysics))
- Spatial audio ([SoLoud](github.com/jarikomppa/soloud))
- Cascading Shadow Maps
- Flow field pathfinding (static quadtree generation)

![screenshot2](https://github.com/snej55/mix_simulator/blob/master/media/screenshots/Screenshot_20260314_192035.png)
![screenshot2](https://github.com/snej55/mix_simulator/blob/master/media/screenshots/Screenshot_20260314_192255.png)
![screenshot2](https://github.com/snej55/mix_simulator/blob/master/media/screenshots/Screenshot_20260314_192231.png)
![screenshot2](https://github.com/snej55/mix_simulator/blob/master/media/screenshots/Screenshot_20260318_193635.png)

## TODO:
- [X] Finish IBL
- [X] Skeletal animation
- [X] PBR Textures
- [X] PBR Material loading
- [X] Deferred rendering
- [X] Mesh sorting (render opaque meshes deferred and transparent/translucent using forward)
- [ ] Duplicate material checks? Batch render meshes using material index vertex data?
- [ ] Animate transparent meshes
- [X] Fog
- [X] Implement static entities
- [X] Fix transparent mesh culling
- [X] SSAO
- [X] Add FXAA anti-aliasing
- [ ] Global materials
- [X] JSON Scene loading
- [X] Frustum culling
- [X] Space partitioning
- [X] Bounding volume generation
- [X] Main game class
- [X] Level editor
- [X] Add lights to level editor
- [ ] Rag dolls?
- [X] Fix arena deletion order (child -> parent -> root)
- [X] Add Jolt Physics
- [X] Point lights
- [ ] Add shader compilation test
- [X] Add lens dirt
- [ ] Find lens dirt texture
- [ ] Area lights
- [ ] Optimize static shader uniforms
- [X] Add player character + controls/kinematic body
- [X] Create player models
- [X] Implement freetype2 font rendering
- [X] Screen coordinates shape rendering
- [X] Add UI framebuffer
- [X] Add cascading shadow maps
- [X] Smooth cascade transition
- [X] Add credits
- [X] Make camera follow player
- [ ] Add alpha to shape rendering
- [ ] Add settings
- [X] Add in game font
- [X] Find new HDRI
- [X] Make some music
- [X] Add physics sfx using jolt contact listeners
- [ ] Add sliding sfx
- [ ] Add different sfx for different materials
- [X] Generate flow field quadtree
- [ ] Paralax mapping with tesselation shader?
- [X] Quad tree generation
- [X] Flow field pathfinding
- [X] Make some models
- [X] Generate shards in blender
- [X] Preload jolt convex hulls
- [X] Shatter logic
- [X] Test shadows with complex shapes
- [X] Flow field generation
- [X] Clean up data folder
- [X] Convex hull preloading (export jolt bodies as binaries)
- [X] Particle vfx
- [X] Load all assets at the start
- [X] Fix sounds
- [X] Fix pathfinding when player is same tile
- [X] Different screen resolutions
- [X] Screenshake
- [ ] Variance shadow mapping?
- [X] Add settings page to menu
- [X] Score
- [ ] Spikes?
- [ ] Star particles?
- [X] Dash
- [ ] Collision particles
- [X] Shockwave postprocessing
- [ ] Perlin noise fog?
- [ ] Make camera scroll change focal length
- [X] Multithreaded assets loading (partially)
- [X] Cloudy skybox
- [ ] Different fog intensities
- [X] Add invisible walls around whole thing
- [X] Try some cool post processing fx (cross stitching?)
- [X] Duck puns on first menu
- [X] Level transitions
- [X] Add tonemapping options to settings
- [X] Lens dirt!

## Game loop (today):
- [X] Create decor
- [ ] Maze generation?
- [X] Walls around level
- [X] New HDRI
- [X] Limit player flying
- [X] Find rubber duck model
- [X] Particles

## Blender items:
- [X] Mug
- [X] Bud vase
- [ ] Pot
- [X] Cider jug (remake)
- [X] Wrecking ball 
- [ ] Pestle
- [X] Anaphora
- [ ] Ewer
- [X] Player character (rubber duck)
- [X] Add all the shards

## Debugging/Release todo:
- [X] Polish shadows
- [X] Fix camera jitter when falling
- [ ] Fix minimizing bug on Windows
- [ ] Fix mouse position scaling with menu button on Windows
- [ ] Fix MacOS build
- [X] Centre models around WS origin
- [X] Compile release version
- [X] Test linux build
- [X] Fix window title
