/*
 * Rocket Flight: a short 3D animated scene for the computer graphics project.
 *
 * Controls:
 *   Space - start / pause the animation
 *   R     - reset to the beginning
 *   Esc   - quit
 *
 * The scene is built from OpenGL/GLUT primitives. It demonstrates 3D modeling,
 * hierarchical transformations, keyframe interpolation, lighting, and camera
 * projection without relying on imported models or pre-rendered animation.
 */

#if defined(__APPLE__)
// The assignment permits traditional OpenGL; silence Apple's deprecation
// annotations for that compatibility API while keeping compiler output
// readable.
#define GL_SILENCE_DEPRECATION
#include <GLUT/glut.h>
#include <OpenGL/glu.h>
#else
#include <GL/glut.h>
#include <GL/glu.h>
#endif

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <vector>

namespace {

constexpr float kPi = 3.14159265358979323846f;
constexpr float kAnimationDuration = 13.5f;

struct Vec3 {
    float x;
    float y;
    float z;
};

struct Keyframe {
    float time;
    Vec3 position;
    float tiltDegrees;
    float scale;
};

struct Star {
    float x;
    float y;
    float z;
};

const std::vector<Keyframe> kRocketKeyframes = {
    {0.0f,  {-5.5f, 0.24f, 0.0f},   0.0f,   1.00f},
    {2.0f,  {-5.5f, 0.24f, 0.0f},   0.0f,   1.00f},
    {5.2f,  {-5.1f, 7.8f,  0.0f},  -3.0f,   1.01f},
    {8.8f,  {-1.3f, 13.6f, 0.0f}, -15.0f,   1.03f},
    {11.5f, {5.8f,  17.1f, 0.0f}, -10.0f,   1.01f},
    {13.5f, {5.8f,  17.1f, 0.0f}, -10.0f,   1.01f},
};

int gWindowWidth = 1000;
int gWindowHeight = 700;
bool gRunning = false;
float gAnimationTime = 0.0f;
int gLastTickMilliseconds = 0;
GLUquadric* gQuadric = nullptr;
std::vector<Star> gStars;

float clamp01(float value) {
    return std::max(0.0f, std::min(1.0f, value));
}

float smoothStep(float value) {
    const float u = clamp01(value);
    return u * u * (3.0f - 2.0f * u);
}

Vec3 interpolate(const Vec3& a, const Vec3& b, float amount) {
    return {
        a.x + (b.x - a.x) * amount,
        a.y + (b.y - a.y) * amount,
        a.z + (b.z - a.z) * amount,
    };
}

Keyframe rocketPoseAt(float time) {
    if (time <= kRocketKeyframes.front().time) {
        return kRocketKeyframes.front();
    }

    for (std::size_t i = 0; i + 1 < kRocketKeyframes.size(); ++i) {
        const Keyframe& start = kRocketKeyframes[i];
        const Keyframe& end = kRocketKeyframes[i + 1];
        if (time <= end.time) {
            const float duration = end.time - start.time;
            const float linearAmount = duration > 0.0f
                ? (time - start.time) / duration
                : 1.0f;
            const float amount = smoothStep(linearAmount);
            return {
                time,
                interpolate(start.position, end.position, amount),
                start.tiltDegrees + (end.tiltDegrees - start.tiltDegrees) * amount,
                start.scale + (end.scale - start.scale) * amount,
            };
        }
    }

    return kRocketKeyframes.back();
}

void setMaterialColor(float red, float green, float blue) {
    glColor3f(red, green, blue);
}

void drawCube(float x, float y, float z,
              float scaleX, float scaleY, float scaleZ,
              float red, float green, float blue) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glScalef(scaleX, scaleY, scaleZ);
    setMaterialColor(red, green, blue);
    glutSolidCube(1.0);
    glPopMatrix();
}

void drawStars() {
    glDisable(GL_LIGHTING);
    glPointSize(2.0f);
    glColor3f(0.78f, 0.84f, 1.0f);
    glBegin(GL_POINTS);
    for (const Star& star : gStars) {
        glVertex3f(star.x, star.y, star.z);
    }
    glEnd();
    glEnable(GL_LIGHTING);
}

