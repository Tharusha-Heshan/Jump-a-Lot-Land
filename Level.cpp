#include "Level.h"
#include "Level1.h"
#include "Level2.h"
#include "level3.h"
#include "level3logic.h"
#include "Player.h"
#include <GL/freeglut.h>
#include <cmath>
#include <cstdlib>

// ---- Level 2 particle data ----
struct Firefly { float x, y, phase; };
struct Leaf    { float x, y, speed, wobble; };
const int FIREFLY_COUNT = 18;
const int LEAF_COUNT    = 20;
static Firefly fireflies[FIREFLY_COUNT];
static Leaf    leaves[LEAF_COUNT];
static bool    level2ParticlesInit = false;

void initLevel2Particles() {
    for (int i = 0; i < FIREFLY_COUNT; i++) {
        fireflies[i].x     = (float)(rand() % 800);
        fireflies[i].y     = (float)(rand() % 600);
        fireflies[i].phase = (float)(rand() % 628) * 0.01f;
    }
    for (int i = 0; i < LEAF_COUNT; i++) {
        leaves[i].x      = (float)(rand() % 800);
        leaves[i].y      = (float)(rand() % 600);
        leaves[i].speed  = 0.4f + (rand() % 10) * 0.08f;
        leaves[i].wobble = (float)(rand() % 628) * 0.01f;
    }
    level2ParticlesInit = true;
}

int levelMap[ROWS][COLS];
int currentActiveLevel = 1;

void loadLevel(int levelID) {
    currentActiveLevel = levelID;


    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            if (levelID == 1) {
                levelMap[r][c] = level1Data[r][c];
            }
            else if (levelID == 2) {
                levelMap[r][c] = level2Data[r][c];
            }
            else if (levelID == 3) {
                levelMap[r][c] = level3Data[r][c];
            }
        }
    }

    switch(levelID)
    {
    case 1:
        setSpawnPoint(50.0f, 250.0f);
        break;

    case 2:
        setSpawnPoint(48.0f, 500.0f);
        break;

    case 3:
        setSpawnPoint(10.0f, 180.0f);
        resetLevel3State();
        break;
    }
}

bool checkCollision(float x, float y, float width, float height) {
    float corners[4][2] = {
        { x, y }, { x + width, y }, { x, y + height }, { x + width, y + height }
    };

    for (int i = 0; i < 4; i++) {
        int col = (int)(corners[i][0] / TILE_SIZE);
        int row = (int)((WINDOW_HEIGHT - corners[i][1]) / TILE_SIZE);

        if (row >= 0 && row < ROWS && col >= 0 && col < COLS) {
            int tileType = levelMap[row][col];
            if (currentActiveLevel == 2) {
                if (tileType == 1 || tileType == 2 || tileType == 5 || tileType == 6 || tileType == 9) return true;
                if (tileType == 7 && playerScale > 0.6f) return true; // Gap wall is passable when shrunk!
            } else {
                // Treat Lava Rock (1, 2) and Moon Ground (11, 12) as solid ground
                if (tileType == 1 || tileType == 2 || tileType == 6 || tileType == 7 || tileType == 11 || tileType == 12) {
                    return true;
                }
            }
        }
    }

    if(currentActiveLevel == 3) {
        if(checkPlatformCollision(x, y, width, height)) {
            return true;
        }
    }

    return false;
}

bool checkLavaCollision(float x, float y, float width, float height) {
    float centerX = x + width / 2.0f;
    float centerY = y + height / 4.0f;

    int col = (int)(centerX / TILE_SIZE);
    int row = (int)((WINDOW_HEIGHT - centerY) / TILE_SIZE);

    if (row >= 0 && row < ROWS && col >= 0 && col < COLS) {
        int tileType = levelMap[row][col];
        if (currentActiveLevel == 2) {
            return (tileType == 3 || tileType == 4);
        } else {
            // Treat Lava (3, 4) and Freezing Water (8, 9) as deadly hazards
            return (tileType == 3 || tileType == 4 || tileType == 8 || tileType == 9);
        }
    }
    return false;
}

