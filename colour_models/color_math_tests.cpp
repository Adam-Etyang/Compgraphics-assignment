#include "color_math.h"

#include <cmath>
#include <iostream>
#include <string>

namespace {

int gFailures = 0;

void expectNear(const std::string& name, float actual, float expected,
                float tolerance = 1.0e-4f) {
    if (std::fabs(actual - expected) > tolerance) {
        std::cerr << "FAIL: " << name << " (expected " << expected
                  << ", got " << actual << ")\n";
        ++gFailures;
    }
}

void expectRGBNear(const std::string& name, colour::RGBColor actual,
                   colour::RGBColor expected) {
    expectNear(name + ".r", actual.r, expected.r);
    expectNear(name + ".g", actual.g, expected.g);
    expectNear(name + ".b", actual.b, expected.b);
}

void testPrimaryAndSecondaryHues() {
    const colour::HSVColor red = colour::rgbToHsv({1.0f, 0.0f, 0.0f});
    expectNear("red hue", red.h, 0.0f);
    expectNear("red saturation", red.s, 1.0f);
    expectNear("red value", red.v, 1.0f);

    expectNear("green hue", colour::rgbToHsv({0.0f, 1.0f, 0.0f}).h, 120.0f);
    expectNear("blue hue", colour::rgbToHsv({0.0f, 0.0f, 1.0f}).h, 240.0f);
    expectNear("yellow hue", colour::rgbToHsv({1.0f, 1.0f, 0.0f}).h, 60.0f);
    expectNear("cyan hue", colour::rgbToHsv({0.0f, 1.0f, 1.0f}).h, 180.0f);
    expectNear("magenta hue", colour::rgbToHsv({1.0f, 0.0f, 1.0f}).h, 300.0f);
}

void testNeutralColours() {
    const colour::HSVColor black = colour::rgbToHsv({0.0f, 0.0f, 0.0f});
    expectNear("black hue", black.h, 0.0f);
    expectNear("black saturation", black.s, 0.0f);
    expectNear("black value", black.v, 0.0f);

    const colour::HSVColor gray = colour::rgbToHsv({0.42f, 0.42f, 0.42f});
    expectNear("gray hue convention", gray.h, 0.0f);
    expectNear("gray saturation", gray.s, 0.0f);
    expectNear("gray value", gray.v, 0.42f);

    expectRGBNear("white conversion", colour::hsvToRgb({0.0f, 0.0f, 1.0f}),
                  {1.0f, 1.0f, 1.0f});
    expectRGBNear("black conversion", colour::hsvToRgb({230.0f, 1.0f, 0.0f}),
                  {0.0f, 0.0f, 0.0f});
}

void testRoundTripsAndHueWrapping() {
    const colour::RGBColor samples[] = {
        {0.20f, 0.45f, 0.80f},
        {0.90f, 0.25f, 0.55f},
        {0.35f, 0.35f, 0.35f},
        {0.02f, 0.80f, 0.40f},
    };
    for (int i = 0; i < 4; ++i) {
        const colour::RGBColor roundTrip =
            colour::hsvToRgb(colour::rgbToHsv(samples[i]));
        expectRGBNear("RGB/HSV round trip " + std::to_string(i),
                      roundTrip, samples[i]);
    }

    expectRGBNear("hue 360 wraps to red",
                  colour::hsvToRgb({360.0f, 1.0f, 1.0f}),
                  {1.0f, 0.0f, 0.0f});
    expectRGBNear("negative hue wraps to blue",
                  colour::hsvToRgb({-120.0f, 1.0f, 1.0f}),
                  {0.0f, 0.0f, 1.0f});
}

}  // namespace

int main() {
    testPrimaryAndSecondaryHues();
    testNeutralColours();
    testRoundTripsAndHueWrapping();

    if (gFailures != 0) {
        std::cerr << gFailures << " colour conversion test(s) failed.\n";
        return 1;
    }

    std::cout << "All colour conversion tests passed.\n";
    return 0;
}
