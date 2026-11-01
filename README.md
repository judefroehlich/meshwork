# meshworks

$\large\text{Meshworks}$ is a lightweight, cross-platform 3d modeling engine.

Created by Jude Froehlich

<!-- Showcase modeling, animation, and a 3d print here -->

## Features

- OpenGL 3.3 core rendering with GLFW and glad
- Shader class that loads and compiles GLSL shaders from files

## Roadmap

- [ ] Transformations
- [ ] Camera
- [ ] Creating shapes, lines, and vertices
- [ ] GUI
- [ ] Vertex-level editing: beveling, splitting meshes, etc.
- [ ] Smart selection
- [ ] Importing/exporting (GLB, STL)
- [ ] Lighting
- [ ] Simple keyframe animation

## Building and running

Meshworks is currently developed and tested on Linux.

### Requirements

- A C++17 compiler (g++ or clang++)
- GLFW 3
- A GPU that supports OpenGL 3.3.
  glad is included in the repo, so you don't need to install it separately.

Install GLFW:

```bash
# Arch
sudo pacman -S glfw

# Debian / Ubuntu
sudo apt install build-essential libglfw3-dev
```

### Build

```bash
git clone https://github.com/judefroehlich/meshworks.git
cd meshworks
g++ -Wall -Wextra src/*.cpp src/glad.c -Iinclude -lglfw -o main
```

### Run

Run from the repo root so Meshworks can find its shader files:

```bash
./main
```

Press `Esc` to close the window.
