#include "Player.h"
#include "Level4Logic.h"
#include "Constants.h"
#include <GL/freeglut.h>
#include <cmath>
#include <cstdlib>

// ─── Moving Cloud Platforms ─────────────────────────────────────────────────
float cloudPlat1X = 160.0f;
float cloudPlat1Y = 340.0f;
float cloudPlat1Speed = 1.0f;

float cloudPlat2X = 400.0f;
float cloudPlat2Y = 240.0f;
float cloudPlat2Speed = -1.3f;

float cloudPlat3X = 580.0f;
float cloudPlat3Y = 160.0f;
float cloudPlat3Speed = 1.6f;

// ─── Portal ──────────────────────────────────────────────────────────────────
float l4PortalX    = 720.0f;
float l4PortalY    = 120.0f;
float l4PortalBaseY = 120.0f;
float l4PortalTime  = 0.0f;

// ─── Level state ────────────────────────────────────────────────────────────
static bool level4Complete = false;

// ─── Birds (hazards, like meteors in Level 3) ───────────────────────────────
struct Bird {
    float x;
    float y;
    float speed;
    bool  active;
};

const int BIRD_COUNT = 6;
Bird birds[BIRD_COUNT];

// ─── Background clouds (decorative, parallax feel) ──────────────────────────
struct BgCloud {
    float x;
    float y;
    float speed;
    float width;
};

const int BGCLOUD_COUNT = 8;
BgCloud bgClouds[BGCLOUD_COUNT];

// ─── Stars / sparkles ───────────────────────────────────────────────────────
struct Sparkle {
    float x;
    float y;
};

const int SPARKLE_COUNT = 60;
Sparkle sparkles[SPARKLE_COUNT];

// ────────────────────────────────────────────────────────────────────────────
void resetLevel4State() {
    level4Complete = false;
    l4PortalTime   = 0.0f;

    // Initialise birds flying left-to-right across the screen
    for (int i = 0; i < BIRD_COUNT; i++) {
        birds[i].active = true;
        birds[i].x      = -(rand() % WINDOW_WIDTH);         // stagger off-screen
        birds[i].y      = 80 + rand() % (WINDOW_HEIGHT - 160);
        birds[i].speed  = 1.5f + (rand() % 20) * 0.1f;
    }

    // Background clouds scroll slowly from right to left
    for (int i = 0; i < BGCLOUD_COUNT; i++) {
        bgClouds[i].x     = rand() % WINDOW_WIDTH;
        bgClouds[i].y     = 60 + rand() % (WINDOW_HEIGHT - 120);
        bgClouds[i].speed = 0.2f + (rand() % 5) * 0.1f;
        bgClouds[i].width = 60 + rand() % 80;
    }

    // Sparkles (static decorative dots)
    for (int i = 0; i < SPARKLE_COUNT; i++) {
        sparkles[i].x = rand() % WINDOW_WIDTH;
        sparkles[i].y = rand() % WINDOW_HEIGHT;
    }
}

// ────────────────────────────────────────────────────────────────────────────
void updateLevel4(float dt) {
    // --- Moving platforms ---
    cloudPlat1X += cloudPlat1Speed;
    if (cloudPlat1X > 320.0f) cloudPlat1Speed = -1.0f;
    if (cloudPlat1X < 80.0f)  cloudPlat1Speed =  1.0f;

    cloudPlat2X += cloudPlat2Speed;
    if (cloudPlat2X > 550.0f) cloudPlat2Speed = -1.3f;
    if (cloudPlat2X < 240.0f) cloudPlat2Speed =  1.3f;

    cloudPlat3X += cloudPlat3Speed;
    if (cloudPlat3X > 680.0f) cloudPlat3Speed = -1.6f;
    if (cloudPlat3X < 440.0f) cloudPlat3Speed =  1.6f;

    // --- Portal bob ---
    l4PortalTime += dt;
    l4PortalY = l4PortalBaseY + sinf(l4PortalTime * 2.0f) * 30.0f;

    // --- Background cloud scroll ---
    for (int i = 0; i < BGCLOUD_COUNT; i++) {
        bgClouds[i].x -= bgClouds[i].speed;
        if (bgClouds[i].x + bgClouds[i].width < 0) {
            bgClouds[i].x = WINDOW_WIDTH + rand() % 100;
        }
    }

    // --- Birds fly left-to-right; kill player on contact ---
    for (int i = 0; i < BIRD_COUNT; i++) {
        if (!birds[i].active) continue;

        birds[i].x += birds[i].speed;
        // Gentle up/down sine drift
        birds[i].y += sinf(glutGet(GLUT_ELAPSED_TIME) * 0.001f + i * 1.2f) * 0.4f;

        // Wrap around when off right edge
        if (birds[i].x > WINDOW_WIDTH + 30) {
            birds[i].x = -(20 + rand() % 120);
            birds[i].y = 80 + rand() % (WINDOW_HEIGHT - 160);
        }

        // Collision with player (bird body ~24×14 px)
        if (birds[i].x < playerX + PLAYER_WIDTH  &&
            birds[i].x + 24 > playerX             &&
            birds[i].y < playerY + PLAYER_HEIGHT  &&
            birds[i].y + 14 > playerY) {
            respawnPlayer();
        }
    }

    // --- Portal win check ---
    if (checkLevel4PortalCollision() && !level4Complete) {
        level4Complete = true;
    }
}

