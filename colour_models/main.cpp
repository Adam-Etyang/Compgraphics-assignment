/*
 * Interactive RGB and HSV colour-model visualizer.
 *
 * Controls:
 *   Drag any RGB or HSV slider with the left mouse button.
 *   Click a preset swatch to select a standard colour.
 *   R     - reset to the initial colour
 *   Esc   - quit
 *
 * The slider groups edit one shared colour. Editing RGB recalculates HSV;
 * editing HSV converts the new HSV value back to RGB.
 */

#if defined(__APPLE__)
#define GL_SILENCE_DEPRECATION
#include <GLUT/glut.h>
#include <OpenGL/glu.h>
#else
#include <GL/glut.h>
#include <GL/glu.h>
#endif

#include "color_math.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>

namespace {

constexpr int kCanvasWidth = 1000;
constexpr int kCanvasHeight = 700;
constexpr int kWindowWidth = 1000;
constexpr int kWindowHeight = 700;

struct Rect {
    float x;
    float y;
    float width;
    float height;
};

enum class SliderType {
    Red,
    Green,
    Blue,
    Hue,
    Saturation,
    Value,
};

struct Slider {
    SliderType type;
    float x;
    float y;
    float width;
    float height;
    const char* label;
};

struct Preset {
    Rect bounds;
    const char* label;
    colour::RGBColor color;
};

colour::RGBColor gRGB = {0.20f, 0.55f, 0.85f};
colour::HSVColor gHSV = colour::rgbToHsv(gRGB);
int gWindowWidth = kWindowWidth;
int gWindowHeight = kWindowHeight;
int gViewportX = 0;
int gViewportY = 0;
int gViewportWidth = kCanvasWidth;
int gViewportHeight = kCanvasHeight;
int gActiveSlider = -1;

const Slider kSliders[] = {
    {SliderType::Red,        220.0f, 520.0f, 320.0f, 18.0f, "R"},
    {SliderType::Green,      220.0f, 470.0f, 320.0f, 18.0f, "G"},
    {SliderType::Blue,       220.0f, 420.0f, 320.0f, 18.0f, "B"},
    {SliderType::Hue,        220.0f, 300.0f, 320.0f, 18.0f, "H"},
    {SliderType::Saturation, 220.0f, 250.0f, 320.0f, 18.0f, "S"},
    {SliderType::Value,      220.0f, 200.0f, 320.0f, 18.0f, "V"},
};

const Preset kPresets[] = {
    {{657.0f, 100.0f, 50.0f, 34.0f}, "Red",   {1.0f, 0.0f, 0.0f}},
    {{718.0f, 100.0f, 50.0f, 34.0f}, "Green", {0.0f, 1.0f, 0.0f}},
    {{779.0f, 100.0f, 50.0f, 34.0f}, "Blue",  {0.0f, 0.0f, 1.0f}},
    {{840.0f, 100.0f, 50.0f, 34.0f}, "White", {1.0f, 1.0f, 1.0f}},
    {{901.0f, 100.0f, 50.0f, 34.0f}, "Black", {0.0f, 0.0f, 0.0f}},
};

bool contains(const Rect& rect, float x, float y) {
    return x >= rect.x && x <= rect.x + rect.width &&
           y >= rect.y && y <= rect.y + rect.height;
}

void drawRect(float x, float y, float width, float height,
              float red, float green, float blue) {
    glColor3f(red, green, blue);
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();
}

void drawBorder(float x, float y, float width, float height,
                float red, float green, float blue) {
    glColor3f(red, green, blue);
    glLineWidth(1.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y);
    glVertex2f(x + width, y);
    glVertex2f(x + width, y + height);
    glVertex2f(x, y + height);
    glEnd();
}

void drawPanel(const Rect& rect) {
    drawRect(rect.x, rect.y, rect.width, rect.height,
             0.075f, 0.095f, 0.145f);
    drawBorder(rect.x, rect.y, rect.width, rect.height,
               0.22f, 0.28f, 0.38f);
}

void drawText(float x, float y, const std::string& text,
              void* font = GLUT_BITMAP_HELVETICA_18) {
    glColor3f(0.91f, 0.94f, 1.0f);
    glRasterPos2f(x, y);
    for (unsigned char character : text) {
        glutBitmapCharacter(font, character);
    }
}

void drawMutedText(float x, float y, const std::string& text) {
    glColor3f(0.65f, 0.72f, 0.83f);
    glRasterPos2f(x, y);
    for (unsigned char character : text) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, character);
    }
}

