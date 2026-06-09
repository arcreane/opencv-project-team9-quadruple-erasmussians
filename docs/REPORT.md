# MyEditor — A Small Image Editor Built On OpenCV

**Multimedia Application · Final Project**
Team 9 — *Quadruple Erasmussians*

**Authors:** Yigit Dagidir 64423 · Ilayda Baburoglu 64421  · Sevil Begum Gurcan 64427 · PinChieh Ho 64429


---

## 1. Introduction

We are four Erasmus students who had never written a desktop application together before, so we treated it as much a teamwork exercise as a computer-vision one.

The result is **MyEditor**. It loads an image, shows it in a window, and lets you run any of thirteen operations on it — the six the brief made mandatory, plus seven extras of our own. Every operation is driven live: you move a slider or click on the picture and the preview updates immediately. You can apply an effect to keep it, undo and redo your way back and forth, and save the result through the normal file dialog of your operating system. It builds and runs on both Windows and macOS, which mattered to us because one of us was working from Taiwan for the whole project.

This report explains what we chose to build and why. It covers the OpenCV functions behind each feature, how the code is organised, how we split the work, and, honestly, the parts that still need fixing. There is no source code here; if you want to see how something is implemented, the commit history will point you to the right file.

---

## 2. What MyEditor Does

When you launch the program it asks for an image straight away (you can also pass a path on the command line). Once a picture is loaded you get two windows: the main canvas with the image on it, and a small **Controls** window holding the sliders for whatever operation is currently selected.

Picking an operation works two ways. There is an *Operation* slider along the top of the canvas that jumps straight to any of the thirteen tools, and there are the `n` / `p` keys to step forward and back through them. The two stay in sync, so you can use whichever feels natural. The bottom of the canvas always shows a thin status bar: the name of the current operation, its position in the list (for example `4 / 13`), and a short reminder of the keyboard shortcuts. We added that bar specifically so you never have to remember what to press.

A typical session looks like this:


 >![[browser.png]]
 ![[initial.png]]
 ![[operation3.png]]



1. Choose an operation and adjust the sliders. The canvas shows the effect as you switch. Nothing is applied yet. It is only preview.
2. Some tools want a mouse instead of sliders. Geometric transforms ask you to click corner points on the image; flood fill wants you to click the region to fill.
3. When you want to save the change, press `a` to **apply** it. Thisbakes the effect into the working image and records the step so it can be undone.
4. `u` and `r` undo and redo. After a save or an open, a small message ("Saved", "Opened") fades in over the image for a second so we get visual confirmation.
5. `s` opens the system's Save dialog and writes the file out.

The whole thing is keyboard-light: open (`o`), apply (`a`), undo/redo (`u`/`r`), next/previous (`n`/`p`), save (`s`), quit (`q`). That is the entire control scheme, and it fits on one line of the status bar.

---

## 3. Choosing The Interface

We went with **OpenCV's own HighGUI** with its windows, trackbars and mouse callbacks. We prefered it rather than Qt. This was a real decision with real trade-offs, so it is worth explaining.

HighGUI won mainly on cost. It comes with OpenCV, so there was nothing extra to install, configure or package. Anyone on the team could clone the repo and build in one command. Specifically since we have different operating systems. We had also already used trackbars in the lab sessions, so nobody had to learn a new framework before writing a single feature. 

The honest downside is that HighGUI is plain. There are no proper menus, no dialog styling, no docking. And a trackbar is the only widget that it really gives. We knew going with this option wouldn't amaze the users. So we spent some effort pushing it as far as it would go. The translucent status bar, the operation picker that lets you jump to any tool, the fade-in confirmation messages, and the live preview on every slider are all things HighGUI does not give us for free. Then we built them on top. The point was to make a minimal toolkit feel responsive and self-explanatory, even if it could never look like Photoshop.

