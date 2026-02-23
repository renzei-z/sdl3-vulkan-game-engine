# SDL3-Vulkan Game Engine

This is an **incomplete** game engine written in C, using [SDL3](https://wiki.libsdl.org/SDL3/FrontPage) and [Vulkan](https://www.vulkan.org/).

## Prerequisites

Before building, ensure you have the following installed:
- Vulkan SDK
- SDL3 Development Headers
- GNU Make
- C Compiler (gcc/clang)
- DXC (DirectX Shader Compiler)
    - Whilst this is for DirectX, it is used to compile shaders to SPIR-V to be used by the engine.

## Usage

1. Clone the repo:
```bash
git clone git@github.com:renzei-z/sdl3-vulkan-game-engine.git
cd sdl3-vulkan-game-engine
```

2. Build:
```bash
make
```
 
3. Run:
```bash
make run
```

## License

This software is provided as is under the MIT license. The full text of the license can be found [here](./LICENSE).

The licenses of third-party libraries which are required to be distributed alongside the software can be found [here](./LICENSES).