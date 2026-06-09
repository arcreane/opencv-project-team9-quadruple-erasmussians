# MyEditor — OpenCV Image Editor

A GIMP-like desktop image editor built with C++ and OpenCV HighGUI

## Dependencies

- A C++17 compiler (g++ / clang / MSVC)
- CMake ≥ 3.16
- **OpenCV 4.x** (tested with 4.x). Some advanced features use the
  `opencv_contrib` `xphoto` module — see notes in the relevant source files.

## Build (one command after configure)

```bash
cmake -B build
cmake --build build
```

## Run

**Windows (recommended): just double-click `build\myeditor.exe`.** A window opens
and a file dialog asks you to pick a photo — no terminal and no PATH setup (the
build copies the OpenCV DLL next to the exe, and the console window is hidden).
You can also drag an image onto the exe, or use right-click → "Open with".

To skip the picker, pass an image path (works on any OS):

```bash
./build/myeditor path/to/your/photo.jpg
```

## Controls

A translucent **status bar** sits along the bottom of the image at all times,
showing the current operation, its position in the list (e.g. `2/3`), and the key
legend — so you never need the (hidden) console to know what to press. An
**Operation** slider at the top of the window lets you jump straight to any
operation, and saving or opening shows a brief on-screen confirmation.

| Key       | Action                                          |
|-----------|-------------------------------------------------|
| `o`       | Open an image (file dialog)                      |
| `n` / `p` | Next / previous operation                       |
| `a`       | Apply current effect (bake it into the image)    |
| `u` / `r` | Undo / redo the last applied effect              |
| `s`       | Save the result (Save As dialog)                |
| `q` / ESC | Quit                                            |

The **Operation** slider and the `n` / `p` keys stay in sync — use whichever you
prefer. Each operation exposes its own sliders in the **Controls** window. Sliders
act on a downscaled preview for responsiveness; saving re-applies the operation to
the full-resolution image.

## Architecture

The shell (`src/main.cpp`) owns the window, the preview pipeline and op
switching. Every feature is an `Operation` subclass (`include/operation.hpp`)
living in its own files under `include/ops/` and `src/ops/`, so each feature has
a single owner and a clean, reviewable commit trail.

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

Advanced: Automatic White Balance · Cartoon Effect · Flood Fill.

### Implemented — notes for the report

- **Geometric Transforms** (`include/ops/geometry.hpp`, `src/ops/geometry.cpp`).
  Interactive perspective/affine correction. *Affine* maps 3 clicked points to
  the top-left/top-right/bottom-left corners (`getAffineTransform` +
  `warpAffine`); *Perspective* maps 4 clicked points to the four corners
  (`getPerspectiveTransform` + `warpPerspective`). **Why:** removes tilt/keystone
  so a slanted photo or a photographed page becomes axis-aligned. Clicks are
  stored normalized, so the same points warp the fast preview and the
  full-resolution save identically (`apply()` stays pure). **Controls:** Mode
  slider (0 = Affine, 1 = Perspective), left-click = place a point, right-click =
  reset. **Limitation:** points map to the image corners, so the result fills the
  frame (aspect ratio is not preserved).

- **Auto White Balance** (`include/ops/white_balance.hpp`,
  `src/ops/white_balance.cpp`). Removes a colour cast. *Gray-World* scales each
  BGR channel so its mean matches the overall mean (`cv::mean` + per-channel
  `convertTo`); *Simple WB* ignores ~2% of the extreme pixels per channel and
  stretches the rest to `[0,255]` — `cv::xphoto::SimpleWB` when opencv_contrib is
  built, otherwise a hand-rolled `cv::calcHist` percentile-clip, selected at
  compile time with `HAVE_OPENCV_XPHOTO`. **Why:** neutralises tinted lighting
  (too warm/cool) without hand-tuning curves. **Controls:** Method slider
  (0 = Gray-World, 1 = Simple) and a Strength slider that blends
  original↔balanced, so the correction is tunable and runs identically on the
  preview and the full-resolution save. **Limitation:** Gray-World over-corrects
  when one colour legitimately dominates the frame (e.g. a close-up of grass),
  pulling it toward gray.

## Team & contributions

- Yigit Dagidir
- Ilayda Baburoglu
- Sevil Begum Gurcan
- PinChieh HO