void drawGround() {
    glNormal3f(0.0f, 1.0f, 0.0f);
    setMaterialColor(0.10f, 0.13f, 0.18f);
    glBegin(GL_QUADS);
    glVertex3f(-35.0f, -0.08f, -25.0f);
    glVertex3f( 35.0f, -0.08f, -25.0f);
    glVertex3f( 35.0f, -0.08f,  25.0f);
    glVertex3f(-35.0f, -0.08f,  25.0f);
    glEnd();

    // Simple runway markings help establish the launch area.
    glDisable(GL_LIGHTING);
    glColor3f(0.33f, 0.39f, 0.47f);
    glBegin(GL_LINES);
    for (int i = -5; i <= 5; ++i) {
        glVertex3f(-5.5f + static_cast<float>(i) * 1.15f, -0.06f, -8.0f);
        glVertex3f(-5.5f + static_cast<float>(i) * 1.15f, -0.06f,  8.0f);
    }
    glEnd();
    glEnable(GL_LIGHTING);
}

void drawLaunchPad() {
    drawCube(-5.5f, 0.12f, 0.0f, 4.0f, 0.24f, 4.0f,
             0.30f, 0.34f, 0.40f);

    // The launch tower is made of stacked beams and crossbars.
    drawCube(-7.35f, 2.05f, -0.45f, 0.36f, 4.1f, 0.36f,
             0.72f, 0.76f, 0.80f);
    drawCube(-7.35f, 2.05f,  0.45f, 0.36f, 4.1f, 0.36f,
             0.72f, 0.76f, 0.80f);
    drawCube(-7.35f, 0.35f, 0.0f, 0.42f, 0.25f, 1.15f,
             0.85f, 0.31f, 0.12f);
    drawCube(-7.35f, 1.20f, 0.0f, 0.42f, 0.18f, 1.15f,
             0.85f, 0.31f, 0.12f);
    drawCube(-7.35f, 2.10f, 0.0f, 0.42f, 0.18f, 1.15f,
             0.85f, 0.31f, 0.12f);
    drawCube(-7.35f, 3.00f, 0.0f, 0.42f, 0.18f, 1.15f,
             0.85f, 0.31f, 0.12f);
}

void drawPlanet() {
    glPushMatrix();
    glTranslatef(13.0f, 20.0f, -8.0f);
    setMaterialColor(0.12f, 0.34f, 0.70f);
    glutSolidSphere(3.8, 36, 28);

    // A few raised green patches make the planet easier to read as a world.
    glPushMatrix();
    glTranslatef(-0.9f, 0.6f, 3.77f);
    glScalef(1.0f, 0.40f, 0.12f);
    setMaterialColor(0.20f, 0.55f, 0.32f);
    glutSolidSphere(1.0, 18, 14);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(1.5f, -0.8f, 3.78f);
    glScalef(0.65f, 0.35f, 0.12f);
    setMaterialColor(0.20f, 0.55f, 0.32f);
    glutSolidSphere(1.0, 18, 14);
    glPopMatrix();
    glPopMatrix();
}

void drawSatellite(float time) {
    glPushMatrix();
    glTranslatef(10.0f, 18.6f, -2.8f);
    glRotatef(time * 8.0f, 0.0f, 1.0f, 0.0f);

    drawCube(0.0f, 0.0f, 0.0f, 0.90f, 0.62f, 0.62f,
             0.76f, 0.67f, 0.36f);
    drawCube(-1.10f, 0.0f, 0.0f, 1.25f, 0.12f, 0.72f,
             0.15f, 0.38f, 0.82f);
    drawCube( 1.10f, 0.0f, 0.0f, 1.25f, 0.12f, 0.72f,
             0.15f, 0.38f, 0.82f);

    glPushMatrix();
    glTranslatef(0.0f, 0.46f, 0.0f);
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    setMaterialColor(0.82f, 0.82f, 0.76f);
    gluCylinder(gQuadric, 0.045, 0.025, 0.58, 10, 3);
    glPopMatrix();

    glPopMatrix();
}

