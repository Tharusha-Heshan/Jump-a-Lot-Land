#include "Player.h"
#include "Level5Logic.h"
#include "Constants.h"
#include <GL/freeglut.h>
#include <cmath>

// Moving vine/log platforms
float vinePlatform1X = 100;
float vinePlatform1Y = 200;
float vinePlatform1Speed = 1.2f;

float vinePlatform2X = 250;
float vinePlatform2Y = 320;
float vinePlatform2Speed = -1.4f;

float vinePlatform3X = 450;
float vinePlatform3Y = 440;
float vinePlatform3Speed = 1.6f;

// Tree portal (goal)
float treePortalX = 680.0f;
float treePortalY = 460.0f;
float treePortalBaseY = 460.0f;
float treePortalTime = 0.0f;

bool jungleLevelComplete = false;

// Decorative fireflies
struct Firefly {
    float x;
    float y;
};

const int FIREFLY_COUNT = 60;
Firefly fireflies[FIREFLY_COUNT];

// Falling leaves / coconuts (hazard)
struct Leaf {
    float x;
    float y;
    float speed;
    bool active;
    int kind; // 0 = leaf, 1 = coconut
};

const int LEAF_COUNT = 8;
Leaf leaves[LEAF_COUNT];

// Patrolling snakes (hazard)
struct Snake {
    float x;
    float y;
    float minX;
    float maxX;
    float speed;
    bool facingRight;
};

const int SNAKE_COUNT = 2;
Snake snakes[SNAKE_COUNT];

// Swinging thorn pendulum (hazard)
struct Pendulum {
    float anchorX;
    float anchorY;
    float length;
    float angularSpeed;
    float maxAngle;
};

const int PENDULUM_COUNT = 1;
Pendulum pendulums[PENDULUM_COUNT];

// Decorative butterflies
struct Butterfly {
    float baseX;
    float baseY;
    float phase;
    int colorIndex;
};

const int BUTTERFLY_COUNT = 8;
Butterfly butterflies[BUTTERFLY_COUNT];

void resetLevel5State() {
    jungleLevelComplete = false;

    for (int i = 0; i < LEAF_COUNT; i++) {
        leaves[i].active = true;
        leaves[i].x = rand() % WINDOW_WIDTH;
        leaves[i].y = WINDOW_HEIGHT + rand() % 1200;
        leaves[i].speed = 1.0f + (rand() % 200) / 100.0f;
        leaves[i].kind = (i % 3 == 0) ? 1 : 0;
    }

    for (int i = 0; i < FIREFLY_COUNT; i++) {
        fireflies[i].x = rand() % WINDOW_WIDTH;
        fireflies[i].y = rand() % WINDOW_HEIGHT;
    }

    snakes[0].x = 150.0f;
    snakes[0].y = 240.0f;
    snakes[0].minX = 130.0f;
    snakes[0].maxX = 330.0f;
    snakes[0].speed = 1.0f;
    snakes[0].facingRight = true;

    snakes[1].x = 460.0f;
    snakes[1].y = 380.0f;
    snakes[1].minX = 360.0f;
    snakes[1].maxX = 560.0f;
    snakes[1].speed = 1.3f;
    snakes[1].facingRight = false;

    pendulums[0].anchorX = 380.0f;
    pendulums[0].anchorY = 600.0f;
    pendulums[0].length = 160.0f;
    pendulums[0].angularSpeed = 1.2f;
    pendulums[0].maxAngle = 1.0f;

    for (int i = 0; i < BUTTERFLY_COUNT; i++) {
        butterflies[i].baseX = 60.0f + (rand() % 700);
        butterflies[i].baseY = 200.0f + (rand() % 320);
        butterflies[i].phase = (float)(rand() % 628) / 100.0f;
        butterflies[i].colorIndex = i % 3;
    }
}