colour::RGBColor sliderGradientColor(const Slider& slider, float amount) {
    switch (slider.type) {
        case SliderType::Red:
            return {amount, 0.0f, 0.0f};
        case SliderType::Green:
            return {0.0f, amount, 0.0f};
        case SliderType::Blue:
            return {0.0f, 0.0f, amount};
        case SliderType::Hue:
            return colour::hsvToRgb({amount * 360.0f, 1.0f, 1.0f});
        case SliderType::Saturation:
            return colour::hsvToRgb({gHSV.h, amount, gHSV.v});
        case SliderType::Value:
            return colour::hsvToRgb({gHSV.h, gHSV.s, amount});
    }
    return {0.0f, 0.0f, 0.0f};
}

float sliderValue(const Slider& slider) {
    switch (slider.type) {
        case SliderType::Red:        return gRGB.r;
        case SliderType::Green:      return gRGB.g;
        case SliderType::Blue:       return gRGB.b;
        case SliderType::Hue:        return gHSV.h / 360.0f;
        case SliderType::Saturation: return gHSV.s;
        case SliderType::Value:      return gHSV.v;
    }
    return 0.0f;
}

void drawSlider(const Slider& slider) {
    drawText(55.0f, slider.y + 3.0f, slider.label);

    constexpr int kGradientSteps = 100;
    glBegin(GL_QUADS);
    for (int i = 0; i < kGradientSteps; ++i) {
        const float from = static_cast<float>(i) / kGradientSteps;
        const float to = static_cast<float>(i + 1) / kGradientSteps;
        const colour::RGBColor color = sliderGradientColor(slider, (from + to) * 0.5f);
        glColor3f(color.r, color.g, color.b);
        const float left = slider.x + slider.width * from;
        const float right = slider.x + slider.width * to;
        glVertex2f(left, slider.y);
        glVertex2f(right, slider.y);
        glVertex2f(right, slider.y + slider.height);
        glVertex2f(left, slider.y + slider.height);
    }
    glEnd();
    drawBorder(slider.x, slider.y, slider.width, slider.height,
               0.02f, 0.03f, 0.06f);

    const float knobX = slider.x + slider.width * sliderValue(slider);
    drawRect(knobX - 3.0f, slider.y - 4.0f, 6.0f, slider.height + 8.0f,
             0.96f, 0.97f, 1.0f);
    drawBorder(knobX - 3.0f, slider.y - 4.0f, 6.0f, slider.height + 8.0f,
               0.10f, 0.12f, 0.18f);

    char valueText[32];
    switch (slider.type) {
        case SliderType::Red:
            std::snprintf(valueText, sizeof(valueText), "%3d / 255",
                          static_cast<int>(std::lround(gRGB.r * 255.0f)));
            break;
        case SliderType::Green:
            std::snprintf(valueText, sizeof(valueText), "%3d / 255",
                          static_cast<int>(std::lround(gRGB.g * 255.0f)));
            break;
        case SliderType::Blue:
            std::snprintf(valueText, sizeof(valueText), "%3d / 255",
                          static_cast<int>(std::lround(gRGB.b * 255.0f)));
            break;
        case SliderType::Hue:
            std::snprintf(valueText, sizeof(valueText), "%3.0f deg", gHSV.h);
            break;
        case SliderType::Saturation:
            std::snprintf(valueText, sizeof(valueText), "%3.0f%%", gHSV.s * 100.0f);
            break;
        case SliderType::Value:
            std::snprintf(valueText, sizeof(valueText), "%3.0f%%", gHSV.v * 100.0f);
            break;
    }
    drawMutedText(555.0f, slider.y + 3.0f, valueText);
}

void setRGB(colour::RGBColor rgb) {
    gRGB = colour::clampRGB(rgb);
    gHSV = colour::rgbToHsv(gRGB);
}

void setSliderFromX(int sliderIndex, float canvasX) {
    if (sliderIndex < 0 ||
        sliderIndex >= static_cast<int>(sizeof(kSliders) / sizeof(kSliders[0]))) {
        return;
    }

    const Slider& slider = kSliders[sliderIndex];
    const float amount = colour::clamp01((canvasX - slider.x) / slider.width);
    switch (slider.type) {
        case SliderType::Red:
            gRGB.r = amount;
            gHSV = colour::rgbToHsv(gRGB);
            break;
        case SliderType::Green:
            gRGB.g = amount;
            gHSV = colour::rgbToHsv(gRGB);
            break;
        case SliderType::Blue:
            gRGB.b = amount;
            gHSV = colour::rgbToHsv(gRGB);
            break;
        case SliderType::Hue:
            gHSV.h = amount * 360.0f;
            gRGB = colour::hsvToRgb(gHSV);
            break;
        case SliderType::Saturation:
            gHSV.s = amount;
            gRGB = colour::hsvToRgb(gHSV);
            break;
        case SliderType::Value:
            gHSV.v = amount;
            gRGB = colour::hsvToRgb(gHSV);
            break;
    }
    glutPostRedisplay();
}

