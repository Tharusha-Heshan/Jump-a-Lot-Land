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

    // Load the correct external array based on the requested level
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

    if (levelID == 1) {
        setSpawnPoint(50.0f, 250.0f);
    } else if (levelID == 2) {
        setSpawnPoint(50.0f, 150.0f);
    }
    else if (levelID == 3) {
        setSpawnPoint(10.0f, 180.0f);
        resetLevel3State();
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
            // Treat both Lava Rock (1, 2) and Ice (6, 7) as solid ground
            if (tileType == 1 || tileType == 2 || tileType == 6 || tileType == 7 || tileType==11 || tileType ==12) {
                return true;
            }
        }
    }
    if(currentActiveLevel == 3)
    {
        if(checkPlatformCollision(x, y, width, height))
            return true;
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

void drawTile(float x, float y, int type) {
    if (type == 0) return;

    // --- LAVA THEME TILES ---
    if (type == 1 || type == 2) {
        glColor3f(0.12f, 0.08f, 0.15f);
        glBegin(GL_QUADS);
        glVertex2f(x, y); glVertex2f(x + TILE_SIZE, y);
        glVertex2f(x + TILE_SIZE, y + TILE_SIZE); glVertex2f(x, y + TILE_SIZE);
        glEnd();

        glColor3f(0.2f, 0.15f, 0.25f);
        glBegin(GL_QUADS);
        glVertex2f(x + 4, y + 4); glVertex2f(x + TILE_SIZE - 4, y + 4);
        glVertex2f(x + TILE_SIZE - 4, y + TILE_SIZE - 4); glVertex2f(x + 4, y + TILE_SIZE - 4);
        glEnd();

        if (type == 2) {
            glColor3f(0.9f, 0.3f, 0.0f);
            glBegin(GL_QUADS);
            glVertex2f(x, y + TILE_SIZE - 6); glVertex2f(x + TILE_SIZE, y + TILE_SIZE - 6);
            glVertex2f(x + TILE_SIZE, y + TILE_SIZE); glVertex2f(x, y + TILE_SIZE);
            glEnd();
        }
    }
    else if (type == 3) {
        glColor3f(1.0f, 0.4f, 0.0f);
        glBegin(GL_QUADS);
        glVertex2f(x, y); glVertex2f(x + TILE_SIZE, y);
        glVertex2f(x + TILE_SIZE, y + TILE_SIZE); glVertex2f(x, y + TILE_SIZE);
        glEnd();

        glColor3f(1.0f, 0.8f, 0.0f);
        glBegin(GL_QUADS);
        glVertex2f(x, y + TILE_SIZE - 8); glVertex2f(x + TILE_SIZE, y + TILE_SIZE - 8);
        glVertex2f(x + TILE_SIZE, y + TILE_SIZE); glVertex2f(x, y + TILE_SIZE);
        glEnd();
    }
    else if (type == 4) {
        glColor3f(0.5f, 0.1f, 0.0f);
        glBegin(GL_QUADS);
        glVertex2f(x, y); glVertex2f(x + TILE_SIZE, y);
        glVertex2f(x + TILE_SIZE, y + TILE_SIZE); glVertex2f(x, y + TILE_SIZE);
        glEnd();
    }
    else if (type == 5) {
        glColor3f(0.08f, 0.02f, 0.02f);
        glBegin(GL_QUADS);
        glVertex2f(x, y); glVertex2f(x + TILE_SIZE, y);
        glVertex2f(x + TILE_SIZE, y + TILE_SIZE); glVertex2f(x, y + TILE_SIZE);
        glEnd();
    }

    // --- NEW ICE THEME TILES ---
    else if (type == 6 || type == 7) {
        glColor3f(0.2f, 0.5f, 0.7f);
        glBegin(GL_QUADS);
        glVertex2f(x, y); glVertex2f(x + TILE_SIZE, y);
        glVertex2f(x + TILE_SIZE, y + TILE_SIZE); glVertex2f(x, y + TILE_SIZE);
        glEnd();

        glColor3f(0.4f, 0.8f, 0.9f);
        glBegin(GL_QUADS);
        glVertex2f(x + 4, y + 4); glVertex2f(x + TILE_SIZE - 4, y + 4);
        glVertex2f(x + TILE_SIZE - 4, y + TILE_SIZE - 4); glVertex2f(x + 4, y + TILE_SIZE - 4);
        glEnd();

        if (type == 7) {
            glColor3f(1.0f, 1.0f, 1.0f);
            glBegin(GL_QUADS);
            glVertex2f(x, y + TILE_SIZE - 6); glVertex2f(x + TILE_SIZE, y + TILE_SIZE - 6);
            glVertex2f(x + TILE_SIZE, y + TILE_SIZE); glVertex2f(x, y + TILE_SIZE);
            glEnd();
        }
    }
    else if (type == 8) {
        glColor3f(0.1f, 0.4f, 0.8f);
        glBegin(GL_QUADS);
        glVertex2f(x, y); glVertex2f(x + TILE_SIZE, y);
        glVertex2f(x + TILE_SIZE, y + TILE_SIZE); glVertex2f(x, y + TILE_SIZE);
        glEnd();

        glColor3f(0.6f, 0.9f, 1.0f);
        glBegin(GL_QUADS);
        glVertex2f(x, y + TILE_SIZE - 8); glVertex2f(x + TILE_SIZE, y + TILE_SIZE - 8);
        glVertex2f(x + TILE_SIZE, y + TILE_SIZE); glVertex2f(x, y + TILE_SIZE);
        glEnd();
    }
    else if (type == 9) {
        glColor3f(0.05f, 0.2f, 0.5f);
        glBegin(GL_QUADS);
        glVertex2f(x, y); glVertex2f(x + TILE_SIZE, y);
        glVertex2f(x + TILE_SIZE, y + TILE_SIZE); glVertex2f(x, y + TILE_SIZE);
        glEnd();
    }
    else if (type == 10) {
        glColor3f(0.02f, 0.05f, 0.15f);
        glBegin(GL_QUADS);
        glVertex2f(x, y); glVertex2f(x + TILE_SIZE, y);
        glVertex2f(x + TILE_SIZE, y + TILE_SIZE); glVertex2f(x, y + TILE_SIZE);
        glEnd();
    }
    //moon tile theme
   else if(type == 11)
    {
        glColor3f(0.18f,0.18f,0.25f);

        glBegin(GL_QUADS);
        glVertex2f(x,y);
        glVertex2f(x+TILE_SIZE,y);
        glVertex2f(x+TILE_SIZE,y+TILE_SIZE);
        glVertex2f(x,y+TILE_SIZE);
        glEnd();

        glColor3f(0.40f,0.40f,0.50f);

        glBegin(GL_QUADS);
        glVertex2f(x+4,y+4);
        glVertex2f(x+TILE_SIZE-4,y+4);
        glVertex2f(x+TILE_SIZE-4,y+TILE_SIZE-4);
        glVertex2f(x+4,y+TILE_SIZE-4);
        glEnd();
    }
    else if(type == 12)
{
    // Moon Ground

    glColor3f(0.22f,0.22f,0.30f);

    glBegin(GL_QUADS);
    glVertex2f(x,y);
    glVertex2f(x+TILE_SIZE,y);
    glVertex2f(x+TILE_SIZE,y+TILE_SIZE);
    glVertex2f(x,y+TILE_SIZE);
    glEnd();

    // Bright Top Edge

    glColor3f(0.80f,0.85f,1.0f);

    glBegin(GL_QUADS);
    glVertex2f(x,y+TILE_SIZE-4);
    glVertex2f(x+TILE_SIZE,y+TILE_SIZE-4);
    glVertex2f(x+TILE_SIZE,y+TILE_SIZE);
    glVertex2f(x,y+TILE_SIZE);
    glEnd();

    // Inner Panel

    glColor3f(0.30f,0.30f,0.40f);

    glBegin(GL_QUADS);
    glVertex2f(x+3,y+3);
    glVertex2f(x+TILE_SIZE-3,y+3);
    glVertex2f(x+TILE_SIZE-3,y+TILE_SIZE-3);
    glVertex2f(x+3,y+TILE_SIZE-3);
    glEnd();
}
    else if (type == 13)
    {
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

void drawLevel()
{
    // Dynamic Background Gradient depending on which level is loaded

    glBegin(GL_QUADS);

    if (currentActiveLevel == 1)
    {
        // Red Lava Glow

        glColor3f(0.18f, 0.02f, 0.02f);
        glVertex2f(0, WINDOW_HEIGHT);
        glVertex2f(WINDOW_WIDTH, WINDOW_HEIGHT);

        glColor3f(0.02f, 0.01f, 0.01f);
        glVertex2f(WINDOW_WIDTH, 0);
        glVertex2f(0, 0);
    }
    else if (currentActiveLevel == 2)
    {
        // Frosty Blue Glow

        glColor3f(0.02f, 0.08f, 0.20f);
        glVertex2f(0, WINDOW_HEIGHT);
        glVertex2f(WINDOW_WIDTH, WINDOW_HEIGHT);

        glColor3f(0.01f, 0.02f, 0.05f);
        glVertex2f(WINDOW_WIDTH, 0);
        glVertex2f(0, 0);
    }
    else if (currentActiveLevel == 3)
    {

        // Deep Space Background

        glColor3f(0.02f, 0.02f, 0.15f);
        glVertex2f(0, WINDOW_HEIGHT);
        glVertex2f(WINDOW_WIDTH, WINDOW_HEIGHT);

        glColor3f(0.00f, 0.00f, 0.08f);
        glVertex2f(WINDOW_WIDTH, 0);
        glVertex2f(0, 0);
        glColor4f(0.30f, 0.10f, 0.50f, 0.20f);

        glBegin(GL_POLYGON);

        for(int i=0;i<360;i++)
        {
            float a = i * 3.14159f / 180.0f;

            glVertex2f(
                250 + cos(a) * 180,
                300 + sin(a) * 100
            );
        }


    }

    glEnd();

    // ===== MOON WORLD EXTRAS =====

    if(currentActiveLevel == 3)
    {
       drawMoonWorld();
    }

    // Draw Map

    for (int row = 0; row < ROWS; row++)
    {
        for (int col = 0; col < COLS; col++)
        {
            float px = col * TILE_SIZE;
            float py = WINDOW_HEIGHT - (row + 1) * TILE_SIZE;

            drawTile(px, py, levelMap[row][col]);
        }
    }
}