void updateLevel5(float dt) {
    vinePlatform1X += vinePlatform1Speed;
    if (vinePlatform1X > 300) vinePlatform1Speed = -1.2f;
    if (vinePlatform1X < 100) vinePlatform1Speed = 1.2f;

    vinePlatform2X += vinePlatform2Speed;
    if (vinePlatform2X > 450) vinePlatform2Speed = -1.4f;
    if (vinePlatform2X < 250) vinePlatform2Speed = 1.4f;

    vinePlatform3X += vinePlatform3Speed;
    if (vinePlatform3X > 650) vinePlatform3Speed = -1.6f;
    if (vinePlatform3X < 450) vinePlatform3Speed = 1.6f;

    treePortalTime += dt;
    treePortalY = treePortalBaseY + sinf(treePortalTime * 2.0f) * 30.0f;

    // Falling leaves and coconuts
    for (int i = 0; i < LEAF_COUNT; i++) {
        leaves[i].y -= leaves[i].speed;
        leaves[i].x += sinf(glutGet(GLUT_ELAPSED_TIME) * 0.001f + i) * 0.4f;

        if (leaves[i].y < -20) {
            leaves[i].y = WINDOW_HEIGHT + rand() % 300;
            leaves[i].x = rand() % WINDOW_WIDTH;
        }

        if (leaves[i].x < playerX + PLAYER_WIDTH &&
            leaves[i].x + 18 > playerX &&
            leaves[i].y < playerY + PLAYER_HEIGHT &&
            leaves[i].y + 18 > playerY) {
            respawnPlayer();
        }
    }

    // Patrolling snakes
    for (int i = 0; i < SNAKE_COUNT; i++) {
        if (snakes[i].facingRight) {
            snakes[i].x += snakes[i].speed;
            if (snakes[i].x >= snakes[i].maxX) {
                snakes[i].x = snakes[i].maxX;
                snakes[i].facingRight = false;
            }
        } else {
            snakes[i].x -= snakes[i].speed;
            if (snakes[i].x <= snakes[i].minX) {
                snakes[i].x = snakes[i].minX;
                snakes[i].facingRight = true;
            }
        }

        if (playerX < snakes[i].x + 40 && playerX + PLAYER_WIDTH > snakes[i].x &&
            playerY < snakes[i].y + 14 && playerY + PLAYER_HEIGHT > snakes[i].y - 6) {
            respawnPlayer();
        }
    }

    // Swinging thorn pendulum
    for (int i = 0; i < PENDULUM_COUNT; i++) {
        float angle = sinf(treePortalTime * pendulums[i].angularSpeed) * pendulums[i].maxAngle;
        float ballX = pendulums[i].anchorX + sinf(angle) * pendulums[i].length;
        float ballY = pendulums[i].anchorY - cosf(angle) * pendulums[i].length;

        if (playerX + PLAYER_WIDTH > ballX - 14 && playerX < ballX + 14 &&
            playerY + PLAYER_HEIGHT > ballY - 14 && playerY < ballY + 14) {
            respawnPlayer();
        }
    }

    if (checkPortal5Collision() && !jungleLevelComplete) {
        jungleLevelComplete = true;
    }
}

bool checkPortal5Collision() {
    if (playerX + PLAYER_WIDTH > treePortalX &&
        playerX < treePortalX + 40 &&
        playerY + PLAYER_HEIGHT > treePortalY &&
        playerY < treePortalY + 80) {
        return true;
    }
    return false;
}

bool isLevel5Complete() {
    return jungleLevelComplete;
}

bool checkPlatform5Collision(float x, float y, float width, float height) {
    if (x + width > vinePlatform1X && x < vinePlatform1X + TILE_SIZE * 2 &&
        y >= vinePlatform1Y + TILE_SIZE - 5 && y <= vinePlatform1Y + TILE_SIZE + 10) return true;

    if (x + width > vinePlatform2X && x < vinePlatform2X + TILE_SIZE * 2 &&
        y >= vinePlatform2Y + TILE_SIZE - 5 && y <= vinePlatform2Y + TILE_SIZE + 10) return true;

    if (x + width > vinePlatform3X && x < vinePlatform3X + TILE_SIZE * 2 &&
        y >= vinePlatform3Y + TILE_SIZE - 5 && y <= vinePlatform3Y + TILE_SIZE + 10) return true;

    return false;
}

