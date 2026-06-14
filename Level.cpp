#include "Level.h"
#include "Level1.h"
#include "Level2.h"
#include "level2logic.h"
#include "level3.h"
#include "level3logic.h"
#include "Level4.h"
#include "Level4Logic.h"
#include "Level5.h"
#include "Level5Logic.h"
#include "Player.h"
#include <GL/freeglut.h>
#include <cmath>

int levelMap[ROWS][COLS];
int currentActiveLevel = 1;
bool levelCompleteUI = false;
bool playerDiedUI = false;
float deathTimer = 0.0f;
bool flagTriggered = false;
float levelCompleteTimer = 0.0f;

struct LavaBlob {
    float x, y;
    float vy;
};

// Create 3 lava blobs spaced across the level map
const int MAX_BLOBS = 3;
LavaBlob lavaBlobs[MAX_BLOBS] = {
    { 250.0f, 600.0f, -3.0f },
    { 450.0f, 750.0f, -4.0f },
    { 650.0f, 900.0f, -2.5f }
};

void loadLevel(int levelID) {

    currentActiveLevel = levelID;

    // Reset all UI overlay states safely upon loading a level
    playerDiedUI = false;
    deathTimer = 0.0f;
    levelCompleteUI = false;
    flagTriggered = false;
    levelCompleteTimer = 0.0f;

    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            if (levelID == 1)
            {
                levelMap[r][c] = level1Data[r][c];
            }
            else if (levelID == 2) {
                levelMap[r][c] = level2Data[r][c];
            }
            else if (levelID == 3)
            {
                levelMap[r][c] = level3Data[r][c];
            }
            else if (levelID == 4) // Added: Load your sky map data array
            {
                levelMap[r][c] = level4Data[r][c];
            }
            else if (levelID == 5)
            {
                levelMap[r][c] = level5Data[r][c];
            }
        }
    }

    switch(levelID)
    {
    case 1:
        setSpawnPoint(50.0f, 200.0f);
        break;

    case 2:
        setSpawnPoint(48.0f, 500.0f);
        break;

    case 3:
        setSpawnPoint(10.0f, 180.0f);
        resetLevel3State();
        break;

    case 4:
        setSpawnPoint(60.0f, 250.0f);
        resetLevel4State();
        break;
    case 5:
        setSpawnPoint(50.0f, 250.0f);
        resetLevel5State();
        break;
    }


    // Move the player physically to the new spawn point
    respawnPlayer();
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

            // Solid Ground Mapping: Added Level 5 solid tiles (15, 16)
            // Updated from: Player.cpp
            if (tileType == 1 || tileType == 2 || tileType == 5 || tileType == 6 || tileType == 7 ||
                tileType == 11 || tileType == 12 || tileType == 20 || tileType == 21 ||
                tileType == 15 || tileType == 16) {
                return true;
            }
        }
    }

    // Level 3 Moving Platform Collisions
    if(currentActiveLevel == 3) {
        if(checkPlatformCollision(x, y, width, height)) {
            return true;
        }
    }

    // Level 4 Moving Platform Collisions
    if(currentActiveLevel == 4) {
        if(checkLevel4PlatformCollision(x, y, width, height)) {
            return true;
        }
    }

    // Level 5 Moving Platform Collisions
    if(currentActiveLevel == 5) {
        if(checkPlatform5Collision(x, y, width, height)) {
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
        // Hazard Tracking: Added wind hazards (23) and instant-death deep sky voids (24)
        return (tileType == 3 || tileType == 4 || tileType == 8 || tileType == 9 ||
                tileType == 23 || tileType == 24);
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
        drawLevel2Tile(x, y, type);
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
        float fireTime = timeSec * 10.0f;

        // Base bright lava layer
        glColor3f(1.0f, 0.35f, 0.0f);
        glBegin(GL_QUADS);
        glVertex2f(x, y);
        glVertex2f(x + TILE_SIZE, y);
        glVertex2f(x + TILE_SIZE, y + TILE_SIZE);
        glVertex2f(x, y + TILE_SIZE);
        glEnd();

        glColor3f(magmaR, magmaG, magmaB);
        glBegin(GL_QUADS);
        glVertex2f(x + 2, y + 2);
        glVertex2f(x + TILE_SIZE - 2, y + 2);
        glVertex2f(x + TILE_SIZE - 2, y + TILE_SIZE - 4);
        glVertex2f(x + 2, y + TILE_SIZE - 4);
        glEnd();

        float flame1 = sinf(fireTime + x * 0.05f) * 4.0f;
        float flame2 = sinf(fireTime * 1.3f + x * 0.07f) * 5.0f;
        float flame3 = sinf(fireTime * 1.7f + x * 0.09f) * 3.5f;

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

        glColor3f(1.0f, 0.85f, 0.1f);
        glBegin(GL_TRIANGLES);
        glVertex2f(x + 6, y + TILE_SIZE - 2);
        glVertex2f(x + 10, y + TILE_SIZE + 4 + flame1);
        glVertex2f(x + 14, y + TILE_SIZE - 2);

        glVertex2f(x + 17, y + TILE_SIZE - 2);
        glVertex2f(x + 22, y + TILE_SIZE + 7 + flame2);
        glVertex2f(x + 27, y + TILE_SIZE - 2);
        glEnd();

        float bubbleOffset = sinf(fireTime * 0.25f + x) * 2.0f;
        glColor3f(1.0f, 0.9f, 0.4f);
        glBegin(GL_QUADS);
        glVertex2f(x + 10, y + 8 + bubbleOffset);
        glVertex2f(x + 12, y + 10 + bubbleOffset);
        glVertex2f(x + 10, y + 12 + bubbleOffset);
        glVertex2f(x + 8,  y + 10 + bubbleOffset);

        glVertex2f(x + 26, y + 16 - bubbleOffset);
        glVertex2f(x + 29, y + 19 - bubbleOffset);
        glVertex2f(x + 26, y + 22 - bubbleOffset);
        glVertex2f(x + 23, y + 19 - bubbleOffset);
        glEnd();
    }

    else if (type == 4)
    {
        glColor3f(0.50f, 0.05f, 0.02f);
        glBegin(GL_QUADS);
        glVertex2f(x, y);
        glVertex2f(x + TILE_SIZE, y);
        glVertex2f(x + TILE_SIZE, y + TILE_SIZE);
        glVertex2f(x, y + TILE_SIZE);
        glEnd();

        float plateShift = sinf(timeSec * 0.8f + x * 0.1f) * 3.0f;

        glColor3f(0.65f, 0.10f, 0.03f);
        glBegin(GL_QUADS);
        glVertex2f(x + 2 + plateShift, y + 4);
        glVertex2f(x + TILE_SIZE/2 + plateShift, y + 8);
        glVertex2f(x + TILE_SIZE/2 - 2 + plateShift, y + TILE_SIZE - 6);
        glVertex2f(x + 4 + plateShift, y + TILE_SIZE - 10);
        glEnd();

        glColor3f(0.40f, 0.02f, 0.0f);
        glBegin(GL_TRIANGLES);
        glVertex2f(x + TILE_SIZE - 4 - plateShift, y + 6);
        glVertex2f(x + TILE_SIZE - 12 - plateShift, y + TILE_SIZE - 8);
        glVertex2f(x + TILE_SIZE/2 + 4 - plateShift, y + 12);
        glEnd();
    }

        else if (type == 5) { // Jumping Block - Compressed Spring-Plank on Solid Base
        float timeSec = glutGet(GLUT_ELAPSED_TIME) * 0.001f;
        float animationSpeed = 6.0f;
        float animationAmount = 3.0f; // Kept a bit tighter for a compressed look

        float animatedPlankYOffset = sinf(timeSec * animationSpeed) * animationAmount;

        // 1. Draw the Solid Block Base (Bottom half of the tile)
        float baseHeight = TILE_SIZE * 0.5f;
        glColor3f(0.15f, 0.15f, 0.15f); // Flat dark rock color for base anchor
        glBegin(GL_QUADS);
        glVertex2f(x, y);
        glVertex2f(x + TILE_SIZE, y);
        glVertex2f(x + TILE_SIZE, y + baseHeight);
        glVertex2f(x, y + baseHeight);
        glEnd();

        // Base block top trim highlight
        glColor3f(0.25f, 0.25f, 0.25f);
        glBegin(GL_QUADS);
        glVertex2f(x, y + baseHeight - 3.0f);
        glVertex2f(x + TILE_SIZE, y + baseHeight - 3.0f);
        glVertex2f(x + TILE_SIZE, y + baseHeight);
        glVertex2f(x, y + baseHeight);
        glEnd();

        // 2. Calculate Dynamic Plank Position (Sits near top, bounces up/down)
        float plankHeight = TILE_SIZE * 0.15f;
        float plankY = y + TILE_SIZE - plankHeight - 4.0f + animatedPlankYOffset;
        float plankColorR = 0.65f, plankColorG = 0.45f, plankColorB = 0.25f; // Flat wooden brown

        // 3. Draw the Compressed Spring (Stretches dynamically between base and plank!)
        glColor3f(0.60f, 0.60f, 0.60f); // Sleek flat grey for metal coils
        glLineWidth(3.0f);
        glBegin(GL_LINES);

        float springBottomY = y + baseHeight;
        float springTopY = plankY;
        float dynamicSpringHeight = springTopY - springBottomY;
        int totalCoils = 3; // Fewer coils makes it look smaller and punchier
        float coilSpacing = dynamicSpringHeight / totalCoils;

        // Increased offset to make the spring narrower and centered
        float coilXOffset = TILE_SIZE * 0.35f;
        float currentCoilY = springBottomY;

        for (int i = 0; i < totalCoils; ++i) {
            glVertex2f(x + coilXOffset, currentCoilY);
            glVertex2f(x + TILE_SIZE - coilXOffset, currentCoilY + coilSpacing * 0.5f);

            glVertex2f(x + TILE_SIZE - coilXOffset, currentCoilY + coilSpacing * 0.5f);
            glVertex2f(x + coilXOffset, currentCoilY + coilSpacing);

            currentCoilY += coilSpacing;
        }
        glEnd();
        glLineWidth(1.0f); // Reset line width to default safely

        // 4. Draw the Wooden Plank
        glColor3f(plankColorR, plankColorG, plankColorB);
        glBegin(GL_QUADS);
        glVertex2f(x + 2.0f, plankY); // Slightly tucked in for aesthetic appeal
        glVertex2f(x + TILE_SIZE - 2.0f, plankY);
        glVertex2f(x + TILE_SIZE - 2.0f, plankY + plankHeight);
        glVertex2f(x + 2.0f, plankY + plankHeight);
        glEnd();

        // Plank top edge surface highlight
        glColor3f(plankColorR + 0.1f, plankColorG + 0.1f, plankColorB + 0.1f);
        glBegin(GL_QUADS);
        glVertex2f(x + 2.0f, plankY + plankHeight - 3.0f);
        glVertex2f(x + TILE_SIZE - 2.0f, plankY + plankHeight - 3.0f);
        glVertex2f(x + TILE_SIZE - 2.0f, plankY + plankHeight);
        glVertex2f(x + 2.0f, plankY + plankHeight);
        glEnd();
    }

    else if (type == 12) {
        glColor3f(0.22f, 0.22f, 0.30f);
        glBegin(GL_QUADS);
        glVertex2f(x, y);
        glVertex2f(x + TILE_SIZE, y);
        glVertex2f(x + TILE_SIZE, y + TILE_SIZE);
        glVertex2f(x, y + TILE_SIZE);
        glEnd();

        glColor3f(0.80f, 0.85f, 1.0f);
        glBegin(GL_QUADS);
        glVertex2f(x, y + TILE_SIZE - 4);
        glVertex2f(x + TILE_SIZE, y + TILE_SIZE - 4);
        glVertex2f(x + TILE_SIZE, y + TILE_SIZE);
        glVertex2f(x, y + TILE_SIZE);
        glEnd();

        glColor3f(0.30f, 0.30f, 0.40f);
        glBegin(GL_QUADS);
        glVertex2f(x + 3, y + 3);
        glVertex2f(x + TILE_SIZE - 3, y + 3);
        glVertex2f(x + TILE_SIZE - 3, y + TILE_SIZE - 3);
        glVertex2f(x + 3, y + TILE_SIZE - 3);
        glEnd();
    }
    else if (type == 13) {
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

    else if (type == 14) { // Taller Flag Checkpoint on a Full-Sized Solid Block Base
        // 1. Draw the Solid Block Base (Full standard TILE_SIZE height)
        glColor3f(0.15f, 0.15f, 0.15f); // Flat dark stone
        glBegin(GL_QUADS);
        glVertex2f(x, y);
        glVertex2f(x + TILE_SIZE, y);
        glVertex2f(x + TILE_SIZE, y + TILE_SIZE);
        glVertex2f(x, y + TILE_SIZE);
        glEnd();

        // Base block top edge style trim
        glColor3f(0.25f, 0.25f, 0.25f);
        glBegin(GL_QUADS);
        glVertex2f(x, y + TILE_SIZE - 4.0f);
        glVertex2f(x + TILE_SIZE, y + TILE_SIZE - 4.0f);
        glVertex2f(x + TILE_SIZE, y + TILE_SIZE);
        glVertex2f(x, y + TILE_SIZE);
        glEnd();

        // 2. Draw a Taller Flagpole (Planted directly on top of the block)
        float poleHeight = TILE_SIZE * 1.6f; // Stands 1.6x tiles taller above the base
        float poleTopY = y + TILE_SIZE + poleHeight;

        glColor3f(0.3f, 0.3f, 0.3f); // Dark iron flagpole
        glBegin(GL_QUADS);
        glVertex2f(x + TILE_SIZE * 0.45f, y + TILE_SIZE);
        glVertex2f(x + TILE_SIZE * 0.55f, y + TILE_SIZE);
        glVertex2f(x + TILE_SIZE * 0.55f, poleTopY);
        glVertex2f(x + TILE_SIZE * 0.45f, poleTopY);
        glEnd();

        // Wave physics calculations
        float waveTime = glutGet(GLUT_ELAPSED_TIME) * 0.006f;
        float waveX = sinf(waveTime) * 5.0f;

        if (flagTriggered) {
            glColor3f(0.2f, 0.8f, 0.2f); // Green when claimed
        } else {
            glColor3f(0.9f, 0.2f, 0.2f); // Red when unclaimed
        }

        // 3. Draw Waving Flag (Flipped to point LEFT away from the pole)
        glBegin(GL_TRIANGLES);
        glVertex2f(x + TILE_SIZE * 0.45f, poleTopY);
        glVertex2f(x + TILE_SIZE * 0.45f, poleTopY - TILE_SIZE * 0.7f);
        glVertex2f(x - TILE_SIZE * 0.6f - waveX, poleTopY - TILE_SIZE * 0.35f);
        glEnd();
    }

    else if (type == 16) {
        // Jungle Ground / Tree Trunk
        glColor3f(0.32f, 0.20f, 0.10f);
        glBegin(GL_QUADS);
        glVertex2f(x, y);
        glVertex2f(x + TILE_SIZE, y);
        glVertex2f(x + TILE_SIZE, y + TILE_SIZE);
        glVertex2f(x, y + TILE_SIZE);
        glEnd();

        // Grassy top layer
        glColor3f(0.18f, 0.55f, 0.18f);
        glBegin(GL_QUADS);
        glVertex2f(x, y + TILE_SIZE - 8);
        glVertex2f(x + TILE_SIZE, y + TILE_SIZE - 8);
        glVertex2f(x + TILE_SIZE, y + TILE_SIZE);
        glVertex2f(x, y + TILE_SIZE);
        glEnd();

        // Bark texture lines
        glColor3f(0.22f, 0.13f, 0.06f);
        glBegin(GL_LINES);
        glVertex2f(x + 8, y + 2);
        glVertex2f(x + 8, y + TILE_SIZE - 10);
        glVertex2f(x + 26, y + 2);
        glVertex2f(x + 26, y + TILE_SIZE - 10);
        glEnd();

        // Swaying grass tufts on top
        float swayG = sinf(timeSec * 2.0f + x * 0.07f) * 2.0f;
        glColor3f(0.28f, 0.72f, 0.22f);
        glBegin(GL_TRIANGLES);
        glVertex2f(x + 3, y + TILE_SIZE - 6);
        glVertex2f(x + 8 + swayG, y + TILE_SIZE + 7);
        glVertex2f(x + 13, y + TILE_SIZE - 6);

        glVertex2f(x + 17, y + TILE_SIZE - 6);
        glVertex2f(x + 22 + swayG, y + TILE_SIZE + 9);
        glVertex2f(x + 27, y + TILE_SIZE - 6);

        glVertex2f(x + 27, y + TILE_SIZE - 6);
        glVertex2f(x + 32 + swayG, y + TILE_SIZE + 6);
        glVertex2f(x + 37, y + TILE_SIZE - 6);
        glEnd();

        // Tiny jungle flower accents (deterministic per-tile placement)
        float flowerPhase = fmodf(x * 0.37f, 3.0f);
        if (flowerPhase < 1.0f) {
            glColor3f(0.95f, 0.30f, 0.55f);
            glBegin(GL_QUADS);
            glVertex2f(x + 20, y + TILE_SIZE - 4);
            glVertex2f(x + 24, y + TILE_SIZE - 4);
            glVertex2f(x + 24, y + TILE_SIZE);
            glVertex2f(x + 20, y + TILE_SIZE);
            glEnd();
        } else if (flowerPhase < 2.0f) {
            glColor3f(1.0f, 0.85f, 0.2f);
            glBegin(GL_QUADS);
            glVertex2f(x + 5, y + TILE_SIZE - 4);
            glVertex2f(x + 9, y + TILE_SIZE - 4);
            glVertex2f(x + 9, y + TILE_SIZE);
            glVertex2f(x + 5, y + TILE_SIZE);
            glEnd();
        }
    }

    else if (type == 15) {
        // Branch Platform
        glColor3f(0.42f, 0.27f, 0.13f);
        glBegin(GL_QUADS);
        glVertex2f(x, y);
        glVertex2f(x + TILE_SIZE, y);
        glVertex2f(x + TILE_SIZE, y + TILE_SIZE - 6);
        glVertex2f(x, y + TILE_SIZE - 6);
        glEnd();

        // Leafy top
        glColor3f(0.20f, 0.62f, 0.20f);
        glBegin(GL_QUADS);
        glVertex2f(x, y + TILE_SIZE - 6);
        glVertex2f(x + TILE_SIZE, y + TILE_SIZE - 6);
        glVertex2f(x + TILE_SIZE, y + TILE_SIZE);
        glVertex2f(x, y + TILE_SIZE);
        glEnd();

        // Bark ring highlight
        glColor3f(0.55f, 0.38f, 0.20f);
        glBegin(GL_QUADS);
        glVertex2f(x + 2, y + 2);
        glVertex2f(x + TILE_SIZE - 2, y + 2);
        glVertex2f(x + TILE_SIZE - 2, y + 6);
        glVertex2f(x + 2, y + 6);
        glEnd();

        // Small swaying leaf tufts on top of the branch
        float swayB = sinf(timeSec * 2.5f + x * 0.09f) * 1.5f;
        glColor3f(0.32f, 0.78f, 0.28f);
        glBegin(GL_TRIANGLES);
        glVertex2f(x + 6, y + TILE_SIZE);
        glVertex2f(x + 10 + swayB, y + TILE_SIZE + 9);
        glVertex2f(x + 14, y + TILE_SIZE);

        glVertex2f(x + 26, y + TILE_SIZE);
        glVertex2f(x + 30 + swayB, y + TILE_SIZE + 9);
        glVertex2f(x + 34, y + TILE_SIZE);
        glEnd();
    }

    else if (type == 17) {
        // Hanging Thorns hazard
        float sway = sinf(timeSec * 4.0f + x * 0.05f) * 3.0f;

        // Vine
        glColor3f(0.15f, 0.35f, 0.10f);
        glBegin(GL_QUADS);
        glVertex2f(x + TILE_SIZE / 2 - 2 + sway, y);
        glVertex2f(x + TILE_SIZE / 2 + 2 + sway, y);
        glVertex2f(x + TILE_SIZE / 2 + 2, y + TILE_SIZE);
        glVertex2f(x + TILE_SIZE / 2 - 2, y + TILE_SIZE);
        glEnd();

        // Thorn spikes
        glColor3f(0.55f, 0.15f, 0.10f);
        glBegin(GL_TRIANGLES);
        glVertex2f(x + 4 + sway, y + TILE_SIZE);
        glVertex2f(x + 14 + sway, y + TILE_SIZE);
        glVertex2f(x + 9 + sway, y + TILE_SIZE - 16);

        glVertex2f(x + TILE_SIZE - 14 + sway, y + TILE_SIZE);
        glVertex2f(x + TILE_SIZE - 4 + sway, y + TILE_SIZE);
        glVertex2f(x + TILE_SIZE - 9 + sway, y + TILE_SIZE - 20);
        glEnd();
    }

    else if (type == 18) {
        // Poison Swamp hazard
        glColor3f(0.18f, 0.30f, 0.10f);
        glBegin(GL_QUADS);
        glVertex2f(x, y);
        glVertex2f(x + TILE_SIZE, y);
        glVertex2f(x + TILE_SIZE, y + TILE_SIZE);
        glVertex2f(x, y + TILE_SIZE);
        glEnd();

        // Toxic surface ripple
        float ripple = (sinf(timeSec * 2.5f + x * 0.08f) + 1.0f) * 0.5f;
        glColor3f(0.35f + ripple * 0.25f, 0.55f + ripple * 0.25f, 0.10f);
        glBegin(GL_QUADS);
        glVertex2f(x, y + TILE_SIZE - 8);
        glVertex2f(x + TILE_SIZE, y + TILE_SIZE - 8);
        glVertex2f(x + TILE_SIZE, y + TILE_SIZE);
        glVertex2f(x, y + TILE_SIZE);
        glEnd();

        // Bubbles
        glColor3f(0.6f, 0.85f, 0.3f);
        glBegin(GL_QUADS);
        glVertex2f(x + 8, y + 10 + ripple * 4);
        glVertex2f(x + 12, y + 10 + ripple * 4);
        glVertex2f(x + 12, y + 14 + ripple * 4);
        glVertex2f(x + 8, y + 14 + ripple * 4);

        glVertex2f(x + 24, y + 16 - ripple * 4);
        glVertex2f(x + 29, y + 16 - ripple * 4);
        glVertex2f(x + 29, y + 21 - ripple * 4);
        glVertex2f(x + 24, y + 21 - ripple * 4);
        glEnd();
    }

    else if (type == 19) {
        // Ground Spikes hazard
        glColor3f(0.12f, 0.10f, 0.10f);
        glBegin(GL_QUADS);
        glVertex2f(x, y);
        glVertex2f(x + TILE_SIZE, y);
        glVertex2f(x + TILE_SIZE, y + TILE_SIZE - 10);
        glVertex2f(x, y + TILE_SIZE - 10);
        glEnd();

        // Sharp metallic spikes
        glColor3f(0.55f, 0.55f, 0.60f);
        glBegin(GL_TRIANGLES);
        glVertex2f(x + 2, y + TILE_SIZE - 10);
        glVertex2f(x + 9, y + TILE_SIZE + 8);
        glVertex2f(x + 16, y + TILE_SIZE - 10);

        glVertex2f(x + 14, y + TILE_SIZE - 10);
        glVertex2f(x + 21, y + TILE_SIZE + 10);
        glVertex2f(x + 28, y + TILE_SIZE - 10);

        glVertex2f(x + 26, y + TILE_SIZE - 10);
        glVertex2f(x + 33, y + TILE_SIZE + 6);
        glVertex2f(x + TILE_SIZE - 2, y + TILE_SIZE - 10);
        glEnd();

        // Spike edge highlight
        glColor3f(0.85f, 0.85f, 0.90f);
        glBegin(GL_LINES);
        glVertex2f(x + 9, y + TILE_SIZE + 8);
        glVertex2f(x + 9, y + TILE_SIZE - 4);
        glVertex2f(x + 21, y + TILE_SIZE + 10);
        glVertex2f(x + 21, y + TILE_SIZE - 4);
        glEnd();
    }

    else if (type == 25) {
    float timeSec = glutGet(GLUT_ELAPSED_TIME) * 0.001f;
    float rotationAngle = timeSec * 50.0f;
    float pulse = (sinf(timeSec * 8.0f) + 1.0f) * 0.5f;
    float cx = x + TILE_SIZE / 2.0f;
    float cy = y + TILE_SIZE / 2.0f;

    // Center hub
    glColor3f(0.15f, 0.15f, 0.15f);
    glBegin(GL_POLYGON);
    for(int j = 0; j < 360; j += 30) {
        float a = j * 3.14159f / 180.0f;
        glVertex2f(cx + 12.0f * cosf(a), cy + 12.0f * sinf(a));
    }
    glEnd();

    glPushMatrix();
    glTranslatef(cx, cy, 0.0f);
    glRotatef(rotationAngle, 0.0f, 0.0f, 1.0f);

    // Blade Layer 1
    glColor3f(0.8f + (0.2f * pulse), 0.1f, 0.1f);
    glBegin(GL_TRIANGLES);
    glVertex2f(12.0f, -4.0f);
    glVertex2f(TILE_SIZE * 1.2f, 0.0f); // Blade length shortened
    glVertex2f(12.0f, 4.0f);

    glVertex2f(-12.0f, -4.0f);
    glVertex2f(-TILE_SIZE * 1.2f, 0.0f); // Blade length shortened
    glVertex2f(-12.0f, 4.0f);
    glEnd();

    // Blade Layer 2
    glColor3f(0.9f, 0.6f + (0.4f * pulse), 0.1f);
    glBegin(GL_TRIANGLES);
    glVertex2f(15.0f, -2.0f);
    glVertex2f(TILE_SIZE * 1.1f, 0.0f); // Blade length shortened
    glVertex2f(15.0f, 2.0f);

    glVertex2f(-15.0f, -2.0f);
    glVertex2f(-TILE_SIZE * 1.1f, 0.0f); // Blade length shortened
    glVertex2f(-15.0f, 2.0f);
    glEnd();

    // Spinner decoration
    glRotatef(-rotationAngle * 2.0f, 0.0f, 0.0f, 1.0f);
    glColor3f(0.9f, 0.2f, 0.2f);
    glBegin(GL_POLYGON);
    for(int j = 0; j < 360; j += 60) {
        float a = j * 3.14159f / 180.0f;
        glVertex2f(6.0f * cosf(a), 6.0f * sinf(a));
    }
    glEnd();

    glPopMatrix();
}

    // ─── LEVEL 4 SKY TILES RENDERING ───
    else if (type == 20) { // Cloud Platform (Solid Walkable Top Layer)
        glColor3f(0.95f, 0.96f, 1.00f);
        glBegin(GL_QUADS);
        glVertex2f(x, y);
        glVertex2f(x + TILE_SIZE, y);
        glVertex2f(x + TILE_SIZE, y + TILE_SIZE);
        glVertex2f(x, y + TILE_SIZE);
        glEnd();

        // Shaded underside line
        glColor3f(0.78f, 0.83f, 0.93f);
        glBegin(GL_QUADS);
        glVertex2f(x, y);
        glVertex2f(x + TILE_SIZE, y);
        glVertex2f(x + TILE_SIZE, y + 4);
        glVertex2f(x, y + 4);
        glEnd();
    }
    else if (type == 21) { // Cloud Block (Full Cloud Interior)
        glColor3f(0.90f, 0.92f, 0.98f);
        glBegin(GL_QUADS);
        glVertex2f(x, y);
        glVertex2f(x + TILE_SIZE, y);
        glVertex2f(x + TILE_SIZE, y + TILE_SIZE);
        glVertex2f(x, y + TILE_SIZE);
        glEnd();
    }
    else if (type == 22) { // Sun Tile (Decorative / Placeholder background elements)
        glColor3f(1.0f, 0.92f, 0.40f);
        glBegin(GL_QUADS);
        glVertex2f(x + 2, y + 2);
        glVertex2f(x + TILE_SIZE - 2, y + 2);
        glVertex2f(x + TILE_SIZE - 2, y + TILE_SIZE - 2);
        glVertex2f(x + 2, y + TILE_SIZE - 2);
        glEnd();
    }
    else if (type == 23) { // Wind/Storm Hazard
        float windShift = sinf(timeSec * 12.0f + x) * 6.0f;
        glColor3f(0.75f, 0.85f, 0.95f);
        glLineWidth(2.0f);
        glBegin(GL_LINES);
        glVertex2f(x + windShift, y + 6);
        glVertex2f(x + TILE_SIZE + windShift, y + 6);
        glVertex2f(x - windShift, y + TILE_SIZE - 6);
        glVertex2f(x + TILE_SIZE - windShift, y + TILE_SIZE - 6);
        glEnd();
        glLineWidth(1.0f);
    }
    else if (type == 24) { // Deep Sky / Void Hazard (Instant Death)
        // Dark purple/blue gradient fading outward to signal danger
        glColor3f(0.08f, 0.05f, 0.18f);
        glBegin(GL_QUADS);
        glVertex2f(x, y);
        glVertex2f(x + TILE_SIZE, y);
        glVertex2f(x + TILE_SIZE, y + TILE_SIZE);
        glVertex2f(x, y + TILE_SIZE);
        glEnd();
    }
}

void drawLavaBackground() {
    float timeSec = glutGet(GLUT_ELAPSED_TIME) * 0.001f;

    // 1. Sky Background (Deep charcoal/lava-warmed dark red)
    glColor3f(0.12f, 0.04f, 0.03f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(WINDOW_WIDTH, 0);
    glVertex2f(WINDOW_WIDTH, WINDOW_HEIGHT);
    glVertex2f(0, WINDOW_HEIGHT);
    glEnd();

    // 2. Far Mountains (Faded, warm dark ash)
    glColor3f(0.22f, 0.12f, 0.08f);
    glBegin(GL_TRIANGLES);
    glVertex2f(-100.0f, 250.0f); glVertex2f(150.0f, 480.0f); glVertex2f(450.0f, 250.0f);
    glVertex2f(250.0f, 250.0f); glVertex2f(600.0f, 550.0f); glVertex2f(950.0f, 250.0f);
    glVertex2f(700.0f, 250.0f); glVertex2f(900.0f, 400.0f); glVertex2f(1200.0f, 250.0f);
    glEnd();

    // 3. Mid Mountains (Slightly darker, earthy brown)
    glColor3f(0.14f, 0.07f, 0.05f);
    glBegin(GL_TRIANGLES);
    glVertex2f(-50.0f, 150.0f); glVertex2f(200.0f, 380.0f); glVertex2f(450.0f, 150.0f);
    glVertex2f(300.0f, 150.0f); glVertex2f(550.0f, 420.0f); glVertex2f(800.0f, 150.0f);
    glVertex2f(650.0f, 150.0f); glVertex2f(850.0f, 320.0f); glVertex2f(1100.0f, 150.0f);
    glEnd();

    // 4. Near Mountains (Crisp foreground, dark burnt umber)
    glColor3f(0.08f, 0.03f, 0.02f);
    glBegin(GL_TRIANGLES);
    glVertex2f(-100.0f, 0.0f); glVertex2f(100.0f, 250.0f); glVertex2f(350.0f, 0.0f);
    glVertex2f(150.0f, 0.0f); glVertex2f(400.0f, 280.0f); glVertex2f(650.0f, 0.0f);
    glVertex2f(450.0f, 0.0f); glVertex2f(750.0f, 300.0f); glVertex2f(1050.0f, 0.0f);
    glEnd();

    // 5. Animating Subtle Mist (Heated, ember-tinted haze)
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Mist Layer 1 (Slow, shifting warm haze)
    float mistOffset1 = sinf(timeSec * 0.4f) * 40.0f;
    glColor4f(0.5f, 0.25f, 0.15f, 0.15f); // Tinted warm/orangey
    glBegin(GL_QUADS);
    glVertex2f(0.0f, 200.0f + mistOffset1);
    glVertex2f(WINDOW_WIDTH, 150.0f - mistOffset1);
    glVertex2f(WINDOW_WIDTH, 0.0f);
    glVertex2f(0.0f, 0.0f);
    glEnd();

    // Mist Layer 2 (Faster, rolling ember glow)
    float mistOffset2 = cosf(timeSec * 0.6f) * 30.0f;
    glColor4f(0.6f, 0.3f, 0.1f, 0.1f); // Slightly more vibrant orange
    glBegin(GL_QUADS);
    glVertex2f(0.0f, 100.0f - mistOffset2);
    glVertex2f(WINDOW_WIDTH, 120.0f + mistOffset2);
    glVertex2f(WINDOW_WIDTH, 0.0f);
    glVertex2f(0.0f, 0.0f);
    glEnd();

    glDisable(GL_BLEND);
}

void drawLevel() {
    glPushMatrix();

    if (playerDiedUI && deathTimer < 0.5f) {
        float shakeX = sinf(deathTimer * 50.0f) * 10.0f;
        float shakeY = cosf(deathTimer * 45.0f) * 10.0f;
        glTranslatef(shakeX, shakeY, 0.0f);
    }

    if (currentActiveLevel == 1) {
        glBegin(GL_QUADS);
        drawLavaBackground();
        glColor3f(0.18f, 0.02f, 0.02f);
        glVertex2f(0, WINDOW_HEIGHT);
        glVertex2f(WINDOW_WIDTH, WINDOW_HEIGHT);
        glColor3f(0.02f, 0.01f, 0.01f);
        glVertex2f(WINDOW_WIDTH, 0);
        glVertex2f(0, 0);
        glEnd();
    }
    else if (currentActiveLevel == 2) {
        glBegin(GL_QUADS);
        glColor3f(0.02f, 0.08f, 0.20f);
        glVertex2f(0, WINDOW_HEIGHT);
        glVertex2f(WINDOW_WIDTH, WINDOW_HEIGHT);
        glColor3f(0.01f, 0.02f, 0.05f);
        glVertex2f(WINDOW_WIDTH, 0);
        glVertex2f(0, 0);
        glEnd();
    }
    else if (currentActiveLevel == 3) {
        glBegin(GL_QUADS);
        glColor3f(0.02f, 0.02f, 0.15f);
        glVertex2f(0, WINDOW_HEIGHT);
        glVertex2f(WINDOW_WIDTH, WINDOW_HEIGHT);
        glColor3f(0.00f, 0.00f, 0.08f);
        glVertex2f(WINDOW_WIDTH, 0);
        glVertex2f(0, 0);
        glEnd();

        glColor4f(0.30f, 0.10f, 0.50f, 0.20f);
        glBegin(GL_POLYGON);
        for(int i = 0; i < 360; i++) {
            float a = i * 3.14159f / 180.0f;
            glVertex2f(250 + cos(a) * 180, 300 + sin(a) * 100);
        }
        glEnd();
    }
    else if (currentActiveLevel == 4) {
        glBegin(GL_QUADS);
        glColor3f(0.53f, 0.81f, 0.98f);
        glVertex2f(0, WINDOW_HEIGHT);
        glVertex2f(WINDOW_WIDTH, WINDOW_HEIGHT);
        glColor3f(0.92f, 0.95f, 1.00f);
        glVertex2f(WINDOW_WIDTH, 0);
        glVertex2f(0, 0);
        glEnd();
    }

    else if (currentActiveLevel == 5) {
        // Jungle Canopy Glow
        glColor3f(0.06f, 0.18f, 0.06f);
        glVertex2f(0, WINDOW_HEIGHT);
        glVertex2f(WINDOW_WIDTH, WINDOW_HEIGHT);
        glColor3f(0.45f, 0.65f, 0.30f);
        glVertex2f(WINDOW_WIDTH, 0);
        glVertex2f(0, 0);
    }

    // Jungle background layer (mountains, sun, distant trees) sits behind the tile grid
    if (currentActiveLevel == 5) {
        drawJungleBackground();
    }

    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLS; col++) {
            float px = col * TILE_SIZE;
            float py = WINDOW_HEIGHT - (row + 1) * TILE_SIZE;
            drawTile(px, py, levelMap[row][col]);
        }
    }

    if(currentActiveLevel == 3) {
        drawMoonWorld();
    }

    if(currentActiveLevel == 4) {
        drawSkyWorld();
    }
    if(currentActiveLevel == 5) {
        drawJungleWorld();
    }

    glPopMatrix();
}