void drawRGBPreview() {
    drawPanel({640.0f, 382.0f, 330.0f, 230.0f});
    drawText(660.0f, 585.0f, "SELECTED COLOUR");

    drawRect(660.0f, 440.0f, 290.0f, 128.0f,
             gRGB.r, gRGB.g, gRGB.b);
    drawBorder(660.0f, 440.0f, 290.0f, 128.0f,
               0.94f, 0.96f, 1.0f);

    char rgbText[96];
    char hsvText[96];
    char hexText[32];
    const int red = static_cast<int>(std::lround(gRGB.r * 255.0f));
    const int green = static_cast<int>(std::lround(gRGB.g * 255.0f));
    const int blue = static_cast<int>(std::lround(gRGB.b * 255.0f));
    std::snprintf(rgbText, sizeof(rgbText), "RGB: %3d, %3d, %3d", red, green, blue);
    std::snprintf(hsvText, sizeof(hsvText), "HSV: %3.0f deg, %3.0f%%, %3.0f%%",
                  gHSV.h, gHSV.s * 100.0f, gHSV.v * 100.0f);
    std::snprintf(hexText, sizeof(hexText), "#%02X%02X%02X", red, green, blue);
    drawText(660.0f, 418.0f, rgbText, GLUT_BITMAP_HELVETICA_12);
    drawText(660.0f, 400.0f, hsvText, GLUT_BITMAP_HELVETICA_12);
    drawText(866.0f, 400.0f, hexText, GLUT_BITMAP_HELVETICA_12);
}

void drawRGBMixingPanel() {
    drawPanel({640.0f, 185.0f, 330.0f, 185.0f});
    drawText(660.0f, 344.0f, "ADDITIVE RGB CHANNELS");

    const float xPositions[] = {663.0f, 770.0f, 877.0f};
    const char* labels[] = {"R contribution", "G contribution", "B contribution"};
    const float values[] = {gRGB.r, gRGB.g, gRGB.b};

    for (int channel = 0; channel < 3; ++channel) {
        const colour::RGBColor contribution =
            colour::channelContribution(gRGB, channel);
        drawRect(xPositions[channel], 266.0f, 74.0f, 55.0f,
                 contribution.r, contribution.g, contribution.b);
        drawBorder(xPositions[channel], 266.0f, 74.0f, 55.0f,
                   0.52f, 0.59f, 0.70f);

        drawMutedText(xPositions[channel], 247.0f, labels[channel]);
        char amount[24];
        std::snprintf(amount, sizeof(amount), "%3d / 255",
                      static_cast<int>(std::lround(values[channel] * 255.0f)));
        drawMutedText(xPositions[channel], 230.0f, amount);
    }

    drawMutedText(660.0f, 203.0f,
                  "The three light channels combine in the preview above.");
}

void drawPresets() {
    drawPanel({640.0f, 65.0f, 330.0f, 105.0f});
    drawText(660.0f, 145.0f, "COLOUR PRESETS");

    for (const Preset& preset : kPresets) {
        drawRect(preset.bounds.x, preset.bounds.y,
                 preset.bounds.width, preset.bounds.height,
                 preset.color.r, preset.color.g, preset.color.b);
        drawBorder(preset.bounds.x, preset.bounds.y,
                   preset.bounds.width, preset.bounds.height,
                   0.78f, 0.83f, 0.91f);
        drawMutedText(preset.bounds.x + 4.0f, 84.0f, preset.label);
    }
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    drawText(35.0f, 665.0f, "COLOUR MODEL VISUALIZER");
    drawMutedText(35.0f, 640.0f,
                  "Drag RGB or HSV sliders to edit the same colour.  Click a preset.  R: reset  Esc: quit");
    drawMutedText(640.0f, 640.0f,
                  "RGB is additive; HSV is another representation of RGB.");

    drawPanel({30.0f, 395.0f, 580.0f, 215.0f});
    drawText(55.0f, 585.0f, "RGB COMPONENTS");
    drawMutedText(55.0f, 563.0f, "Each slider controls one channel from 0 to 255.");

    drawPanel({30.0f, 150.0f, 580.0f, 225.0f});
    drawText(55.0f, 348.0f, "HSV COMPONENTS");
    drawMutedText(55.0f, 326.0f, "Hue is in degrees; saturation and value are percentages.");

    for (const Slider& slider : kSliders) {
        drawSlider(slider);
    }

    drawRGBPreview();
    drawRGBMixingPanel();
    drawPresets();

    glutSwapBuffers();
}

