![https://hackatime-badge.hackclub.com/U08264TFUKV/mix_simulator](https://hackatime-badge.hackclub.com/U08264TFUKV/mix_simulator)
# Crazy Mix Simulator

A physics based game made using C++ and OpenGL. Currently a renderer based off of my opengl framework.

![screenshot](https://github.com/snej55/mix_simulator/blob/master/media/screenshots/Screenshot_20251213_093650.png)

Sketchfab GLTF model used: [https://sketchfab.com/3d-models/spartan-armour-mkv-halo-reach-57070b2fd9ff472c8988e76d8c5cbe66](https://sketchfab.com/3d-models/spartan-armour-mkv-halo-reach-57070b2fd9ff472c8988e76d8c5cbe66)

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
- [ ] Fog
- [X] Fix transparent mesh culling
- [X] SSAO
- [X] Add FXAA anti-aliasing
- [ ] Global materials
- [X] JSON Scene loading
- [X] Frustum culling
- [X] Space partitioning
- [X] Bounding volume generation
- [ ] Main game class
- [ ] Level editor
- [ ] Rag dolls
- [ ] Physics engine (Jolt? Box2D?)
- [ ] Optimize static shader uniforms
- [ ] Create player models