// ────────────────────────────────────────────────────────────────────────────
bool checkLevel4PortalCollision() {
    return (playerX + PLAYER_WIDTH  > l4PortalX      &&
            playerX                 < l4PortalX + 40 &&
            playerY + PLAYER_HEIGHT > l4PortalY      &&
            playerY                 < l4PortalY + 80);
}

// ────────────────────────────────────────────────────────────────────────────
bool isLevel4Complete() {
    return level4Complete;
}

// ────────────────────────────────────────────────────────────────────────────
bool checkLevel4PlatformCollision(float x, float y, float width, float height) {
    float platH = (float)TILE_SIZE;   // platforms are one tile tall

    // Cloud platform 1
    if (x + width > cloudPlat1X &&
        x         < cloudPlat1X + TILE_SIZE * 2 &&
        y         >= cloudPlat1Y + platH - 5 &&
        y         <= cloudPlat1Y + platH + 10) return true;

    // Cloud platform 2
    if (x + width > cloudPlat2X &&
        x         < cloudPlat2X + TILE_SIZE * 2 &&
        y         >= cloudPlat2Y + platH - 5 &&
        y         <= cloudPlat2Y + platH + 10) return true;

    // Cloud platform 3
    if (x + width > cloudPlat3X &&
        x         < cloudPlat3X + TILE_SIZE * 2 &&
        y         >= cloudPlat3Y + platH - 5 &&
        y         <= cloudPlat3Y + platH + 10) return true;

    return false;
}

// ─── Small helper: draw a fluffy cloud shape at (cx, cy) with given width ───
static void drawCloudShape(float cx, float cy, float w) {
    float h = w * 0.38f;
    // Bottom rectangle
    glBegin(GL_QUADS);
    glVertex2f(cx,     cy);
    glVertex2f(cx + w, cy);
    glVertex2f(cx + w, cy + h * 0.6f);
    glVertex2f(cx,     cy + h * 0.6f);
    glEnd();
    // Three bumps on top
    float bumpR[3] = { h * 0.45f, h * 0.55f, h * 0.45f };
    float bumpCX[3] = { cx + w * 0.22f, cx + w * 0.50f, cx + w * 0.78f };
    float bumpCY    = cy + h * 0.55f;
    for (int b = 0; b < 3; b++) {
        glBegin(GL_POLYGON);
        for (int j = 0; j < 20; j++) {
            float a = j * 3.14159f / 20.0f;   // half circle
            glVertex2f(bumpCX[b] + cosf(a) * bumpR[b],
                       bumpCY    + sinf(a) * bumpR[b]);
        }
        glEnd();
    }
}

// ─── Helper: draw a single moving cloud platform ────────────────────────────
static void drawMovingCloudPlatform(float px, float py) {
    // Dark outline / shadow
    glColor3f(0.70f, 0.78f, 0.90f);
    drawCloudShape(px - 2, py - 2, TILE_SIZE * 2 + 4);
    // White fluffy body
    glColor3f(0.97f, 0.97f, 1.00f);
    drawCloudShape(px, py, TILE_SIZE * 2);
    // Soft blue tint on underside
    glColor3f(0.82f, 0.88f, 0.98f);
    glBegin(GL_QUADS);
    glVertex2f(px + 4, py);
    glVertex2f(px + TILE_SIZE * 2 - 4, py);
    glVertex2f(px + TILE_SIZE * 2 - 4, py + 6);
    glVertex2f(px + 4, py + 6);
    glEnd();
}