Qt was the recommended choice and would have given us a far nicer-looking program, but the signals-and-slots model and the build setup were a lot to take on alongside the actual image processing. Dear ImGui was tempting for a tool like this, but wiring its immediate-mode drawing to an OpenCV image buffer was more plumbing than we wanted early on. If we picked the project up again, a Qt front-end is the first thing we would change. The processing code underneath would barely have to move which is the point we come back to in the next section.

---

## 4. How the Program is Put Together

### 4.1 One idea: Every Tool is an "Operation"

The core of the design is a single small interface that every tool implements. An *Operation* knows three things: its name, how to set up its own sliders in the Controls window and how to apply itself to an image (take a picture in, return the processed picture out). A fourth, smaller flag tells the app whether a tool manages its own undo history, which matters for the click driven tools.

Everything else is built around this architecture. The application keeps a list of operations, a vector of owning pointers filled once at start-up and the rest of the program never needs to know which tool is active. To render the screen it just asks the current operation to apply itself and shows the result. To build the sliders it asks the current operation to set up its trackbars. Adding a new feature means writing one new class and adding one line to the list. Nothing else changes this way.

This was the decision that made the teamwork possible. Because each operation lives in its own file behind the same interface, four people could write four features at the same time without stepping on each other. It also kept the git history clean: a feature is almost always one file, so it is obvious from a commit who wrote what.

### 4.2 The Pieces Around It

> ![[architecture.png]]

*The render loop never edits the working image directly; it asks the active operation for a fresh result every time and draws the HUD on top of that copy.*

The **application shell** loads the image, makes a downscaled copy for previewing, opens the windows, and runs the event loop. The loop waits briefly for a key, checks whether the operation picker is changed, then clears any expired confirmation message. Otherwise it stays out of the way.

The **HUD layer** (the status bar and the fade-in messages) is display only. It draws into the copy of the image that is about to be shown, never onto the image itself, so the text can never end up in a saved file. We used simple drawing calls for it. It is a darkened strip blended over the bottom of the frame with `addWeighted`, then `putText` for the labels with `getTextSize` to right-align the shortcut legend and shrink it if the window is too narrow.

The **file dialogs** are wrapped behind two functions, open and save, with three implementations underneath: the native Windows dialogs (`GetOpenFileNameW` / `GetSaveFileNameW`), an AppleScript call on macOS through `osascript`, and a plain fallback elsewhere. One hardness we hit early: reading an image straight from a path breaks on non-Latin characters, which is common on the machines we were using. We solved it by reading the file into a byte buffer ourselves and handing that to `imdecode`, and by writing with `imencode` plus a normal file write. That keeps Unicode paths working and keeps the OS specific code small.

### 4.3 The Data We Keep

The working image is a single colour `Mat`. To stay responsive, the preview is a downscaled copy: we cap the longest side at 720 pixels because recomputing a filter on a 24 megapixel photo every time a slider changes would make the program laggy(unresponsive). Each operation holds its own little bundle of parameters — threshold values, kernel sizes, blur radii — bound to its sliders.

Undo and redo are two stacks of image snapshots. Applying an effect pushes a clone of the new image into the undo stack and clears the redo stack. Undo moves the top snapshot across to the redo stack and steps back. Redo does the reverse. We cap the history at thirty steps so memory stays bounded on large pictures. Snapshots are the direct approach.A more memory-efficient approach would be to keep track of reversible commands instead of saving full images. But since the history is limited anyway, storing full snapshots is simpler and much less error prone. In practice we preferred that reliability over the extra memory savings.

---

## 5. The Six Core Operations

### 5.1 Thresholding

Thresholding turns a grey image into pure black-and-white by a rule on pixel brightness. We expose three modes through one slider. Plain binary thresholding (`threshold` with a fixed cut) is the baseline. **Otsu** (the same call with the Otsu flag) is the one we reach for most, because it picks the cut automatically by finding the value that best separates the two brightness groups (no manual tuning). **Adaptive** thresholding (`adaptiveThreshold`, mean type) computes a separate threshold for each small neighbourhood which is the only one of the three that survives uneven lighting. It is what you want for a photographed document where one corner is in shadow. We convert to grey first and then for the adaptive mode we force the neighbourhood size to an odd number because the function requires it.