void reshape(int width, int height) {
    gWindowWidth = std::max(1, width);
    gWindowHeight = std::max(1, height);

    const float scale = std::min(
        static_cast<float>(gWindowWidth) / kCanvasWidth,
        static_cast<float>(gWindowHeight) / kCanvasHeight);
    gViewportWidth = static_cast<int>(std::lround(kCanvasWidth * scale));
    gViewportHeight = static_cast<int>(std::lround(kCanvasHeight * scale));
    gViewportX = (gWindowWidth - gViewportWidth) / 2;
    gViewportY = (gWindowHeight - gViewportHeight) / 2;

    glViewport(gViewportX, gViewportY, gViewportWidth, gViewportHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, kCanvasWidth, 0.0, kCanvasHeight, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
}

bool windowToCanvas(int windowX, int windowY, float& canvasX, float& canvasY) {
    const float bottomY = static_cast<float>(gWindowHeight - windowY);
    if (windowX < gViewportX || windowX > gViewportX + gViewportWidth ||
        bottomY < gViewportY || bottomY > gViewportY + gViewportHeight) {
        return false;
    }

    canvasX = (static_cast<float>(windowX - gViewportX) * kCanvasWidth) /
              static_cast<float>(gViewportWidth);
    canvasY = ((bottomY - gViewportY) * kCanvasHeight) /
              static_cast<float>(gViewportHeight);
    return true;
}

int sliderAt(float x, float y) {
    const int count = static_cast<int>(sizeof(kSliders) / sizeof(kSliders[0]));
    for (int i = 0; i < count; ++i) {
        const Slider& slider = kSliders[i];
        const Rect hitArea = {
            slider.x - 8.0f,
            slider.y - 10.0f,
            slider.width + 16.0f,
            slider.height + 20.0f,
        };
        if (contains(hitArea, x, y)) {
            return i;
        }
    }
    return -1;
}

void mouse(int button, int state, int windowX, int windowY) {
    if (button != GLUT_LEFT_BUTTON) {
        return;
    }

    float canvasX = 0.0f;
    float canvasY = 0.0f;
    if (!windowToCanvas(windowX, windowY, canvasX, canvasY)) {
        if (state == GLUT_UP) {
            gActiveSlider = -1;
        }
        return;
    }

    if (state == GLUT_DOWN) {
        for (const Preset& preset : kPresets) {
            if (contains(preset.bounds, canvasX, canvasY)) {
                setRGB(preset.color);
                glutPostRedisplay();
                return;
            }
        }

        gActiveSlider = sliderAt(canvasX, canvasY);
        if (gActiveSlider >= 0) {
            setSliderFromX(gActiveSlider, canvasX);
        }
    } else if (state == GLUT_UP) {
        gActiveSlider = -1;
    }
}

void motion(int windowX, int windowY) {
    if (gActiveSlider < 0) {
        return;
    }

    float canvasX = 0.0f;
    float canvasY = 0.0f;
    if (windowToCanvas(windowX, windowY, canvasX, canvasY)) {
        setSliderFromX(gActiveSlider, canvasX);
    } else {
        // Keep dragging responsive if the pointer briefly leaves the viewport.
        const float scaleX = static_cast<float>(gViewportWidth) / kCanvasWidth;
        canvasX = (static_cast<float>(windowX - gViewportX)) / scaleX;
        setSliderFromX(gActiveSlider, canvasX);
    }
}

void keyboard(unsigned char key, int, int) {
    if (key == 27) {
        std::exit(0);
    }
    if (key == 'r' || key == 'R') {
        setRGB({0.20f, 0.55f, 0.85f});
        glutPostRedisplay();
    }
}

void initialize() {
    glClearColor(0.025f, 0.035f, 0.065f, 1.0f);
}

}  // namespace

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(kWindowWidth, kWindowHeight);
    glutCreateWindow("Colour Model Visualizer - RGB and HSV");

    initialize();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutKeyboardFunc(keyboard);
    glutMainLoop();
    return 0;
}
