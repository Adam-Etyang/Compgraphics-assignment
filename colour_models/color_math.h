#pragma once

#include <algorithm>
#include <cmath>

namespace colour {

struct RGBColor {
    float r;
    float g;
    float b;
};

// Hue is measured in degrees (360 is accepted as equivalent to 0);
// saturation and value are in [0, 1].
struct HSVColor {
    float h;
    float s;
    float v;
};

inline float clamp01(float value) {
    return std::max(0.0f, std::min(1.0f, value));
}

inline RGBColor clampRGB(RGBColor rgb) {
    return {clamp01(rgb.r), clamp01(rgb.g), clamp01(rgb.b)};
}

inline HSVColor rgbToHsv(RGBColor input) {
    const RGBColor rgb = clampRGB(input);
    const float maximum = std::max(rgb.r, std::max(rgb.g, rgb.b));
    const float minimum = std::min(rgb.r, std::min(rgb.g, rgb.b));
    const float delta = maximum - minimum;

    HSVColor hsv = {0.0f, 0.0f, maximum};
    if (delta <= 1.0e-6f) {
        // Hue is undefined for a neutral colour; use zero by convention.
        return hsv;
    }

    if (maximum == rgb.r) {
        hsv.h = 60.0f * std::fmod((rgb.g - rgb.b) / delta, 6.0f);
    } else if (maximum == rgb.g) {
        hsv.h = 60.0f * (((rgb.b - rgb.r) / delta) + 2.0f);
    } else {
        hsv.h = 60.0f * (((rgb.r - rgb.g) / delta) + 4.0f);
    }

    if (hsv.h < 0.0f) {
        hsv.h += 360.0f;
    }
    hsv.s = maximum > 1.0e-6f ? delta / maximum : 0.0f;
    return hsv;
}

inline RGBColor hsvToRgb(HSVColor input) {
    float hue = std::fmod(input.h, 360.0f);
    if (hue < 0.0f) {
        hue += 360.0f;
    }
    const float saturation = clamp01(input.s);
    const float value = clamp01(input.v);

    const float chroma = value * saturation;
    const float sector = hue / 60.0f;
    const float x = chroma * (1.0f - std::fabs(std::fmod(sector, 2.0f) - 1.0f));
    const float offset = value - chroma;

    RGBColor rgb;
    if (hue < 60.0f) {
        rgb = {chroma, x, 0.0f};
    } else if (hue < 120.0f) {
        rgb = {x, chroma, 0.0f};
    } else if (hue < 180.0f) {
        rgb = {0.0f, chroma, x};
    } else if (hue < 240.0f) {
        rgb = {0.0f, x, chroma};
    } else if (hue < 300.0f) {
        rgb = {x, 0.0f, chroma};
    } else {
        rgb = {chroma, 0.0f, x};
    }

    return clampRGB({rgb.r + offset, rgb.g + offset, rgb.b + offset});
}

inline RGBColor channelContribution(RGBColor rgb, int channel) {
    switch (channel) {
        case 0: return {clamp01(rgb.r), 0.0f, 0.0f};
        case 1: return {0.0f, clamp01(rgb.g), 0.0f};
        default: return {0.0f, 0.0f, clamp01(rgb.b)};
    }
}

}  // namespace colour
