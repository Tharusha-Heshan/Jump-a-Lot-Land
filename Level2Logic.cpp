#include "Level2Logic.h"
#include "Constants.h"
#include <GL/freeglut.h>
#include <cstdlib>
#include <cmath>

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

void drawLevel2Background() {
    float bgTime = glutGet(GLUT_ELAPSED_TIME) * 0.001f;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Green aurora band 1
    float cy1 = 470 + 16.0f * sinf(bgTime * 0.6f + 470 * 0.012f);
    float a1 = 0.06f + 0.03f * sinf(bgTime + 470 * 0.02f);
    glColor4f(0.15f, 0.75f, 0.35f, a1);
    glBegin(GL_QUADS);
    glVertex2f(0, cy1-28); glVertex2f(WINDOW_WIDTH, cy1-28);
    glVertex2f(WINDOW_WIDTH, cy1+28); glVertex2f(0, cy1+28);
    glEnd();

    // Green aurora band 2
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
}

void drawLevel2Particles() {
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

void drawLevel2Tile(float x, float y, int type) {
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
}
