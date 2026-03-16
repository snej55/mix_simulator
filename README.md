![https://hackatime-badge.hackclub.com/U08264TFUKV/mix_simulator](https://hackatime-badge.hackclub.com/U08264TFUKV/mix_simulator)
# Crazy Mix Simulator

A physics based game made using C++ and OpenGL. Currently a renderer based off of my opengl framework.

![screenshot](https://github.com/snej55/mix_simulator/blob/master/media/screenshots/Screenshot_20260314_192138.png)

Sketchfab GLTF model used: [https://sketchfab.com/3d-models/spartan-armour-mkv-halo-reach-57070b2fd9ff472c8988e76d8c5cbe66](https://sketchfab.com/3d-models/spartan-armour-mkv-halo-reach-57070b2fd9ff472c8988e76d8c5cbe66)

## Building:

```
git clone --depth 1 https://github.com/snej55/mix_simulator.git
cd mix_simulator

# build it
cmake -S . -B build
cmake --build build/ -j$(nproc)

# to run
cd build
./main
```

## Features:
- Physically based rendering
- IBL (Image Based Lighting)
- Skeletal animation
- GLTF Model loading
- Hybrid renderer (deferred for opaque meshes, forward for transparent)
- HDR [ACES & Khronos PBR Neutral Tonemapping](https://github.com/KhronosGroup/ToneMapping/tree/main/PBR_Neutral)
- Bloom (PBR)
- SSAO (Screen Space Ambient Occlusion)
- Anti-aliasing (FXAA)
- Frustum Culling (AABB bounding volumes)
- Freetype2 font rendering
- Physics engine ([Jolt](https://github.com/jrouwe/JoltPhysics))
- Cascading Shadow Maps
- Flow field pathfinding

![screenshot2](https://github.com/snej55/mix_simulator/blob/master/media/screenshots/Screenshot_20260314_192035.png)
![screenshot2](https://github.com/snej55/mix_simulator/blob/master/media/screenshots/Screenshot_20260314_192255.png)
![screenshot2](https://github.com/snej55/mix_simulator/blob/master/media/screenshots/Screenshot_20260314_192231.png)
![screenshot2](https://github.com/snej55/mix_simulator/blob/master/media/screenshots/Screenshot_20251213_093650.png)

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
- [ ] Find new HDRI
- [X] Make some music
- [X] Add physics sfx using jolt contact listeners
- [ ] Add sliding sfx
- [ ] Add different sfx for different materials
- [X] Generate flow field quadtree
- [ ] Paralax mapping with tesselation shader?
- [X] Quad tree generation
- [X] Flow field pathfinding
- [ ] Make some models
- [X] Generate shards in blender
- [X] Preload jolt convex hulls
- [X] Shatter logic
- [X] Test shadows with complex shapes
- [X] Flow field generation
- [ ] Clean up data folder
- [X] Convex hull preloading (export jolt bodies as binaries)
- [ ] Particle vfx
- [X] Load all assets at the start
- [ ] Fix sounds
- [ ] Make shards despawn
- [X] Fix pathfinding when player is same tile
- [X] Different screen resolutions
- [X] Screenshake
- [ ] Variance shadow mapping?
- [X] Add settings page to menu
- [ ] Make player explodable
- [ ] Score
- [ ] Spikes?
- [ ] Star particles?
- [X] Dash
- [ ] Collision particles
- [ ] Shockwave postprocessing
- [ ] Perlin noise fog?
- [ ] Make camera scroll change focal length
- [ ] Cloudy skybox
- [ ] Different fog intensities
- [ ] Level transitions
- [X] Add invisible walls around whole thing
- [ ] Try some cool post processing fx (cross stitching?)
- [ ] Duck jokes on first menu

## Game loop (today):
- [ ] Create decor
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
- [ ] Anaphora
- [ ] Ewer
- [X] Player character (rubber duck)
- [ ] Add all the shards

## Debugging/Release todo:
- [X] Polish shadows
- [X] Fix camera jitter when falling
- [ ] Fix minimizing bug on Windows
- [ ] Fix mouse position scaling with menu button on Windows
- [ ] Fix MacOS build
- [ ] Centre models around WS origin
- [ ] Compile release version