void drawRoundedRect(float x, float y, float w, float h, float r, float rColor, float gColor, float bColor) {
    glColor3f(rColor, gColor, bColor);
    glBegin(GL_POLYGON);
    glVertex2f(x + r, y);
    glVertex2f(x + w - r, y);
    glVertex2f(x + w, y + r);
    glVertex2f(x + w, y + h - r);
    glVertex2f(x + w - r, y + h);
    glVertex2f(x + r, y + h);
    glVertex2f(x, y + h - r);
    glVertex2f(x, y + r);
    glEnd();
}

void drawTile(float x, float y, int type) {
    if (type == 0) return;

    if (currentActiveLevel == 2) {
        switch (type) {
        case 1: {
            glColor3f(0.18f, 0.52f, 0.32f);
            glBegin(GL_QUADS);
            glVertex2f(x,y); glVertex2f(x+TILE_SIZE,y);
            glVertex2f(x+TILE_SIZE,y+TILE_SIZE); glVertex2f(x,y+TILE_SIZE);
            glEnd();
            glColor3f(0.35f, 0.82f, 0.60f);
            glBegin(GL_QUADS);
            glVertex2f(x+3,y+3); glVertex2f(x+TILE_SIZE-3,y+3);
            glVertex2f(x+TILE_SIZE-3,y+TILE_SIZE-3); glVertex2f(x+3,y+TILE_SIZE-3);
            glEnd();
            // Animated shimmer highlight sweeping across solid blocks
            float t2 = glutGet(GLUT_ELAPSED_TIME) * 0.001f;
            float sweep = fmodf(t2 * 60.0f + x + y, 900.0f) - 50.0f;
            if (sweep > 0 && sweep < TILE_SIZE + 6) {
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                float alpha = 0.18f * (1.0f - fabsf(sweep - TILE_SIZE*0.5f) / (TILE_SIZE*0.5f + 3.0f));
                glColor4f(0.8f, 1.0f, 0.85f, alpha);
                glBegin(GL_QUADS);
                glVertex2f(x+sweep-3, y); glVertex2f(x+sweep+3, y);
                glVertex2f(x+sweep+3, y+TILE_SIZE); glVertex2f(x+sweep-3, y+TILE_SIZE);
                glEnd();
                glDisable(GL_BLEND);
            }
            break;
        }
        case 2:
            glColor3f(0.18f, 0.52f, 0.32f);
            glBegin(GL_QUADS);
            glVertex2f(x,y); glVertex2f(x+TILE_SIZE,y);
            glVertex2f(x+TILE_SIZE,y+TILE_SIZE); glVertex2f(x,y+TILE_SIZE);
            glEnd();
            glColor3f(0.35f, 0.82f, 0.60f);
            glBegin(GL_QUADS);
            glVertex2f(x+3,y+3); glVertex2f(x+TILE_SIZE-3,y+3);
            glVertex2f(x+TILE_SIZE-3,y+TILE_SIZE-3); glVertex2f(x+3,y+TILE_SIZE-3);
            glEnd();
            glColor3f(0.88f, 1.0f, 0.96f);
            glBegin(GL_QUADS);
            glVertex2f(x,y+TILE_SIZE-7); glVertex2f(x+TILE_SIZE,y+TILE_SIZE-7);
            glVertex2f(x+TILE_SIZE,y+TILE_SIZE); glVertex2f(x,y+TILE_SIZE);
            glEnd();
            break;
        case 3:
            glColor3f(0.50f, 1.0f, 0.82f);
            glBegin(GL_TRIANGLES);
            glVertex2f(x+3,  y+2); glVertex2f(x+18, y+2); glVertex2f(x+10, y+TILE_SIZE-6);
            glVertex2f(x+22, y+2); glVertex2f(x+37, y+2); glVertex2f(x+30, y+TILE_SIZE-6);
            glEnd();
            glColor3f(1.0f, 1.0f, 1.0f);
            glBegin(GL_TRIANGLES);
            glVertex2f(x+9,  y+TILE_SIZE-6); glVertex2f(x+11, y+TILE_SIZE-6); glVertex2f(x+10, y+TILE_SIZE-2);
            glVertex2f(x+29, y+TILE_SIZE-6); glVertex2f(x+31, y+TILE_SIZE-6); glVertex2f(x+30, y+TILE_SIZE-2);
            glEnd();
            break;
        case 4:
            glColor3f(0.01f, 0.10f, 0.03f);
            glBegin(GL_QUADS);
            glVertex2f(x,y); glVertex2f(x+TILE_SIZE,y);
            glVertex2f(x+TILE_SIZE,y+TILE_SIZE); glVertex2f(x,y+TILE_SIZE);
            glEnd();
            break;
        case 5:
            glColor3f(0.14f, 0.46f, 0.26f);
            glBegin(GL_QUADS);
            glVertex2f(x,y); glVertex2f(x+TILE_SIZE,y);
            glVertex2f(x+TILE_SIZE,y+TILE_SIZE); glVertex2f(x,y+TILE_SIZE);
            glEnd();
            glColor3f(0.48f, 0.92f, 0.72f);
            glBegin(GL_QUADS);
            glVertex2f(x+17,y+2); glVertex2f(x+21,y+2);
            glVertex2f(x+21,y+TILE_SIZE-2); glVertex2f(x+17,y+TILE_SIZE-2);
            glEnd();
            break;
        case 6:
            glColor3f(0.38f, 0.88f, 0.66f);
            glBegin(GL_QUADS);
            glVertex2f(x,y+TILE_SIZE-12); glVertex2f(x+TILE_SIZE,y+TILE_SIZE-12);
            glVertex2f(x+TILE_SIZE,y+TILE_SIZE); glVertex2f(x,y+TILE_SIZE);
            glEnd();
            glColor3f(0.82f, 1.0f, 0.95f);
            glBegin(GL_QUADS);
            glVertex2f(x,y+TILE_SIZE-12); glVertex2f(x+TILE_SIZE,y+TILE_SIZE-12);
            glVertex2f(x+TILE_SIZE,y+TILE_SIZE-9); glVertex2f(x,y+TILE_SIZE-9);
            glEnd();
            glColor3f(0.50f, 0.94f, 0.78f);
            glBegin(GL_TRIANGLES);
            glVertex2f(x+5, y+TILE_SIZE-12); glVertex2f(x+11,y+TILE_SIZE-12); glVertex2f(x+8, y+TILE_SIZE-4);
            glVertex2f(x+18,y+TILE_SIZE-12); glVertex2f(x+24,y+TILE_SIZE-12); glVertex2f(x+21,y+TILE_SIZE-3);
            glVertex2f(x+29,y+TILE_SIZE-12); glVertex2f(x+35,y+TILE_SIZE-12); glVertex2f(x+32,y+TILE_SIZE-5);
            glEnd();
            break;
        case 7:
            glColor3f(0.10f, 0.38f, 0.20f);
            glBegin(GL_QUADS);
            glVertex2f(x,y); glVertex2f(x+TILE_SIZE,y);
            glVertex2f(x+TILE_SIZE,y+TILE_SIZE); glVertex2f(x,y+TILE_SIZE);
            glEnd();
            glColor3f(0.25f, 0.70f, 0.45f);
            glBegin(GL_LINES);
            glVertex2f(x+8, y+4);  glVertex2f(x+8, y+TILE_SIZE-4);
            glVertex2f(x+20,y+4);  glVertex2f(x+20,y+TILE_SIZE-4);
            glVertex2f(x+32,y+4);  glVertex2f(x+32,y+TILE_SIZE-4);
            glEnd();
            glColor3f(0.40f, 0.90f, 0.65f);
            glBegin(GL_LINES);
            glVertex2f(x,y+TILE_SIZE); glVertex2f(x+TILE_SIZE,y+TILE_SIZE);
            glEnd();
            break;
        case 9:
            glColor3f(0.2f, 0.3f, 0.1f);
            glBegin(GL_QUADS);
            glVertex2f(x,y); glVertex2f(x+TILE_SIZE,y);
            glVertex2f(x+TILE_SIZE,y+TILE_SIZE); glVertex2f(x,y+TILE_SIZE);
            glEnd();
            glColor3f(0.6f, 0.8f, 0.3f);
            glBegin(GL_LINES);
            glVertex2f(x+TILE_SIZE*0.2f, y); glVertex2f(x+TILE_SIZE*0.7f, y+TILE_SIZE);
            glVertex2f(x+TILE_SIZE*0.5f, y); glVertex2f(x+TILE_SIZE*1.0f, y+TILE_SIZE);
            glEnd();
            break;
        case 10: {
            // Pulsing exit door
            float pt = glutGet(GLUT_ELAPSED_TIME) * 0.001f;
            float pulse10 = 0.6f + 0.4f * sinf(pt * 3.0f);
            glColor3f(0.2f * pulse10, 0.8f * pulse10, 0.4f * pulse10);
            glBegin(GL_QUADS);
            glVertex2f(x,y); glVertex2f(x+TILE_SIZE,y);
            glVertex2f(x+TILE_SIZE,y+TILE_SIZE); glVertex2f(x,y+TILE_SIZE);
            glEnd();
            glColor3f(0.5f * pulse10, 1.0f * pulse10, 0.6f * pulse10);
            glBegin(GL_QUADS);
            glVertex2f(x+5,y+5); glVertex2f(x+TILE_SIZE-5,y+5);
            glVertex2f(x+TILE_SIZE-5,y+TILE_SIZE-5); glVertex2f(x+5,y+TILE_SIZE-5);
            glEnd();
            // Glow ring
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glColor4f(0.3f, 1.0f, 0.5f, 0.15f * pulse10);
            glBegin(GL_QUADS);
            glVertex2f(x-4,y-4); glVertex2f(x+TILE_SIZE+4,y-4);
            glVertex2f(x+TILE_SIZE+4,y+TILE_SIZE+4); glVertex2f(x-4,y+TILE_SIZE+4);
            glEnd();
            glDisable(GL_BLEND);
            break;
        }
        }
        return;
    }

    // Global time variables for smooth animations
    float timeSec = glutGet(GLUT_ELAPSED_TIME) * 0.001f;
    float pulse = (sinf(timeSec * 3.0f) + 1.0f) * 0.5f;

    // Slowly shift between a bright yellow/orange and a deeper red
    float magmaR = 1.0f;
    float magmaG = 0.2f + (pulse * 0.5f);
    float magmaB = 0.05f;

    if(type == 1)
    {
        // Shadow base
        glColor3f(0.05f, 0.05f, 0.05f);
        glBegin(GL_QUADS);
        glVertex2f(x, y);
        glVertex2f(x + TILE_SIZE, y);
        glVertex2f(x + TILE_SIZE, y + TILE_SIZE);
        glVertex2f(x, y + TILE_SIZE);
        glEnd();

        // Main rock
        glColor3f(0.15f, 0.15f, 0.15f);
        glBegin(GL_QUADS);
        glVertex2f(x + 2, y + 2);
        glVertex2f(x + TILE_SIZE - 2, y + 2);
        glVertex2f(x + TILE_SIZE - 2, y + TILE_SIZE - 2);
        glVertex2f(x + 2, y + TILE_SIZE - 2);
        glEnd();

        // Top highlight
        glColor3f(0.28f, 0.28f, 0.28f);
        glBegin(GL_QUADS);
        glVertex2f(x + 2, y + TILE_SIZE - 6);
        glVertex2f(x + TILE_SIZE - 2, y + TILE_SIZE - 6);
        glVertex2f(x + TILE_SIZE - 2, y + TILE_SIZE - 2);
        glVertex2f(x + 2, y + TILE_SIZE - 2);
        glEnd();

        // Glowing Animated Cracks
        glColor3f(magmaR, magmaG, magmaB);
        glBegin(GL_QUADS);
        glVertex2f(x + 6, y + 22);
        glVertex2f(x + 14, y + 22);
        glVertex2f(x + 14, y + 30);
        glVertex2f(x + 6, y + 30);

        glVertex2f(x + 18, y + 12);
        glVertex2f(x + 26, y + 12);
        glVertex2f(x + 26, y + 20);
        glVertex2f(x + 18, y + 20);

        glVertex2f(x + 28, y + 4);
        glVertex2f(x + TILE_SIZE - 2, y + 4);
        glVertex2f(x + TILE_SIZE - 2, y + 10);
        glVertex2f(x + 28, y + 10);
        glEnd();
    }

    else if (type == 2)
    {
        // BASE ROCK
        glColor3f(0.18f, 0.14f, 0.14f);
        glBegin(GL_QUADS);
        glVertex2f(x, y);
        glVertex2f(x + TILE_SIZE, y);
        glVertex2f(x + TILE_SIZE, y + TILE_SIZE);
        glVertex2f(x, y + TILE_SIZE);
        glEnd();

        // TOP SURFACE
        glColor3f(0.28f, 0.22f, 0.22f);
        glBegin(GL_QUADS);
        glVertex2f(x + 2, y + TILE_SIZE - 10);
        glVertex2f(x + TILE_SIZE - 2, y + TILE_SIZE - 10);
        glVertex2f(x + TILE_SIZE - 2, y + TILE_SIZE - 4);
        glVertex2f(x + 2, y + TILE_SIZE - 4);
        glEnd();

        // BIG CRACK LINES
        glColor3f(0.10f, 0.07f, 0.07f);
        glBegin(GL_QUADS);
        glVertex2f(x + 6,  y + TILE_SIZE - 9);
        glVertex2f(x + 14, y + TILE_SIZE - 9);
        glVertex2f(x + 14, y + TILE_SIZE - 6);
        glVertex2f(x + 6,  y + TILE_SIZE - 6);

        glVertex2f(x + 18, y + TILE_SIZE - 9);
        glVertex2f(x + 30, y + TILE_SIZE - 9);
        glVertex2f(x + 30, y + TILE_SIZE - 6);
        glVertex2f(x + 18, y + TILE_SIZE - 6);
        glEnd();

        // GLOWING MAGMA SEAMS
        glColor3f(magmaR, magmaG, magmaB);
        glBegin(GL_QUADS);
        glVertex2f(x + 8, y + TILE_SIZE - 8);
        glVertex2f(x + 12, y + TILE_SIZE - 8);
        glVertex2f(x + 12, y + TILE_SIZE - 6);
        glVertex2f(x + 8, y + TILE_SIZE - 6);

        glVertex2f(x + 20, y + TILE_SIZE - 8);
        glVertex2f(x + 28, y + TILE_SIZE - 8);
        glVertex2f(x + 28, y + TILE_SIZE - 6);
        glVertex2f(x + 20, y + TILE_SIZE - 6);
        glEnd();

        // EDGE HIGHLIGHT
        glColor3f(0.40f, 0.30f, 0.30f);
        glBegin(GL_LINES);
        glVertex2f(x, y + TILE_SIZE - 2);
        glVertex2f(x + TILE_SIZE, y + TILE_SIZE - 2);
        glEnd();
    }

    else if (type == 3)
    {
        // Faster time variable specifically for fluid fire motion
        float fireTime = timeSec * 10.0f;

        // Base bright lava layer
        glColor3f(1.0f, 0.35f, 0.0f);
        glBegin(GL_QUADS);
        glVertex2f(x, y);
        glVertex2f(x + TILE_SIZE, y);
        glVertex2f(x + TILE_SIZE, y + TILE_SIZE);
        glVertex2f(x, y + TILE_SIZE);
        glEnd();

        // Hotter inner lava offset slightly to give a border effect
        glColor3f(magmaR, magmaG, magmaB);
        glBegin(GL_QUADS);
        glVertex2f(x + 2, y + 2);
        glVertex2f(x + TILE_SIZE - 2, y + 2);
        glVertex2f(x + TILE_SIZE - 2, y + TILE_SIZE - 4);
        glVertex2f(x + 2, y + TILE_SIZE - 4);
        glEnd();

        // Animated flames (Kept your exact logic, they look great!)
        float flame1 = sinf(fireTime + x * 0.05f) * 4.0f;
        float flame2 = sinf(fireTime * 1.3f + x * 0.07f) * 5.0f;
        float flame3 = sinf(fireTime * 1.7f + x * 0.09f) * 3.5f;

        // Dark orange outer flame
        glColor3f(0.95f, 0.20f, 0.0f);
        glBegin(GL_TRIANGLES);
        glVertex2f(x + 4, y + TILE_SIZE - 2);
        glVertex2f(x + 10, y + TILE_SIZE + 8 + flame1);
        glVertex2f(x + 16, y + TILE_SIZE - 2);

        glVertex2f(x + 14, y + TILE_SIZE - 2);
        glVertex2f(x + 22, y + TILE_SIZE + 12 + flame2);
        glVertex2f(x + 30, y + TILE_SIZE - 2);

        glVertex2f(x + 26, y + TILE_SIZE - 2);
        glVertex2f(x + 34, y + TILE_SIZE + 7 + flame3);
        glVertex2f(x + 40, y + TILE_SIZE - 2);
        glEnd();

        // Yellow flame cores
        glColor3f(1.0f, 0.85f, 0.1f);
        glBegin(GL_TRIANGLES);
        glVertex2f(x + 6, y + TILE_SIZE - 2);
        glVertex2f(x + 10, y + TILE_SIZE + 4 + flame1);
        glVertex2f(x + 14, y + TILE_SIZE - 2);

        glVertex2f(x + 17, y + TILE_SIZE - 2);
        glVertex2f(x + 22, y + TILE_SIZE + 7 + flame2);
        glVertex2f(x + 27, y + TILE_SIZE - 2);
        glEnd();

        // Fluid, non-robotic diamond bubbles shifting up and down
        float bubbleOffset = sinf(fireTime * 0.25f + x) * 2.0f;
        glColor3f(1.0f, 0.9f, 0.4f);
        glBegin(GL_QUADS);
        // Diamond 1
        glVertex2f(x + 10, y + 8 + bubbleOffset);
        glVertex2f(x + 12, y + 10 + bubbleOffset);
        glVertex2f(x + 10, y + 12 + bubbleOffset);
        glVertex2f(x + 8,  y + 10 + bubbleOffset);
        // Diamond 2
        glVertex2f(x + 26, y + 16 - bubbleOffset);
        glVertex2f(x + 29, y + 19 - bubbleOffset);
        glVertex2f(x + 26, y + 22 - bubbleOffset);
        glVertex2f(x + 23, y + 19 - bubbleOffset);
        glEnd();
    }

    else if (type == 4)
    {
        // Deep Magma Base
        glColor3f(0.50f, 0.05f, 0.02f);
        glBegin(GL_QUADS);
        glVertex2f(x, y);
        glVertex2f(x + TILE_SIZE, y);
        glVertex2f(x + TILE_SIZE, y + TILE_SIZE);
        glVertex2f(x, y + TILE_SIZE);
        glEnd();

        // Shifting tectonic magma plates (Subtle and flowing)
        float plateShift = sinf(timeSec * 0.8f + x * 0.1f) * 3.0f;

        glColor3f(0.65f, 0.10f, 0.03f);
        glBegin(GL_QUADS);
        // Angled tectonic piece 1
        glVertex2f(x + 2 + plateShift, y + 4);
        glVertex2f(x + TILE_SIZE/2 + plateShift, y + 8);
        glVertex2f(x + TILE_SIZE/2 - 2 + plateShift, y + TILE_SIZE - 6);
        glVertex2f(x + 4 + plateShift, y + TILE_SIZE - 10);
        glEnd();

        glColor3f(0.40f, 0.02f, 0.0f);
        glBegin(GL_TRIANGLES);
        // Floating dark crust
        glVertex2f(x + TILE_SIZE - 4 - plateShift, y + 6);
        glVertex2f(x + TILE_SIZE - 12 - plateShift, y + TILE_SIZE - 8);
        glVertex2f(x + TILE_SIZE/2 + 4 - plateShift, y + 12);
        glEnd();
    }

    else if (type == 12) {
        // Moon Ground (Fixed missing else if condition)
        glColor3f(0.22f, 0.22f, 0.30f);
        glBegin(GL_QUADS);
        glVertex2f(x, y);
        glVertex2f(x + TILE_SIZE, y);
        glVertex2f(x + TILE_SIZE, y + TILE_SIZE);
        glVertex2f(x, y + TILE_SIZE);
        glEnd();

        // Bright Top Edge
        glColor3f(0.80f, 0.85f, 1.0f);
        glBegin(GL_QUADS);
        glVertex2f(x, y + TILE_SIZE - 4);
        glVertex2f(x + TILE_SIZE, y + TILE_SIZE - 4);
        glVertex2f(x + TILE_SIZE, y + TILE_SIZE);
        glVertex2f(x, y + TILE_SIZE);
        glEnd();

        // Inner Panel
        glColor3f(0.30f, 0.30f, 0.40f);
        glBegin(GL_QUADS);
        glVertex2f(x + 3, y + 3);
        glVertex2f(x + TILE_SIZE - 3, y + 3);
        glVertex2f(x + TILE_SIZE - 3, y + TILE_SIZE - 3);
        glVertex2f(x + 3, y + TILE_SIZE - 3);
        glEnd();
    }
    else if (type == 13) {
        // Moving Platform Tile
        glColor3f(1.0f, 0.85f, 0.0f);
        glBegin(GL_QUADS);
        glVertex2f(x, y);
        glVertex2f(x + TILE_SIZE, y);
        glVertex2f(x + TILE_SIZE, y + TILE_SIZE);
        glVertex2f(x, y + TILE_SIZE);
        glEnd();

        glColor3f(1.0f, 1.0f, 0.4f);
        glBegin(GL_QUADS);
        glVertex2f(x + 4, y + 4);
        glVertex2f(x + TILE_SIZE - 4, y + 4);
        glVertex2f(x + TILE_SIZE - 4, y + TILE_SIZE - 4);
        glVertex2f(x + 4, y + TILE_SIZE - 4);
        glEnd();
    }
}