static void drawEllipse(float cx, float cy, float rx, float ry, int segments) {
    glBegin(GL_POLYGON);
    for (int i = 0; i < segments; i++) {
        float a = i * 6.28318f / segments;
        glVertex2f(cx + cosf(a) * rx, cy + sinf(a) * ry);
    }
    glEnd();
}

static void drawLogPlatform(float x, float y) {
    // Log body
    glColor3f(0.42f, 0.27f, 0.13f);
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + 64, y);
    glVertex2f(x + 64, y + 32);
    glVertex2f(x, y + 32);
    glEnd();

    // Bark rings highlight
    glColor3f(0.55f, 0.38f, 0.20f);
    glBegin(GL_QUADS);
    glVertex2f(x + 3, y + 3);
    glVertex2f(x + 61, y + 3);
    glVertex2f(x + 61, y + 8);
    glVertex2f(x + 3, y + 8);
    glEnd();

    // Leafy top
    glColor3f(0.15f, 0.55f, 0.15f);
    glBegin(GL_QUADS);
    glVertex2f(x, y + 28);
    glVertex2f(x + 64, y + 28);
    glVertex2f(x + 64, y + 34);
    glVertex2f(x, y + 34);
    glEnd();
}

static void drawLeaf(float x, float y) {
    glColor3f(0.20f, 0.65f, 0.20f);
    glBegin(GL_TRIANGLES);
    glVertex2f(x, y + 9);
    glVertex2f(x + 9, y + 18);
    glVertex2f(x + 18, y + 9);

    glVertex2f(x, y + 9);
    glVertex2f(x + 18, y + 9);
    glVertex2f(x + 9, y);
    glEnd();

    glColor3f(0.10f, 0.40f, 0.10f);
    glBegin(GL_LINES);
    glVertex2f(x + 1, y + 9);
    glVertex2f(x + 17, y + 9);
    glEnd();
}

static void drawCoconut(float x, float y) {
    glColor3f(0.32f, 0.20f, 0.10f);
    drawEllipse(x + 9, y + 9, 9.0f, 9.0f, 14);

    glColor3f(0.55f, 0.42f, 0.22f);
    drawEllipse(x + 9, y + 9, 4.0f, 4.0f, 10);

    glColor3f(0.18f, 0.10f, 0.05f);
    glBegin(GL_LINES);
    glVertex2f(x + 6, y + 6);
    glVertex2f(x + 12, y + 12);
    glVertex2f(x + 12, y + 6);
    glVertex2f(x + 6, y + 12);
    glEnd();
}

static void drawBackgroundTree(float cx, float topY, float rx, float ry) {
    // Trunk
    glColor3f(0.14f, 0.10f, 0.06f);
    glBegin(GL_QUADS);
    glVertex2f(cx - 7, 0);
    glVertex2f(cx + 7, 0);
    glVertex2f(cx + 5, topY - ry * 0.5f);
    glVertex2f(cx - 5, topY - ry * 0.5f);
    glEnd();

    // Canopy (desaturated, layered for depth)
    glColor4f(0.05f, 0.22f, 0.10f, 0.85f);
    drawEllipse(cx, topY, rx, ry, 28);

    glColor4f(0.09f, 0.32f, 0.15f, 0.85f);
    drawEllipse(cx, topY + ry * 0.18f, rx * 0.62f, ry * 0.62f, 24);
}

static void drawHangingVine(float x, float topY, float length, float phase, float timeSec) {
    const int segments = 6;
    float prevX = x;
    float prevY = topY;

    for (int i = 1; i <= segments; i++) {
        float t = (float)i / segments;
        float segY = topY - length * t;
        float sway = sinf(timeSec * 1.5f + phase + t * 2.0f) * (5.0f * t);
        float segX = x + sway;

        glColor3f(0.14f, 0.42f, 0.12f);
        glBegin(GL_QUADS);
        glVertex2f(prevX - 2, prevY);
        glVertex2f(prevX + 2, prevY);
        glVertex2f(segX + 2, segY);
        glVertex2f(segX - 2, segY);
        glEnd();

        prevX = segX;
        prevY = segY;
    }

    // Leaf cluster at the tip
    glColor3f(0.20f, 0.60f, 0.18f);
    for (int k = 0; k < 3; k++) {
        float lx = prevX + (k - 1) * 7.0f;
        float ly = prevY - (float)k * 2.0f;
        glBegin(GL_TRIANGLES);
        glVertex2f(lx - 6, ly);
        glVertex2f(lx + 6, ly);
        glVertex2f(lx, ly - 12);
        glEnd();
    }
}