bool isTouchingBouncePad(float x, float y, float width, float height) {
    // We check just slightly below the player's feet (y - 1.0f)
    float feetY = y - 1.0f;
    float centerX = x + width / 2.0f;

    int col = (int)(centerX / TILE_SIZE);
    int row = (int)((WINDOW_HEIGHT - feetY) / TILE_SIZE);

    if (row >= 0 && row < ROWS && col >= 0 && col < COLS) {
        if (levelMap[row][col] == 5) {
            return true;
        }
    }
    return false;
}

void updateLevelElements() {
    // 1. Level Complete Sequence Tracker
    if (flagTriggered) {
        levelCompleteTimer += 0.016f;
        if (levelCompleteTimer >= 3.0f) {
            // Clear tracking variables before starting the next map
            flagTriggered = false;
            levelCompleteUI = false;
            levelCompleteTimer = 0.0f;

            loadLevel(2); // This will invoke setSpawnPoint(50.0f, 200.0f) or similar internally!
        }
        return; // Stops hazards from updating while winning
    }

    // 2. Player Death Sequence Tracker
    if (playerDiedUI) {
        deathTimer += 0.016f;
        if (deathTimer >= 2.0f) { // Show the UI for 2 seconds
            playerDiedUI = false;
            deathTimer = 0.0f;
            respawnPlayer(); // Triggers your engine to reset player position
        }
        return; // Pauses the lava blobs from moving while the death UI is shown
    }

    // 3. Falling Lava Blobs Physics
    if (currentActiveLevel == 1) {
        for (int i = 0; i < MAX_BLOBS; i++) {
            lavaBlobs[i].y += lavaBlobs[i].vy;

            // Reset back to sky when it dips below the window floor
            if (lavaBlobs[i].y < -20.0f) {
                lavaBlobs[i].y = WINDOW_HEIGHT + 50.0f;
            }

            // Adjusted AABB Collision for the new circular blobs
            if (lavaBlobs[i].x + 14.0f > playerX && lavaBlobs[i].x < playerX + 24.0f &&
                lavaBlobs[i].y + 14.0f > playerY && lavaBlobs[i].y < playerY + 32.0f) {

                playerDiedUI = true; // Trigger the UI overlay instead of instant death
            }
        }
    }

    float bladeTime = glutGet(GLUT_ELAPSED_TIME) * 0.001f;
    float rotationAngleRad = (bladeTime * 50.0f) * 3.14159f / 180.0f;
    // Set to 1.2f to match the drawing logic above
    float bladeLength = TILE_SIZE * 1.2f;

    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            if (levelMap[r][c] == 25) {
                float cx = c * TILE_SIZE + TILE_SIZE / 2.0f;
                float cy = WINDOW_HEIGHT - (r + 1) * TILE_SIZE + TILE_SIZE / 2.0f;

                // Check collision only along the blade length
                for (float step = 15.0f; step <= bladeLength; step += 8.0f) {
                    float bx1 = cx + step * cosf(rotationAngleRad);
                    float by1 = cy + step * sinf(rotationAngleRad);
                    float bx2 = cx - step * cosf(rotationAngleRad);
                    float by2 = cy - step * sinf(rotationAngleRad);

                    if ((bx1 > playerX && bx1 < playerX + 24.0f && by1 > playerY && by1 < playerY + 32.0f) ||
                        (bx2 > playerX && bx2 < playerX + 24.0f && by2 > playerY && by2 < playerY + 32.0f)) {
                        playerDiedUI = true;
                    }
                }
            }
        }
    }
}