// ────────────────────────────────────────────────────────────────────────────
void drawSkyWorld() {
    float timeSec = glutGet(GLUT_ELAPSED_TIME) * 0.001f;

    // 1. Decorative background clouds (slowly scrolling)
    glColor3f(0.90f, 0.93f, 1.00f);
    for (int i = 0; i < BGCLOUD_COUNT; i++) {
        drawCloudShape(bgClouds[i].x, bgClouds[i].y, bgClouds[i].width);
    }

    // 2. Sun (top-right, static glow + body)
    float sunX = 660.0f, sunY = 460.0f, sunR = 55.0f;

    // Outer glow
    glColor3f(1.0f, 0.95f, 0.50f);
    glBegin(GL_POLYGON);
    for (int i = 0; i < 360; i++) {
        float a = i * 3.14159f / 180.0f;
        glVertex2f(sunX + cosf(a) * (sunR + 18), sunY + sinf(a) * (sunR + 18));
    }
    glEnd();

    // Sun body
    glColor3f(1.0f, 0.90f, 0.10f);
    glBegin(GL_POLYGON);
    for (int i = 0; i < 360; i++) {
        float a = i * 3.14159f / 180.0f;
        glVertex2f(sunX + cosf(a) * sunR, sunY + sinf(a) * sunR);
    }
    glEnd();

    // Rotating sun rays (8 rays)
    glColor3f(1.0f, 0.85f, 0.20f);
    float rayAngle = timeSec * 0.4f;
    for (int r = 0; r < 8; r++) {
        float a = rayAngle + r * (3.14159f / 4.0f);
        float x1 = sunX + cosf(a) * (sunR + 5);
        float y1 = sunY + sinf(a) * (sunR + 5);
        float x2 = sunX + cosf(a) * (sunR + 24);
        float y2 = sunY + sinf(a) * (sunR + 24);
        glLineWidth(3.0f);
        glBegin(GL_LINES);
        glVertex2f(x1, y1);
        glVertex2f(x2, y2);
        glEnd();
    }
    glLineWidth(1.0f);

    // 3. Sparkles (twinkling small dots high in the sky)
    for (int i = 0; i < SPARKLE_COUNT; i++) {
        float brightness = 0.6f + 0.4f * sinf(timeSec * 2.0f + i * 0.7f);
        glColor3f(brightness, brightness, 1.0f);
        glPointSize((i % 5 == 0) ? 3.0f : 2.0f);
        glBegin(GL_POINTS);
        glVertex2f(sparkles[i].x, sparkles[i].y);
        glEnd();
    }
    glPointSize(1.0f);

    // 4. Moving cloud platforms
    drawMovingCloudPlatform(cloudPlat1X, cloudPlat1Y);
    drawMovingCloudPlatform(cloudPlat2X, cloudPlat2Y);
    drawMovingCloudPlatform(cloudPlat3X, cloudPlat3Y);

    // 5. Birds (simple V-shape silhouettes)
    for (int i = 0; i < BIRD_COUNT; i++) {
        if (!birds[i].active) continue;

        float bx = birds[i].x;
        float by = birds[i].y;
        float wingFlap = sinf(timeSec * 8.0f + i * 1.5f) * 4.0f;

        glColor3f(0.15f, 0.15f, 0.20f);
        glBegin(GL_LINES);
        // Left wing
        glVertex2f(bx + 12, by + 7);
        glVertex2f(bx,      by + wingFlap);
        // Right wing
        glVertex2f(bx + 12, by + 7);
        glVertex2f(bx + 24, by + wingFlap);
        glEnd();

        // Body dot
        glPointSize(4.0f);
        glBegin(GL_POINTS);
        glVertex2f(bx + 12, by + 7);
        glEnd();
        glPointSize(1.0f);
    }

    // 6. Portal (golden sky gateway)
    // Outer aura
    glColor4f(1.0f, 0.85f, 0.0f, 0.25f);
    glBegin(GL_POLYGON);
    for (int i = 0; i < 40; i++) {
        float a = i * 6.28318f / 40.0f;
        glVertex2f(l4PortalX + 20 + cosf(a) * 48, l4PortalY + 40 + sinf(a) * 68);
    }
    glEnd();

    // Dark frame
    glColor3f(0.20f, 0.15f, 0.05f);
    glBegin(GL_QUADS);
    glVertex2f(l4PortalX - 5, l4PortalY);
    glVertex2f(l4PortalX + 45, l4PortalY);
    glVertex2f(l4PortalX + 45, l4PortalY + 80);
    glVertex2f(l4PortalX - 5, l4PortalY + 80);
    glEnd();

    // Bright inner ring
    glColor3f(1.0f, 0.80f, 0.0f);
    glBegin(GL_POLYGON);
    for (int i = 0; i < 40; i++) {
        float a = i * 6.28318f / 40.0f;
        glVertex2f(l4PortalX + 20 + cosf(a) * 18, l4PortalY + 40 + sinf(a) * 35);
    }
    glEnd();

    // White-gold core
    glColor3f(1.0f, 0.97f, 0.70f);
    glBegin(GL_POLYGON);
    for (int i = 0; i < 40; i++) {
        float a = i * 6.28318f / 40.0f;
        glVertex2f(l4PortalX + 20 + cosf(a) * 10, l4PortalY + 40 + sinf(a) * 20);
    }
    glEnd();
}