static void drawSnake(float x, float y, bool facingRight, float timeSec) {
    // Body segments (slithering wave)
    glColor3f(0.18f, 0.55f, 0.16f);
    for (int i = 0; i < 4; i++) {
        float segX = x + i * 8.0f;
        float segY = y + sinf(timeSec * 8.0f + i * 1.2f) * 3.0f;
        drawEllipse(segX, segY, 7.0f, 5.0f, 14);
    }

    // Belly stripes
    glColor3f(0.85f, 0.80f, 0.30f);
    for (int i = 0; i < 4; i++) {
        float segX = x + i * 8.0f;
        float segY = y + sinf(timeSec * 8.0f + i * 1.2f) * 3.0f;
        drawEllipse(segX, segY - 1.5f, 3.0f, 1.5f, 8);
    }

    // Head
    float headX = facingRight ? x + 34.0f : x - 6.0f;
    float headY = y + sinf(timeSec * 8.0f + 4 * 1.2f) * 3.0f;
    glColor3f(0.10f, 0.65f, 0.20f);
    drawEllipse(headX, headY, 8.0f, 6.0f, 14);

    // Eye
    glColor3f(0.95f, 0.15f, 0.15f);
    glPointSize(3);
    glBegin(GL_POINTS);
    glVertex2f(headX + (facingRight ? 4.0f : -4.0f), headY + 2.0f);
    glEnd();

    // Forked tongue
    glColor3f(0.85f, 0.10f, 0.10f);
    glBegin(GL_LINES);
    glVertex2f(headX + (facingRight ? 8.0f : -8.0f), headY);
    glVertex2f(headX + (facingRight ? 13.0f : -13.0f), headY + 2.0f);
    glVertex2f(headX + (facingRight ? 8.0f : -8.0f), headY);
    glVertex2f(headX + (facingRight ? 13.0f : -13.0f), headY - 2.0f);
    glEnd();
}

static void drawThornBall(float x, float y, float timeSec) {
    float spin = timeSec * 2.0f;

    // Wooden core
    glColor3f(0.40f, 0.26f, 0.13f);
    drawEllipse(x, y, 13.0f, 13.0f, 18);

    // Spikes radiating outward
    glColor3f(0.75f, 0.62f, 0.35f);
    for (int i = 0; i < 8; i++) {
        float a = i * 6.28318f / 8.0f + spin;
        float cx = x + cosf(a) * 13.0f;
        float cy = y + sinf(a) * 13.0f;
        float ex = x + cosf(a) * 23.0f;
        float ey = y + sinf(a) * 23.0f;
        float px = -sinf(a) * 3.0f;
        float py = cosf(a) * 3.0f;

        glBegin(GL_TRIANGLES);
        glVertex2f(cx + px, cy + py);
        glVertex2f(cx - px, cy - py);
        glVertex2f(ex, ey);
        glEnd();
    }
}

static void drawButterfly(float x, float y, float flap, float r, float g, float b) {
    glColor3f(r, g, b);
    glBegin(GL_TRIANGLES);
    // Left wing
    glVertex2f(x, y);
    glVertex2f(x - 10.0f * flap, y + 8.0f);
    glVertex2f(x - 6.0f * flap, y - 6.0f);
    // Right wing
    glVertex2f(x, y);
    glVertex2f(x + 10.0f * flap, y + 8.0f);
    glVertex2f(x + 6.0f * flap, y - 6.0f);
    glEnd();

    // Body
    glColor3f(0.10f, 0.10f, 0.10f);
    glBegin(GL_LINES);
    glVertex2f(x, y - 6.0f);
    glVertex2f(x, y + 4.0f);
    glEnd();
}