> ![[threshold.png]]

### 5.2 Histogram Equalization

This spreads an image's tones out to use the full range and lift contrast. The trick we learned here was *where* to do it. Equalising the red, green and blue channels separately wrecks the colours. So we convert to YCrCb, equalise only the luma (brightness) channel, and convert back. The contrast improves while the colours stay put. We offer two methods: global equalisation (`equalizeHist`), which is one curve for the whole image and **CLAHE** (`createCLAHE`) which equalises in tiles and clips the histogram to stop noise blowing up in flat areas. CLAHE has two controls: The clip limit and the tile grid. It is the better choice on images where one global curve over brightens the sky to fix the foreground.

> ![[histogram.png]]

### 5.3 Morphology

Morphological operations reshape the bright regions of an image with a small probe called a structuring element. We build that element with `getStructuringElement` and let the user choose its shape (rectangle, cross or ellipse) and size. Five operations sit behind a single mode slider: dilation grows bright areas, erosion shrinks them, opening (erode then dilate) removes small specks, closing (the reverse) fills small holes and the morphological gradient (the difference of the two) outlines edges. Dilation and erosion are the direct calls. The other three go through `morphologyEx`.

> ![[morphology.png]]

### 5.4 Canny Edge Detection

Canny is the classic edge finder. After converting to grey we call `Canny` with both hysteresis thresholds and the aperture size as user controls. The two thresholds matter because Canny keeps strong edges outright and keeps weak ones only if they connect to a strong one; sliding them changes how much fine detail survives. The aperture sets the size of the gradient operator underneath. We return the result as a 3 channel image so it slots back into the rest of the pipeline like any other picture.

> ![[canny_edge_detection.png]]

### 5.5 Geometric Transforms

This tool straightens or re-projects an image from points you click on it. In affine mode you click three points and we map them to the image corners with `getAffineTransform`, then warp with `warpAffine`; three points fix rotation, scale, shear and translation. In perspective mode you click four corners and we use `getPerspectiveTransform` and `warpPerspective`, which can flatten a photo taken at an angle. A slanted page becomes a clean rectangle. We store the clicked points relative to the image size so they stay correct on the downscaled preview, draw markers as you place them and let a right-click reset if we misclick.

> ![[geometry.png]]

### 5.6 Panorama / Stitching

Stitching blends two overlapping photos into one wide image. Because the editor works on one open picture at a time. We load the second image on demand. Sliding the trigger control opens a file dialog. Then once a second picture is in we hand both to OpenCV's `Stitcher` in panorama mode (which finds matching features, estimates the geometry and blends the seams) . Stitching fails for honest reasons (too little overlap, not enough texture) so we read the status code back and show a clear message instead of crashing: "need more images", "no overlap or features found", and so on. If the two images are different sizes we resize the second to match before stitching.

---

## 6. The Features We Added

The brief wanted at least two advanced features and asked that every member own one. We ended up with seven, partly because the Operation interface made each one cheap to add once the shell existed. They span filters, creative effects and interactive tools.

### 6.1 Automatic White Balance — *Yigit*

This removes a colour cast so whites look white. We implemented two methods. **Gray-world** assumes the whole scene should average out to neutral, measures each channel's mean with `mean`, and scales the channels with `convertTo` until they line up; a warm indoor photo cools back to neutral. **Simple white balance** stretches each channel after ignoring a small percentage of the darkest and brightest pixels, so a few stray hot pixels don't throw it off. Where OpenCV's `xphoto` contrib module is available we use its `SimpleWB`; where it isn't, we fall back to our own version built on `calcHist`. That fallback was a deliberate portability choice. It means the feature works even on an OpenCV build without the extra modules, which not everyone had installed. A strength slider blends the corrected image back towards the original with `addWeighted`, so you can dial the effect down if it overshoots.

> ![[autowb,.png]]

### 6.2 Undo / Redo — *Yigit*