void drawFin() {
    // One triangular fin; the caller rotates copies around the rocket body.
    glBegin(GL_TRIANGLES);
    glNormal3f(0.0f, 0.0f, 1.0f);
    glVertex3f(0.42f, 0.62f, 0.0f);
    glVertex3f(1.00f, -0.46f, 0.0f);
    glVertex3f(0.42f, -0.46f, 0.0f);
    glEnd();
}

void drawRocket(float time, const Keyframe& pose) {
    glPushMatrix();
    glTranslatef(pose.position.x, pose.position.y, pose.position.z);
    glRotatef(pose.tiltDegrees, 0.0f, 0.0f, 1.0f);
    glScalef(pose.scale, pose.scale, pose.scale);

    // Main cylinder: GLU cylinders point along local +Z, so rotate to +Y.
    glPushMatrix();
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    setMaterialColor(0.82f, 0.85f, 0.88f);
    gluCylinder(gQuadric, 0.48, 0.48, 2.35, 28, 8);
    glPopMatrix();

    // Nose cone.
    glPushMatrix();
    glTranslatef(0.0f, 2.32f, 0.0f);
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    setMaterialColor(0.86f, 0.20f, 0.13f);
    glutSolidCone(0.48, 0.95, 28, 8);
    glPopMatrix();

    // Coloured bands and a porthole.
    glPushMatrix();
    glTranslatef(0.0f, 0.68f, 0.0f);
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    setMaterialColor(0.86f, 0.20f, 0.13f);
    gluCylinder(gQuadric, 0.49, 0.49, 0.20, 28, 2);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.0f, 1.72f, 0.45f);
    glScalef(0.20f, 0.20f, 0.09f);
    setMaterialColor(0.16f, 0.65f, 0.89f);
    glutSolidSphere(1.0, 20, 16);
    glPopMatrix();

    setMaterialColor(0.82f, 0.20f, 0.13f);
    for (int i = 0; i < 4; ++i) {
        glPushMatrix();
        glRotatef(static_cast<float>(i) * 90.0f, 0.0f, 1.0f, 0.0f);
        drawFin();
        glPopMatrix();
    }

    // The flame begins at liftoff and pulses by scaling its length.
    if (time >= 2.0f && time < 11.8f) {
        const float pulse = 0.82f + 0.22f * (0.5f + 0.5f * std::sin(time * 17.0f));
        glPushMatrix();
        glTranslatef(0.0f, 0.08f, 0.0f);
        glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
        glScalef(1.0f, 1.0f, pulse);
        setMaterialColor(1.0f, 0.30f, 0.035f);
        glutSolidCone(0.40, 1.25, 20, 6);
        glTranslatef(0.0f, 0.0f, 0.10f);
        setMaterialColor(1.0f, 0.78f, 0.15f);
        glutSolidCone(0.22, 0.83, 18, 5);
        glPopMatrix();
    }

    glPopMatrix();
}

const char* phaseLabel() {
    if (gAnimationTime < 0.01f) {
        return "READY FOR LAUNCH";
    }
    if (gAnimationTime < 2.0f) {
        return "SYSTEMS CHECK";
    }
    if (gAnimationTime < 5.2f) {
        return "LIFTOFF";
    }
    if (gAnimationTime < 8.8f) {
        return "ASCENT";
    }
    if (gAnimationTime < 11.5f) {
        return "ORBITAL APPROACH";
    }
    return "MISSION COMPLETE";
}

void drawBitmapText(float x, float y, const char* text) {
    glRasterPos2f(x, y);
    for (const char* character = text; *character != '\0'; ++character) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *character);
    }
}

