# Rocket Flight

A short C++/OpenGL animation for the 3D Animated Movie assignment. The scene is
constructed from OpenGL/GLUT primitives and demonstrates object modeling,
translation, rotation, scaling, keyframe interpolation, lighting, and a
perspective camera.

## Controls

- **Space**: start or pause
- **R**: reset to the beginning
- **Esc**: quit

## Build on macOS

The project uses Apple's GLUT and OpenGL frameworks:

```sh
clang++ -std=c++11 main.cpp -framework OpenGL -framework GLUT -o rocket_movie
./rocket_movie
```

Alternatively, build with CMake:

```sh
cmake -S . -B build
cmake --build build
./build/rocket_movie
```

## Animation outline

The rocket waits at the launch pad, lifts off, ascends, and arrives near the
satellite. Its position, tilt, and scale are interpolated between timed
keyframes using smoothstep easing. The engine flame pulses while the rocket is
in flight. The scene contains a rocket, launch tower/pad, satellite, and planet.