void drawJungleBackground() {
    float timeSec = glutGet(GLUT_ELAPSED_TIME) * 0.001f;

    // Glowing sun
    glColor4f(1.0f, 0.95f, 0.55f, 0.18f);
    drawEllipse(110.0f, 540.0f, 90.0f, 90.0f, 40);

    glColor3f(1.0f, 0.92f, 0.55f);
    drawEllipse(110.0f, 540.0f, 38.0f, 38.0f, 30);

    // Sun rays
    glColor4f(1.0f, 0.95f, 0.6f, 0.35f);
    for (int i = 0; i < 8; i++) {
        float a = i * 6.28318f / 8.0f + timeSec * 0.1f;
        float ax = cosf(a);
        float ay = sinf(a);
        glBegin(GL_TRIANGLES);
        glVertex2f(110 + ax * 46, 540 + ay * 46);
        glVertex2f(110 + ax * 96 - ay * 6, 540 + ay * 96 + ax * 6);
        glVertex2f(110 + ax * 96 + ay * 6, 540 + ay * 96 - ax * 6);
        glEnd();
    }

    // Distant mountain silhouettes (layered for depth)
    glColor4f(0.05f, 0.22f, 0.12f, 0.55f);
    glBegin(GL_POLYGON);
    glVertex2f(0, 280);
    glVertex2f(0, 380);
    glVertex2f(120, 460);
    glVertex2f(260, 380);
    glVertex2f(400, 470);
    glVertex2f(540, 390);
    glVertex2f(680, 450);
    glVertex2f(800, 380);
    glVertex2f(800, 280);
    glEnd();

    glColor4f(0.04f, 0.18f, 0.10f, 0.5f);
    glBegin(GL_POLYGON);
    glVertex2f(0, 240);
    glVertex2f(0, 320);
    glVertex2f(180, 390);
    glVertex2f(360, 310);
    glVertex2f(560, 400);
    glVertex2f(800, 320);
    glVertex2f(800, 240);
    glEnd();

    // Smaller background trees for jungle depth
    drawBackgroundTree(150.0f, 380.0f, 70.0f, 55.0f);
    drawBackgroundTree(430.0f, 410.0f, 60.0f, 48.0f);
    drawBackgroundTree(740.0f, 360.0f, 55.0f, 45.0f);

    // Soft mist band over the ground
    glColor4f(0.85f, 0.95f, 0.85f, 0.10f);
    glBegin(GL_QUADS);
    glVertex2f(0, 70);
    glVertex2f(WINDOW_WIDTH, 70);
    glVertex2f(WINDOW_WIDTH, 160);
    glVertex2f(0, 160);
    glEnd();
}