void drawHud() {
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0.0, static_cast<double>(gWindowWidth),
               0.0, static_cast<double>(gWindowHeight));

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glColor3f(0.93f, 0.96f, 1.0f);
    drawBitmapText(22.0f, static_cast<float>(gWindowHeight - 32),
                   "ROCKET FLIGHT  |  SPACE: start/pause   R: reset   ESC: quit");

    char status[128];
    if (gRunning) {
        std::snprintf(status, sizeof(status), "%s  |  %4.1f s",
                      phaseLabel(), gAnimationTime);
    } else if (gAnimationTime >= kAnimationDuration) {
        std::snprintf(status, sizeof(status), "%s  |  Press R to replay",
                      phaseLabel());
    } else if (gAnimationTime > 0.0f) {
        std::snprintf(status, sizeof(status), "PAUSED  |  %s",
                      phaseLabel());
    } else {
        std::snprintf(status, sizeof(status), "%s",
                      phaseLabel());
    }
    glColor3f(1.0f, 0.77f, 0.35f);
    drawBitmapText(22.0f, static_cast<float>(gWindowHeight - 58), status);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(5.0, 14.5, 40.0,
              1.0, 10.0, 0.0,
              0.0, 1.0, 0.0);

    const GLfloat lightPosition[] = {-8.0f, 26.0f, 18.0f, 1.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

    drawStars();
    drawGround();
    drawPlanet();
    drawSatellite(gAnimationTime);
    drawLaunchPad();
    drawRocket(gAnimationTime, rocketPoseAt(gAnimationTime));
    drawHud();

    glutSwapBuffers();
}

void reshape(int width, int height) {
    gWindowWidth = std::max(1, width);
    gWindowHeight = std::max(1, height);
    glViewport(0, 0, gWindowWidth, gWindowHeight);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(48.0,
                   static_cast<double>(gWindowWidth) /
                       static_cast<double>(gWindowHeight),
                   0.1, 150.0);
    glMatrixMode(GL_MODELVIEW);
}

void updateAnimation() {
    if (!gRunning) {
        return;
    }

    const int now = glutGet(GLUT_ELAPSED_TIME);
    const float deltaSeconds =
        static_cast<float>(now - gLastTickMilliseconds) / 1000.0f;
    gLastTickMilliseconds = now;
    gAnimationTime = std::min(kAnimationDuration,
                              gAnimationTime + deltaSeconds);

    if (gAnimationTime >= kAnimationDuration) {
        gRunning = false;
    }
    glutPostRedisplay();
}

void keyboard(unsigned char key, int, int) {
    if (key == 27) {
        if (gQuadric != nullptr) {
            gluDeleteQuadric(gQuadric);
        }
        std::exit(0);
    }

    if (key == ' ' || key == 'p' || key == 'P') {
        if (!gRunning && gAnimationTime >= kAnimationDuration) {
            gAnimationTime = 0.0f;
        }
        gRunning = !gRunning;
        gLastTickMilliseconds = glutGet(GLUT_ELAPSED_TIME);
        glutPostRedisplay();
    } else if (key == 'r' || key == 'R') {
        gAnimationTime = 0.0f;
        gRunning = false;
        gLastTickMilliseconds = glutGet(GLUT_ELAPSED_TIME);
        glutPostRedisplay();
    }
}

void initializeScene() {
    glClearColor(0.015f, 0.025f, 0.075f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_NORMALIZE);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    const GLfloat ambient[] = {0.24f, 0.24f, 0.30f, 1.0f};
    const GLfloat diffuse[] = {0.95f, 0.90f, 0.78f, 1.0f};
    const GLfloat specular[] = {0.75f, 0.75f, 0.75f, 1.0f};
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 35.0f);

    gQuadric = gluNewQuadric();
    if (gQuadric != nullptr) {
        gluQuadricNormals(gQuadric, GLU_SMOOTH);
    }

    std::mt19937 randomGenerator(17);
    std::uniform_real_distribution<float> xDistribution(-24.0f, 24.0f);
    std::uniform_real_distribution<float> yDistribution(-2.0f, 34.0f);
    std::uniform_real_distribution<float> zDistribution(-22.0f, -16.0f);
    for (int i = 0; i < 150; ++i) {
        gStars.push_back({
            xDistribution(randomGenerator),
            yDistribution(randomGenerator),
            zDistribution(randomGenerator),
        });
    }
}

}  // namespace

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(gWindowWidth, gWindowHeight);
    glutCreateWindow("Rocket Flight - OpenGL Animation");

    initializeScene();
    gLastTickMilliseconds = glutGet(GLUT_ELAPSED_TIME);

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutIdleFunc(updateAnimation);

    glutMainLoop();
    return 0;
}
