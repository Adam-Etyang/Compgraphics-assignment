# Computer Graphics OpenGL Projects

This repository contains two independent C++/OpenGL coursework projects. Each
project has its own source, CMake build, report, and demonstration media.

## Projects

### 1. 3D Animated Movie

Source and report: [`animated_movie/`](animated_movie/)

Build and run from the repository root on macOS:

```sh
cmake -S animated_movie -B animated_movie/build
cmake --build animated_movie/build
./animated_movie/build/rocket_movie
```

Controls: **Space** to start/pause, **R** to reset, **Esc** to quit.

### 2. Colour Models

Source and report: [`colour_models/`](colour_models/)

Build and run from the repository root on macOS:

```sh
cmake -S colour_models -B colour_models/build
cmake --build colour_models/build
./colour_models/build/colour_visualizer
```

Run the colour-conversion tests:

```sh
ctest --test-dir colour_models/build --output-on-failure
```

Controls: drag an RGB or HSV slider, click a preset, **R** to reset, **Esc** to
quit.

## Requirements

- C++11 compiler
- CMake 3.16 or newer
- OpenGL, GLU, and GLUT/FreeGLUT development libraries

The projects have been built and tested on macOS with its system OpenGL and
GLUT frameworks. Other platforms may require installing GLUT/FreeGLUT and GLU.

## Repository contents

- `animated_movie/` — rocket animation source, report, screenshots, and recording.
- `colour_models/` — colour visualizer source, conversion tests, report,
  screenshot, and recording.
- `docs/` — original assignment brief.

Generated build directories and executables are excluded by `.gitignore`.
Reports and demonstration media are included.
