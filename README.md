# MyEditor — OpenCV Image Editor

A GIMP-like desktop image editor with C++ and OpenCV HighGUI 

## Dependencies

- A C++17 compiler (g++ / clang / MSVC)
- CMake ≥ 3.16
- **OpenCV 4.x** .

## Build (one command after configure)

```bash
cmake -B build
cmake --build build
```

## Run

```bash
./build/myeditor samples/sample.jpg
```

## Controls

| Key       | Action                                          |
|-----------|-------------------------------------------------|
| `n` / `p` | Next / previous operation                       |
| `s`       | Save the full-resolution result to `output.png` |
| `q` / ESC | Quit                                            |

Each operation exposes its own sliders in the **Controls** window. Sliders act
on a downscaled preview for responsiveness; saving re-applies the operation to
the full-resolution image.

## Architecture

The shell (`src/main.cpp`) owns the window, the preview pipeline and op
switching. Every feature is an `Operation` subclass (`include/operation.hpp`)
living in its own files under `include/ops/` and `src/ops/`, so each feature has
a single owner and a clean commit path.

```
include/
  operation.hpp     # the feature interface
  app.hpp           # shared hooks (trackbar callback, render request)
  ops/*.hpp
src/
  main.cpp          # application shell
  ops/*.cpp
samples/            # test images
```

## Features

Core: Thresholding · Histogram Equalization · Morphology · Canny Edge
Detection · Geometric Transforms · Panorama / Stitching.

Advanced: Automatic White Balance · Cartoon Effect.

## Team & contributions

See `DEVELOPMENT.md` for the contribution matrix and per-feature ownership.