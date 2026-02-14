![https://hackatime-badge.hackclub.com/U08264TFUKV/mix_simulator](https://hackatime-badge.hackclub.com/U08264TFUKV/mix_simulator)
# Crazy Mix Simulator

A physics based game made using C++ and OpenGL. Currently a renderer based off of my opengl framework.

![screenshot](https://github.com/snej55/mix_simulator/blob/master/media/screenshots/Screenshot_20260214_202103.png)

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
- HDR ([Khronos PBR Neutral Tonemapping)](https://github.com/KhronosGroup/ToneMapping/tree/main/PBR_Neutral)
- Bloom (PBR)
- SSAO (Screen Space Ambient Occlusion)
- Anti-aliasing (FXAA)
- Frustum Culling (AABB bounding volumes)
- Freetype2 font rendering
- Physics engine ([Jolt](https://github.com/jrouwe/JoltPhysics))
- Cascading Shadow Maps

![screenshot2](https://github.com/snej55/mix_simulator/blob/master/media/screenshots/Screenshot_20251213_093758.png)

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
- [ ] Smooth cascade transition
- [ ] Add credits
- [ ] Add alpha to shape rendering
- [ ] Add settings
- [X] Add in game font
- [ ] Find new HDRI
- [ ] Make some music
- [X] Add physics sfx using jolt contact listeners
- [ ] Add sliding sfx
- [ ] Add flow field pathfinding algorithm
- [ ] Clean up data folder
