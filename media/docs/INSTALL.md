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