void drawLevel() {
    glBegin(GL_QUADS);

    if (currentActiveLevel == 1) {
        // Red Lava Glow
        glColor3f(0.18f, 0.02f, 0.02f);
        glVertex2f(0, WINDOW_HEIGHT);
        glVertex2f(WINDOW_WIDTH, WINDOW_HEIGHT);
        glColor3f(0.02f, 0.01f, 0.01f);
        glVertex2f(WINDOW_WIDTH, 0);
        glVertex2f(0, 0);
    }
    else if (currentActiveLevel == 2) {
        // Forest Green Glow
        glColor3f(0.02f, 0.20f, 0.08f);
        glVertex2f(0, WINDOW_HEIGHT);
        glVertex2f(WINDOW_WIDTH, WINDOW_HEIGHT);
        glColor3f(0.01f, 0.05f, 0.02f);
        glVertex2f(WINDOW_WIDTH, 0);
        glVertex2f(0, 0);
        glEnd();

        float bgTime = glutGet(GLUT_ELAPSED_TIME) * 0.001f;

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        // Green aurora
        float cy1 = 470 + 16.0f * sinf(bgTime * 0.6f + 470 * 0.012f);
        float a1 = 0.06f + 0.03f * sinf(bgTime + 470 * 0.02f);
        glColor4f(0.15f, 0.75f, 0.35f, a1);
        glBegin(GL_QUADS);
        glVertex2f(0, cy1-28); glVertex2f(WINDOW_WIDTH, cy1-28);
        glVertex2f(WINDOW_WIDTH, cy1+28); glVertex2f(0, cy1+28);
        glEnd();

        float cy2 = 370 + 16.0f * sinf(bgTime * 0.6f + 370 * 0.012f);
        float a2 = 0.06f + 0.03f * sinf(bgTime + 370 * 0.02f);
        glColor4f(0.0f, 0.85f, 0.65f, a2);
        glBegin(GL_QUADS);
        glVertex2f(0, cy2-28); glVertex2f(WINDOW_WIDTH, cy2-28);
        glVertex2f(WINDOW_WIDTH, cy2+28); glVertex2f(0, cy2+28);
        glEnd();

        glDisable(GL_BLEND);

        // Background spires
        glColor3f(0.05f, 0.15f, 0.08f);
        float spires[][3] = {
            {15,28,75},{60,18,50},{100,32,95},{170,22,65},
            {240,28,85},{315,18,55},{385,38,115},{460,22,70},
            {530,28,90},{600,18,60},{660,32,80},{725,26,68},{768,20,48}
        };
        glBegin(GL_TRIANGLES);
        for (int i=0; i<13; i++) {
            glVertex2f(spires[i][0], 80);
            glVertex2f(spires[i][0]+spires[i][1], 80);
            glVertex2f(spires[i][0]+spires[i][1]*0.5f, 80+spires[i][2]);
        }
        glEnd();

        glBegin(GL_QUADS); // Resume QUADS for the next blocks
    }
    else if (currentActiveLevel == 3) {
        // Deep Space Background
        glColor3f(0.02f, 0.02f, 0.15f);
        glVertex2f(0, WINDOW_HEIGHT);
        glVertex2f(WINDOW_WIDTH, WINDOW_HEIGHT);
        glColor3f(0.00f, 0.00f, 0.08f);
        glVertex2f(WINDOW_WIDTH, 0);
        glVertex2f(0, 0);

        glColor4f(0.30f, 0.10f, 0.50f, 0.20f);
        glBegin(GL_POLYGON);
        for(int i = 0; i < 360; i++) {
            float a = i * 3.14159f / 180.0f;
            glVertex2f(250 + cos(a) * 180, 300 + sin(a) * 100);
        }
        glEnd(); // Added missing glEnd for space aura
    }
    glEnd();

    // Draw Map Grid First
    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLS; col++) {
            float px = col * TILE_SIZE;
            float py = WINDOW_HEIGHT - (row + 1) * TILE_SIZE;
            drawTile(px, py, levelMap[row][col]);
        }
    }

    // Draw Level 3 Specific Entities on top of background
    if(currentActiveLevel == 3) {
        drawMoonWorld();
    }
    
    if(currentActiveLevel == 2) {
        if (!level2ParticlesInit) initLevel2Particles();

        float t = glutGet(GLUT_ELAPSED_TIME) * 0.001f;

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // --- Fireflies: glowing green dots that drift in gentle sine waves ---
        for (int i = 0; i < FIREFLY_COUNT; i++) {
            float fx = fireflies[i].x + 30.0f * sinf(t * 0.7f + fireflies[i].phase);
            float fy = fireflies[i].y + 20.0f * cosf(t * 0.5f + fireflies[i].phase * 1.3f);
            float glow = 0.5f + 0.5f * sinf(t * 2.5f + fireflies[i].phase);
            // Outer soft glow
            glColor4f(1.0f, 0.95f, 0.1f, glow * 0.22f);
            glBegin(GL_QUADS);
            glVertex2f(fx-5, fy-5); glVertex2f(fx+5, fy-5);
            glVertex2f(fx+5, fy+5); glVertex2f(fx-5, fy+5);
            glEnd();
            // Bright core
            glColor4f(1.0f, 1.0f, 0.4f, glow * 0.95f);
            glBegin(GL_QUADS);
            glVertex2f(fx-2, fy-2); glVertex2f(fx+2, fy-2);
            glVertex2f(fx+2, fy+2); glVertex2f(fx-2, fy+2);
            glEnd();
        }

        // --- Falling leaves: small green diamonds drifting downward ---
        for (int i = 0; i < LEAF_COUNT; i++) {
            leaves[i].y -= leaves[i].speed;
            if (leaves[i].y < -10) {
                leaves[i].y = 610.0f;
                leaves[i].x = (float)(rand() % 800);
            }
            float lx = leaves[i].x + 18.0f * sinf(t * 1.2f + leaves[i].wobble);
            float ly = leaves[i].y;
            float spin = sinf(t * 1.5f + leaves[i].wobble) * 4.0f;
            glColor4f(0.25f, 0.85f, 0.45f, 0.55f);
            glBegin(GL_TRIANGLES);
            // Small diamond-leaf shape
            glVertex2f(lx,      ly + 5 + spin);
            glVertex2f(lx + 4,  ly);
            glVertex2f(lx,      ly - 5 + spin);

            glVertex2f(lx,      ly + 5 + spin);
            glVertex2f(lx - 4,  ly);
            glVertex2f(lx,      ly - 5 + spin);
            glEnd();
        }

        glDisable(GL_BLEND);

        // --- HUD key guide, bottom-left corner ---
        glColor3f(0.65f, 1.0f, 0.65f);
        glRasterPos2f(10, 20);
        const char* title = "LEVEL 2  |  S: Shrink  |  A/D: Move  |  W: Jump";
        for (const char* c = title; *c; c++) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
        }
    }
}