void drawJungleWorld() {
    float timeSec = glutGet(GLUT_ELAPSED_TIME) * 0.001f;

    // 1. Hanging vines with leaf clusters
    drawHangingVine(80.0f, 600.0f, 90.0f, 0.3f, timeSec);
    drawHangingVine(230.0f, 600.0f, 70.0f, 1.4f, timeSec);
    drawHangingVine(320.0f, 600.0f, 110.0f, 2.1f, timeSec);
    drawHangingVine(470.0f, 600.0f, 80.0f, 0.8f, timeSec);
    drawHangingVine(760.0f, 600.0f, 95.0f, 1.9f, timeSec);

    // 2. Fireflies
    for (int i = 0; i < FIREFLY_COUNT; i++) {
        float glow = (sinf(timeSec * 3.0f + i) + 1.0f) * 0.5f;
        glColor3f(0.6f + glow * 0.4f, 1.0f, 0.3f + glow * 0.3f);
        if (i % 8 == 0) glPointSize(4);
        else glPointSize(2);
        glBegin(GL_POINTS);
        glVertex2f(fireflies[i].x, fireflies[i].y);
        glEnd();
    }

    // 3. Giant tree trunk
    glColor3f(0.30f, 0.18f, 0.08f);
    glBegin(GL_QUADS);
    glVertex2f(540, 0);
    glVertex2f(600, 0);
    glVertex2f(590, 320);
    glVertex2f(550, 320);
    glEnd();

    // 4. Tree canopy (layered glow + body)
    glColor4f(0.10f, 0.45f, 0.10f, 0.25f);
    drawEllipse(560.0f, 350.0f, 150.0f, 120.0f, 40);

    glColor3f(0.13f, 0.50f, 0.13f);
    drawEllipse(560.0f, 350.0f, 110.0f, 90.0f, 40);

    glColor3f(0.20f, 0.62f, 0.20f);
    drawEllipse(540.0f, 380.0f, 55.0f, 45.0f, 40);

    // 5. Moving vine/log platforms
    drawLogPlatform(vinePlatform1X, vinePlatform1Y);
    drawLogPlatform(vinePlatform2X, vinePlatform2Y);
    drawLogPlatform(vinePlatform3X, vinePlatform3Y);

    // 6. Falling leaves and coconuts
    for (int i = 0; i < LEAF_COUNT; i++) {
        if (leaves[i].kind == 1) drawCoconut(leaves[i].x, leaves[i].y);
        else drawLeaf(leaves[i].x, leaves[i].y);
    }

    // 7. Patrolling snakes
    for (int i = 0; i < SNAKE_COUNT; i++) {
        drawSnake(snakes[i].x, snakes[i].y, snakes[i].facingRight, timeSec);
    }

    // 8. Swinging thorn pendulum
    for (int i = 0; i < PENDULUM_COUNT; i++) {
        float angle = sinf(treePortalTime * pendulums[i].angularSpeed) * pendulums[i].maxAngle;
        float ballX = pendulums[i].anchorX + sinf(angle) * pendulums[i].length;
        float ballY = pendulums[i].anchorY - cosf(angle) * pendulums[i].length;

        glColor3f(0.30f, 0.20f, 0.10f);
        glBegin(GL_LINES);
        glVertex2f(pendulums[i].anchorX, pendulums[i].anchorY);
        glVertex2f(ballX, ballY);
        glEnd();

        drawThornBall(ballX, ballY, timeSec);
    }

    // 9. Decorative butterflies
    for (int i = 0; i < BUTTERFLY_COUNT; i++) {
        float bx = butterflies[i].baseX + sinf(timeSec * 1.3f + butterflies[i].phase) * 50.0f;
        float by = butterflies[i].baseY + cosf(timeSec * 1.7f + butterflies[i].phase) * 25.0f;
        float flap = 0.4f + fabsf(sinf(timeSec * 9.0f + butterflies[i].phase)) * 0.8f;

        float r, g, b;
        switch (butterflies[i].colorIndex) {
            case 0: r = 1.0f; g = 0.75f; b = 0.15f; break;
            case 1: r = 0.55f; g = 0.75f; b = 1.0f; break;
            default: r = 1.0f; g = 0.95f; b = 0.95f; break;
        }

        drawButterfly(bx, by, flap, r, g, b);
    }

    // 10. Tree portal (vine gate)
    glColor4f(0.4f, 1.0f, 0.3f, 0.25f);
    drawEllipse(treePortalX + 20.0f, treePortalY + 40.0f, 40.0f, 60.0f, 40);

    glColor3f(0.25f, 0.18f, 0.08f);
    glBegin(GL_QUADS);
    glVertex2f(treePortalX - 5, treePortalY);
    glVertex2f(treePortalX + 45, treePortalY);
    glVertex2f(treePortalX + 45, treePortalY + 80);
    glVertex2f(treePortalX - 5, treePortalY + 80);
    glEnd();

    glColor3f(0.20f, 0.85f, 0.25f);
    drawEllipse(treePortalX + 20.0f, treePortalY + 40.0f, 18.0f, 35.0f, 40);

    glColor3f(0.75f, 1.0f, 0.5f);
    drawEllipse(treePortalX + 20.0f, treePortalY + 40.0f, 10.0f, 20.0f, 40);
}
