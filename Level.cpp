#include "Level.h"
#include "Level1.h"
#include "level3.h"
#include "level3logic.h"
#include "Player.h"
#include <GL/freeglut.h>
#include <cmath>

int levelMap[ROWS][COLS];
int currentActiveLevel = 1;

void loadLevel(int levelID) {
    currentActiveLevel = levelID;


    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            if (levelID == 1) {
                levelMap[r][c] = level1Data[r][c];
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
            // Treat Lava Rock (1, 2) and Moon Ground (11, 12) as solid ground
            if (tileType == 1 || tileType == 2 || tileType == 6 || tileType == 7 || tileType == 11 || tileType == 12) {
                return true;
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
        // Treat Lava (3, 4) and Freezing Water (8, 9) as deadly hazards
        return (tileType == 3 || tileType == 4 || tileType == 8 || tileType == 9);
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
        // Frosty Blue Glow
        glColor3f(0.02f, 0.08f, 0.20f);
        glVertex2f(0, WINDOW_HEIGHT);
        glVertex2f(WINDOW_WIDTH, WINDOW_HEIGHT);
        glColor3f(0.01f, 0.02f, 0.05f);
        glVertex2f(WINDOW_WIDTH, 0);
        glVertex2f(0, 0);
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
}