Covered in the architecture section, this is the interactive feature that ties the others together. The model is the snapshot stack described above with one design point worth repeating: because previews are non-destructive, "undo" only means something once an edit is committed, so we made `a` (apply) the explicit step that bakes the current effect in and records it. This was the part we reworked most recently. An earlier version recorded history for only one tool, so undo appeared to do nothing for the rest. It now genuinely steps backwards and forwards through applied edits.

### 6.3 Unsharp Mask — *Sevil*

It is the photographer's sharpening trick rather than a naive edge boost. We blur a copy of the image with `GaussianBlur`, subtract it from the original to isolate the fine detail, and add that detail back with `addWeighted`. The strength slider controls how much detail is added and the radius controls the blur, so we can can sharpen fine texture or broad edges. Doing it this way avoids amplifying single-pixel noise the way a plain high-pass filter would.

### 6.4 Pencil Sketch — *Sevil*

It is a quick artistic effect using OpenCV's `pencilSketch`, which produces both a grey graphite look and a softer coloured pencil version from the same call. We expose both modes and the line-thickness control. It is a small feature but it shows off the non photographic side of the editor.

> ![[pencil_sketch.png]]

### 6.5 Cartoon — *Ilayda*

The cartoon look is two effects combined. First we flatten colour with several passes of `bilateralFilter`, which smooths areas while keeping edges crisp, and then quantise the colours through a lookup table (`LUT`) so the image reads as flat cartoon shading rather than smooth gradients. Separately we find bold outlines on a grey, median-blurred copy with `adaptiveThreshold`. Laying the black outlines over the flattened colour with `bitwise_and` gives the final comic-strip result. The first attempt smoothed the whole picture into mush; the bilateral filter and the separate edge pass were what made it look intentional.

> ![[cartoon.png]]

### 6.6 Flood Fill (Magic Wand) — *Chieh*

This is the "magic wand" from any paint program: click a spot and a connected region of similar colour gets filled. We use `floodFill` in fixed-range mode, where the upper and lower tolerance sliders decide how far the fill spreads from the clicked colour, and a colour picker sets the fill. The seed point comes from a mouse click on the canvas. Each fill is committed to the undo history, so you can try a click, undo it, and try again with a wider tolerance.


### 6.7 Brightness and Contrast — *Yigit*

The simplest tool, and the one we wrote first to prove the shell worked. It is a linear adjustment through `convertTo` — contrast scales the pixel values, brightness shifts them. We kept it because it is genuinely useful and it doubles as the "hello world" that every later operation was modelled on.

>![[brightness_contrast.png]]

---

## 7. Working as a Team

