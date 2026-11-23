# Colour Model Visualizer

An interactive C++/OpenGL visualizer for additive RGB colour mixing and
RGB-to-HSV / HSV-to-RGB conversion.

## Controls

- Drag any RGB or HSV slider to edit the selected colour.
- Click a preset swatch (Red, Green, Blue, White, or Black).
- **R** resets to the initial blue colour.
- **Esc** quits.

The large preview shows the combined RGB result. The three smaller swatches show
the selected red, green, and blue channel contributions. Editing either slider
group updates the other model's values and the preview.

## Build and run on macOS

```sh
cmake -S colour_models -B colour_models/build
cmake --build colour_models/build
./colour_models/build/colour_visualizer
```

Run the colour conversion tests with:

```sh
ctest --test-dir colour_models/build --output-on-failure
```

The app uses traditional OpenGL and GLUT, as permitted by the assignment. It
does not load image textures or perform CMYK conversion; CMYK is discussed in
the research report, while the required interactive demonstration focuses on
RGB mixing and RGB/HSV conversion.

## Report and demonstration files

- `report.pdf` — final report with the visualizer screenshot.
- `report.docx` — editable report.
- `report.md` — editable report source.
- `screenshots/color_visualizer.png` — captured application window.
- `screenshots/colour_recording.mov` — supplementary recording.