void drawLevelUI() {
    // 1. Render Active Lava Hazards (Round and pulsating!)
    if (currentActiveLevel == 1 && !levelCompleteUI && !playerDiedUI) {
        float timeSec = glutGet(GLUT_ELAPSED_TIME) * 0.005f;
        float pulseScale = 1.0f + sinf(timeSec) * 0.2f;

        for (int i = 0; i < MAX_BLOBS; i++) {
            float centerX = lavaBlobs[i].x + 7.0f;
            float centerY = lavaBlobs[i].y + 7.0f;
            float radius = 8.0f * pulseScale;

            glColor3f(1.0f, 0.4f, 0.0f);
            glBegin(GL_POLYGON);
            for(int j = 0; j < 360; j += 20) {
                float theta = j * 3.14159f / 180.0f;
                glVertex2f(centerX + radius * cosf(theta), centerY + radius * sinf(theta));
            }
            glEnd();

            glColor3f(1.0f, 0.8f, 0.0f);
            glBegin(GL_POLYGON);
            for(int j = 0; j < 360; j += 30) {
                float theta = j * 3.14159f / 180.0f;
                glVertex2f(centerX + (radius * 0.5f) * cosf(theta), centerY + (radius * 0.5f) * sinf(theta));
            }
            glEnd();
        }
    }

    // 2. Render Victory Announcement Screen
    if (levelCompleteUI) {
        glColor4f(0.0f, 0.0f, 0.0f, 0.65f); // Black overlay
        glBegin(GL_QUADS);
        glVertex2f(WINDOW_WIDTH * 0.15f, WINDOW_HEIGHT * 0.35f);
        glVertex2f(WINDOW_WIDTH * 0.85f, WINDOW_HEIGHT * 0.35f);
        glVertex2f(WINDOW_WIDTH * 0.85f, WINDOW_HEIGHT * 0.65f);
        glVertex2f(WINDOW_WIDTH * 0.15f, WINDOW_HEIGHT * 0.65f); // FIXED: Corrected top-left vertex
        glEnd();

        glColor3f(1.0f, 0.82f, 0.0f); // Gold outline
        glLineWidth(3.0f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(WINDOW_WIDTH * 0.18f, WINDOW_HEIGHT * 0.38f);
        glVertex2f(WINDOW_WIDTH * 0.82f, WINDOW_HEIGHT * 0.38f);
        glVertex2f(WINDOW_WIDTH * 0.82f, WINDOW_HEIGHT * 0.62f);
        glVertex2f(WINDOW_WIDTH * 0.18f, WINDOW_HEIGHT * 0.62f);
        glEnd();
        glLineWidth(1.0f);

        glColor3f(1.0f, 1.0f, 1.0f);
        glRasterPos2f(WINDOW_WIDTH * 0.38f, WINDOW_HEIGHT * 0.48f);
        const char* textMessage = "LEVEL 1 COMPLETE!";
        for (const char* letter = textMessage; *letter != '\0'; letter++) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *letter);
        }
    }

    // 3. Render Death Screen
    if (playerDiedUI) {
        glColor4f(0.5f, 0.0f, 0.0f, 0.65f); // Dark red overlay
        glBegin(GL_QUADS);
        glVertex2f(WINDOW_WIDTH * 0.15f, WINDOW_HEIGHT * 0.35f);
        glVertex2f(WINDOW_WIDTH * 0.85f, WINDOW_HEIGHT * 0.35f);
        glVertex2f(WINDOW_WIDTH * 0.85f, WINDOW_HEIGHT * 0.65f);
        glVertex2f(WINDOW_WIDTH * 0.15f, WINDOW_HEIGHT * 0.65f); // FIXED: Corrected top-left vertex
        glEnd();

        glColor3f(1.0f, 0.2f, 0.2f); // Bright red outline
        glLineWidth(3.0f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(WINDOW_WIDTH * 0.18f, WINDOW_HEIGHT * 0.38f);
        glVertex2f(WINDOW_WIDTH * 0.82f, WINDOW_HEIGHT * 0.38f);
        glVertex2f(WINDOW_WIDTH * 0.82f, WINDOW_HEIGHT * 0.62f);
        glVertex2f(WINDOW_WIDTH * 0.18f, WINDOW_HEIGHT * 0.62f);
        glEnd();
        glLineWidth(1.0f);

        glColor3f(1.0f, 1.0f, 1.0f);
        glRasterPos2f(WINDOW_WIDTH * 0.44f, WINDOW_HEIGHT * 0.48f);
        const char* deadText = "YOU DIED!";
        for (const char* letter = deadText; *letter != '\0'; letter++) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *letter);
        }
    }
}