Our workflow was feature branches and pull requests on the GitHub Classroom repository. Branches were named after the feature and its issue. For example `6-corefunctions-Panorama-Stitching`, `13-advanced-feature-automatic-white-balance`, and `16-enhance-ui-and-ux` — and merged into `main` through reviewed pull requests (#17 through #20 among them). Keeping one feature per branch and, usually, one feature per file meant merges rarely conflicted, and it kept each person's contribution legible in the history.

We split ownership early, which the architecture made painless: with the Operation interface agreed up front, each of us could take a tool and work to the same contract without waiting on anyone else. Yiğit built the application shell, the HUD, the file dialogs and the undo system first so that there was a skeleton to plug features. The survival tip from the brief about building the GUI skeleton before the features is one we actually followed. After that the core and advanced operations were divided up roughly evenly and developed in parallel.

Our toolchain was deliberately ordinary: Git and GitHub for collaboration, CMake to build the same way on every machine, OpenCV (with the optional `xphoto` module where available), and a mix of Visual Studio and VS Code depending on personal preference. The README documents the dependencies and the one-command build, and the repository carries a sample image for testing.

---

## 8. Who Built What

| Member                 | Core features                        | Advanced features                                    | Other                                                                                                    |
| ---------------------- | ------------------------------------ | ---------------------------------------------------- | -------------------------------------------------------------------------------------------------------- |
| **Yiğit Dağıdır**      | Geometric transforms                 | Auto white balance, undo/redo, brightness & contrast | Application shell & event loop, operation picker, HUD (status bar + toasts), cross-platform file dialogs |
| **Ilayda Baburoglu**   | Thresholding, histogram equalisation | Cartoon                                              | Presentation, Report and Comments                                                                        |
| **Sevil Begum Gurcan** | Morphology, Canny edge detection     | Unsharp mask, pencil sketch                          | Comments                                                                                                 |
| **- PinChieh HO**      | Panorama / stitching                 | Flood fill (magic wand)                              | Cross-platform / build testing                                                                           |

Every member owns at least one advanced feature. The work divides cleanly along the operation files, which the commit history reflects.

---

## 9. What is Still Rough? and What We Would Do Next?

We would rather be straight about the weak spots than pretend they aren't there.

**Export saves the preview, not the full-resolution image.** The biggest one. We keep the full resolution original in memory and downscale a copy for the preview. Exactly as the brief advises. But the save path currently writes out the preview sized copy rather than rerunning the operation on the original. For a large photo that means the exported file is smaller than it should be. The fix is to replay the applied edits on the full-resolution image at save time; we ran out of week before wiring that through, and we would tackle it first.

**Mouse handling is shared too loosely.** Geometric transforms and flood fill both want mouse clicks, and they currently compete for the same callback on the canvas. After you have visited the geometry tool, flood-fill clicks can stop registering, and clicks meant for nothing in particular can leak into whichever tool last claimed the mouse. It needs a single owner that routes clicks to the active tool only; right now the ownership is implicit and order-dependent.

**Stitching recomputes on every redraw.** Once a second image is loaded, the panorama is re-stitched each time the window repaints, which makes the program stutter while that tool is selected. The result should be computed once and cached until the inputs change.

**A few tools draw their hints onto the image.** Geometry, stitching and flood fill draw on-canvas guidance (point markers, status text) inside the same step that produces the image, so in some situations that guidance can be baked into a saved file. The clean separation we use for the HUD  should be extended to those tools too.

Smaller things: the editor works on one image at a time with no layers; the undo history is capped at thirty steps and is not saved between sessions; and HighGUI sets a ceiling on how polished the window can look.

If we kept going, the priority list would be: full-resolution export, a single mouse router, and caching the panorama (those three remove real friction). After that, a Qt front-end would lift the whole feel of the program without touching the processing code. Because every tool already hides behind the same small interface. We were also tempted by a couple of the advanced features we didn't reach, GrabCut and watershed segmentation in particular, and by saveable "recipes" so a set of edits could be reapplied to another photo.

---

## 10. Conclusion

We set out to build a real image editor, not a script, and we got there: thirteen working operations, a live-preview GUI with undo and redo, sensible failure messages, and a build that runs on more than one operating system. All six mandatory operations are present and exposed properly, and we added seven features on top of them.

The technical lesson we keep coming back to is how much the early architecture decision paid off. Agreeing on one tiny interface for every tool, before anyone wrote a feature, is what let four people work in parallel without tripping over each other. It is the same decision that makes the program easy to extend now. The teamwork is simple: commit small and often, build the skeleton first, and write things down as you go. The features we are proudest of are not the flashiest ones but the ones where we understood the technique well enough to make a real choice. For ex: equalising luma instead of colour, sharpening through an unsharp mask instead of a crude filter, picking Otsu when the image allows it and adaptive when it doesn't.

The limitations above are the ones we are aware about them. But it is a program we would actually open to fix a crooked photo, and after six weeks that feels like the right place to have landed.

---

## Appendix

**Build and run.** See the repository `README.md` for dependencies and the one-command build. In short: OpenCV (with the optional `xphoto` module for the contrib white-balance path), a C++17 compiler and CMake. On Windows the build bundles the OpenCV runtime next to the executable.

**Tested with.** OpenCV ⟨version — confirm⟩, on Windows 11 and macOS.

