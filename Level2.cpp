#include <GL/freeglut.h>
#include <cmath>

const int WINDOW_WIDTH  = 800;
const int WINDOW_HEIGHT = 600;
const int TILE_SIZE     = 40;
const int ROWS          = 15;
const int COLS          = 20;

// ============================================================
//  TILE KEY
//   0 = air
//   1 = solid ice block (interior)
//   2 = solid ice block (snow top)
//   3 = spike (instant death)
//   4 = abyss (instant death, bottom row)
//   5 = ice pillar / wall
//   6 = thin floating slab (solid, can land on top)
//   7 = narrow gap wall  – solid to NORMAL size, passable when scaled down
//   9 = slanted gate – solid unless sheared or scaled down
//  10 = exit door
// ============================================================

// Gap walls (type 7) are 1 tile wide.
// Normal player  = 24 px wide → fills the gap → blocked
// Scaled player  = 12 px wide → fits through → passable
// The gap is created by placing type-7 tiles with exactly 1 tile of
// open air above them, so the player must shrink to squeeze under.

int levelMap[ROWS][COLS] = {
//  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}, // row 0
    {1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 1}, // row 1
    {1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 1}, // row 2
    {1, 2, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 1}, // row 3
    {1, 1, 3, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1}, // row 4
    {1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 0, 0, 1}, // row 5
    {1, 1, 0, 3, 1, 0, 0, 0, 0, 0, 0, 0, 6, 6, 0, 0, 0, 0, 0, 1}, // row 6 (moved upper tiles here, using 6)
    {1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 7, 0, 0, 1}, // row 7
    {1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 9, 9, 0, 0, 7, 7, 2, 2, 1}, // row 8 (reverted 9s here)
    {1, 1, 0, 7, 7, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 7, 7, 0, 0, 1}, // row 9
    {1, 1, 2, 2, 2, 2, 0, 0, 0, 0, 2, 0, 3, 3, 3, 7, 7, 0, 0, 1}, // row 10
    {1, 1, 1, 1, 1, 1, 0, 2, 0, 0, 1, 2, 2, 2, 2, 2, 2, 0, 0, 1}, // row 11
    {1, 1, 1, 4, 4, 1, 4, 4, 4, 4, 1, 4, 4, 4, 4, 1, 1, 4, 4, 1}, // row 12
    {1, 1, 1, 4, 4, 1, 4, 4, 4, 4, 1, 4, 4, 4, 4, 1, 1, 4, 4, 1}, // row 13
    {1, 1, 1, 4, 4, 1, 4, 4, 4, 4, 1, 4, 4, 4, 4, 1, 1, 4, 4, 1}, // row 14
};

// ============================================================
//  PLAYER STATE
// ============================================================
float playerX = 48.0f;
float playerY = 500.0f;

const float PLAYER_WIDTH  = 24.0f;
const float PLAYER_HEIGHT = 32.0f;

float playerVx = 0.0f;
float playerVy = 0.0f;
const float MOVE_SPEED   = 3.8f;
const float GRAVITY      = -0.38f;
const float JUMP_FORCE   = 8.5f;
const float ICE_FRICTION = 0.86f;

bool isGrounded  = false;
bool facingRight = true;
bool isMoving    = false;

int animFrame  = 0;
int frameTicks = 0;

// ============================================================
//  SCALING, ROTATION, SHEAR STATE
// ============================================================
float playerScale  = 1.0f;
float scaleTarget  = 1.0f;
bool  isScaledDown = false;
const float SCALE_SMALL  = 0.5f;
const float SCALE_NORMAL = 1.0f;
const float SCALE_LERP   = 0.14f;

bool isRotated = false;
bool isSheared = false;

bool keyStates[256] = { false };
bool sKeyWasDown    = false;
bool rKeyWasDown    = false;
bool eKeyWasDown    = false;

// ============================================================
//  COLLISION
// ============================================================
bool isSolidTile(int type, bool smallPlayer) {
    if (type == 1 || type == 2 || type == 5 || type == 6) return true;
    if (type == 7) return !smallPlayer;  // gap wall: solid unless tiny
    if (type == 9) return !(isSheared || smallPlayer); // slanted gap
    return false;
}

bool checkCollision(float x, float y) {
    float w = (isRotated ? PLAYER_HEIGHT : PLAYER_WIDTH) * playerScale;
    float h = (isRotated ? PLAYER_WIDTH : PLAYER_HEIGHT) * playerScale;
    bool small = (playerScale <= 0.55f);

    float shearOffset = isSheared ? h * 0.5f : 0.0f;
    float inset = 1.0f;

    float pts[][2] = {
        { x + inset,                        y         },
        { x + w - inset,                    y         },
        { x + inset + shearOffset * 0.5f,   y + h * 0.5f },
        { x + w - inset + shearOffset * 0.5f, y + h * 0.5f },
        { x + inset + shearOffset,          y + h - 1 },
        { x + w - inset + shearOffset,      y + h - 1 }
    };
    for (int i = 0; i < 6; i++) {
        int col = (int)(pts[i][0] / TILE_SIZE);
        int row = (int)((WINDOW_HEIGHT - pts[i][1]) / TILE_SIZE);
        if (row >= 0 && row < ROWS && col >= 0 && col < COLS) {
            if (isSolidTile(levelMap[row][col], small)) return true;
        }
    }
    return false;
}

bool checkHazard(float x, float y) {
    float w = (isRotated ? PLAYER_HEIGHT : PLAYER_WIDTH) * playerScale;
    float h = (isRotated ? PLAYER_WIDTH : PLAYER_HEIGHT) * playerScale;
    float shearOffset = isSheared ? h * 0.5f : 0.0f;

    // Four inner sample points
    float pts[4][2] = {
        { x + w * 0.5f + shearOffset * 0.1f, y + h * 0.1f },
        { x + w * 0.5f + shearOffset * 0.85f, y + h * 0.85f },
        { x + w * 0.2f + shearOffset * 0.5f, y + h * 0.5f },
        { x + w * 0.8f + shearOffset * 0.5f, y + h * 0.5f }
    };
    for (int i = 0; i < 4; i++) {
        int col = (int)(pts[i][0] / TILE_SIZE);
        int row = (int)((WINDOW_HEIGHT - pts[i][1]) / TILE_SIZE);
        if (row >= 0 && row < ROWS && col >= 0 && col < COLS) {
            int t = levelMap[row][col];
            if (t == 3 || t == 4) return true;
        }
    }
    return false;
}

void respawn() {
    playerX = 48.0f; playerY = 500.0f;
    playerVx = 0.0f; playerVy = 0.0f;
    playerScale = 1.0f; scaleTarget = 1.0f; isScaledDown = false;
    isRotated = false; isSheared = false;
}

// ============================================================
//  PHYSICS  (60 Hz)
// ============================================================
void updatePhysics(int) {
    // Scale toggle (one-shot)
    bool sNow = keyStates['s'] || keyStates['S'];
    if (sNow && !sKeyWasDown) {
        if (isScaledDown) {
            float oldScale = playerScale;
            playerScale = SCALE_NORMAL;
            if (!checkCollision(playerX, playerY)) {
                isScaledDown = false;
                scaleTarget = SCALE_NORMAL;
            }
            playerScale = oldScale;
        } else {
            isScaledDown = true;
            scaleTarget  = SCALE_SMALL;
        }
    }
    sKeyWasDown = sNow;

    // Rotation toggle (one-shot)
    bool rNow = keyStates['r'] || keyStates['R'];
    if (rNow && !rKeyWasDown) {
        isRotated = !isRotated;
        if (checkCollision(playerX, playerY)) isRotated = !isRotated; // revert if blocked
    }
    rKeyWasDown = rNow;

    // Shear toggle (one-shot)
    bool eNow = keyStates['e'] || keyStates['E'];
    if (eNow && !eKeyWasDown) {
        isSheared = !isSheared;
        if (checkCollision(playerX, playerY)) isSheared = !isSheared; // revert if blocked
    }
    eKeyWasDown = eNow;

    // Lerp scale
    playerScale += (scaleTarget - playerScale) * SCALE_LERP;
    if (fabsf(playerScale - scaleTarget) < 0.004f) playerScale = scaleTarget;

    float currentMaxSpeed = scaleTarget <= 0.5f ? 2.0f : MOVE_SPEED;
    float currentAccel    = scaleTarget <= 0.5f ? 0.3f : 0.9f;

    // Horizontal input
    if (keyStates['a'] || keyStates['A']) {
        playerVx -= currentAccel;
        if (playerVx < -currentMaxSpeed) playerVx = -currentMaxSpeed;
        facingRight = false; isMoving = true;
    } else if (keyStates['d'] || keyStates['D']) {
        playerVx += currentAccel;
        if (playerVx > currentMaxSpeed) playerVx = currentMaxSpeed;
        facingRight = true; isMoving = true;
    } else {
        playerVx *= ICE_FRICTION;
        if (fabsf(playerVx) < 0.08f) { playerVx = 0.0f; isMoving = false; }
        else isMoving = true;
    }

    // Jump
    if ((keyStates['w'] || keyStates['W']) && isGrounded) {
        if (scaleTarget <= 0.5f) {
            playerVy = 11.0f; // High jump for small
        } else {
            playerVy = JUMP_FORCE; // Normal jump
        }
        isGrounded = false;
    }

    // Horizontal move + collision
    playerX += playerVx;
    float pw = (isRotated ? PLAYER_HEIGHT : PLAYER_WIDTH) * playerScale;
    if (playerX < 0)                   { playerX = 0;                   playerVx = 0; }
    if (playerX + pw > WINDOW_WIDTH)   { playerX = WINDOW_WIDTH - pw;   playerVx = 0; }
    if (checkCollision(playerX, playerY)) {
        playerX -= playerVx;
        playerVx = 0.0f;
    }

    // Vertical move + collision
    playerVy += GRAVITY;
    playerY  += playerVy;
    if (checkCollision(playerX, playerY)) {
        if (playerVy < 0) isGrounded = true;
        playerY  -= playerVy;
        playerVy  = 0.0f;
    } else {
        isGrounded = false;
    }

    // Hazards / out of bounds
    if (checkHazard(playerX, playerY) || playerY < 0) respawn();

    // Exit check
    float ph = (isRotated ? PLAYER_WIDTH : PLAYER_HEIGHT) * playerScale;
    int cCol = (int)((playerX + pw*0.5f) / TILE_SIZE);
    int cRow = (int)((WINDOW_HEIGHT - (playerY + ph*0.5f)) / TILE_SIZE);
    if (cRow >= 0 && cRow < ROWS && cCol >= 0 && cCol < COLS) {
        if (levelMap[cRow][cCol] == 10) {
            respawn(); // Reset for now
        }
    }

    // Walk animation
    if (isMoving && isGrounded) {
        if (++frameTicks >= 8) { animFrame = (animFrame + 1) % 4; frameTicks = 0; }
    } else {
        animFrame = 0;
    }

    glutPostRedisplay();
    glutTimerFunc(16, updatePhysics, 0);
}

// ============================================================
//  INPUT
// ============================================================
void handleKeyDown(unsigned char k, int, int) { keyStates[k] = true;  }
void handleKeyUp  (unsigned char k, int, int) { keyStates[k] = false; }

// ============================================================
//  TILE DRAWING
// ============================================================
void drawTile(float x, float y, int type) {
    switch (type) {
    case 0: return;

    case 1: // solid ice (interior)
        glColor3f(0.18f, 0.32f, 0.52f);
        glBegin(GL_QUADS);
        glVertex2f(x,y); glVertex2f(x+TILE_SIZE,y);
        glVertex2f(x+TILE_SIZE,y+TILE_SIZE); glVertex2f(x,y+TILE_SIZE);
        glEnd();
        glColor3f(0.35f, 0.60f, 0.82f);
        glBegin(GL_QUADS);
        glVertex2f(x+3,y+3); glVertex2f(x+TILE_SIZE-3,y+3);
        glVertex2f(x+TILE_SIZE-3,y+TILE_SIZE-3); glVertex2f(x+3,y+TILE_SIZE-3);
        glEnd();
        break;

    case 2: // surface ice (snow stripe on top)
        glColor3f(0.18f, 0.32f, 0.52f);
        glBegin(GL_QUADS);
        glVertex2f(x,y); glVertex2f(x+TILE_SIZE,y);
        glVertex2f(x+TILE_SIZE,y+TILE_SIZE); glVertex2f(x,y+TILE_SIZE);
        glEnd();
        glColor3f(0.35f, 0.60f, 0.82f);
        glBegin(GL_QUADS);
        glVertex2f(x+3,y+3); glVertex2f(x+TILE_SIZE-3,y+3);
        glVertex2f(x+TILE_SIZE-3,y+TILE_SIZE-3); glVertex2f(x+3,y+TILE_SIZE-3);
        glEnd();
        glColor3f(0.88f, 0.96f, 1.0f); // snow top
        glBegin(GL_QUADS);
        glVertex2f(x,y+TILE_SIZE-7); glVertex2f(x+TILE_SIZE,y+TILE_SIZE-7);
        glVertex2f(x+TILE_SIZE,y+TILE_SIZE); glVertex2f(x,y+TILE_SIZE);
        glEnd();
        break;

    case 3: { // spike
        glColor3f(0.50f, 0.82f, 1.0f);
        glBegin(GL_TRIANGLES);
        // spike A
        glVertex2f(x+3,  y+2); glVertex2f(x+18, y+2); glVertex2f(x+10, y+TILE_SIZE-6);
        // spike B
        glVertex2f(x+22, y+2); glVertex2f(x+37, y+2); glVertex2f(x+30, y+TILE_SIZE-6);
        glEnd();
        // glint tips
        glColor3f(1.0f, 1.0f, 1.0f);
        glBegin(GL_TRIANGLES);
        glVertex2f(x+9,  y+TILE_SIZE-6); glVertex2f(x+11, y+TILE_SIZE-6); glVertex2f(x+10, y+TILE_SIZE-2);
        glVertex2f(x+29, y+TILE_SIZE-6); glVertex2f(x+31, y+TILE_SIZE-6); glVertex2f(x+30, y+TILE_SIZE-2);
        glEnd();
        break;
    }

    case 4: // abyss
        glColor3f(0.01f, 0.03f, 0.10f);
        glBegin(GL_QUADS);
        glVertex2f(x,y); glVertex2f(x+TILE_SIZE,y);
        glVertex2f(x+TILE_SIZE,y+TILE_SIZE); glVertex2f(x,y+TILE_SIZE);
        glEnd();
        break;

    case 5: // pillar
        glColor3f(0.14f, 0.26f, 0.46f);
        glBegin(GL_QUADS);
        glVertex2f(x,y); glVertex2f(x+TILE_SIZE,y);
        glVertex2f(x+TILE_SIZE,y+TILE_SIZE); glVertex2f(x,y+TILE_SIZE);
        glEnd();
        glColor3f(0.48f, 0.72f, 0.92f);
        glBegin(GL_QUADS);
        glVertex2f(x+17,y+2); glVertex2f(x+21,y+2);
        glVertex2f(x+21,y+TILE_SIZE-2); glVertex2f(x+17,y+TILE_SIZE-2);
        glEnd();
        break;

    case 6: { // thin floating platform with icicle drips
        // slab body
        glColor3f(0.38f, 0.66f, 0.88f);
        glBegin(GL_QUADS);
        glVertex2f(x,y+TILE_SIZE-12); glVertex2f(x+TILE_SIZE,y+TILE_SIZE-12);
        glVertex2f(x+TILE_SIZE,y+TILE_SIZE); glVertex2f(x,y+TILE_SIZE);
        glEnd();
        // gloss strip
        glColor3f(0.82f, 0.95f, 1.0f);
        glBegin(GL_QUADS);
        glVertex2f(x,y+TILE_SIZE-12); glVertex2f(x+TILE_SIZE,y+TILE_SIZE-12);
        glVertex2f(x+TILE_SIZE,y+TILE_SIZE-9); glVertex2f(x,y+TILE_SIZE-9);
        glEnd();
        // icicle drips
        glColor3f(0.50f, 0.78f, 0.94f);
        glBegin(GL_TRIANGLES);
        glVertex2f(x+5, y+TILE_SIZE-12); glVertex2f(x+11,y+TILE_SIZE-12); glVertex2f(x+8, y+TILE_SIZE-4);
        glVertex2f(x+18,y+TILE_SIZE-12); glVertex2f(x+24,y+TILE_SIZE-12); glVertex2f(x+21,y+TILE_SIZE-3);
        glVertex2f(x+29,y+TILE_SIZE-12); glVertex2f(x+35,y+TILE_SIZE-12); glVertex2f(x+32,y+TILE_SIZE-5);
        glEnd();
        break;
    }

    case 7: { // narrow gap wall – dark ice gate, shorter to signal "squeeze through"
        // Tall blocking wall from bottom to near-top of tile, leaving visible gap hint
        glColor3f(0.10f, 0.20f, 0.38f);
        glBegin(GL_QUADS);
        glVertex2f(x,y); glVertex2f(x+TILE_SIZE,y);
        glVertex2f(x+TILE_SIZE,y+TILE_SIZE); glVertex2f(x,y+TILE_SIZE);
        glEnd();
        // Crystal texture lines
        glColor3f(0.25f, 0.45f, 0.70f);
        glBegin(GL_LINES);
        glVertex2f(x+8, y+4);  glVertex2f(x+8, y+TILE_SIZE-4);
        glVertex2f(x+20,y+4);  glVertex2f(x+20,y+TILE_SIZE-4);
        glVertex2f(x+32,y+4);  glVertex2f(x+32,y+TILE_SIZE-4);
        glEnd();
        // Bright warning edge
        glColor3f(0.40f, 0.65f, 0.90f);
        glBegin(GL_LINES);
        glVertex2f(x,y+TILE_SIZE); glVertex2f(x+TILE_SIZE,y+TILE_SIZE);
        glEnd();
        break;
    }

    case 9: { // slanted gate
        glColor3f(0.2f, 0.1f, 0.3f);
        glBegin(GL_QUADS);
        glVertex2f(x,y); glVertex2f(x+TILE_SIZE,y);
        glVertex2f(x+TILE_SIZE,y+TILE_SIZE); glVertex2f(x,y+TILE_SIZE);
        glEnd();
        glColor3f(0.6f, 0.3f, 0.8f);
        glBegin(GL_LINES);
        glVertex2f(x+TILE_SIZE*0.2f, y); glVertex2f(x+TILE_SIZE*0.7f, y+TILE_SIZE);
        glVertex2f(x+TILE_SIZE*0.5f, y); glVertex2f(x+TILE_SIZE*1.0f, y+TILE_SIZE);
        glEnd();
        break;
    }

    case 10: { // exit door
        glColor3f(0.2f, 0.8f, 0.4f);
        glBegin(GL_QUADS);
        glVertex2f(x,y); glVertex2f(x+TILE_SIZE,y);
        glVertex2f(x+TILE_SIZE,y+TILE_SIZE); glVertex2f(x,y+TILE_SIZE);
        glEnd();
        glColor3f(0.5f, 1.0f, 0.6f);
        glBegin(GL_QUADS);
        glVertex2f(x+5,y+5); glVertex2f(x+TILE_SIZE-5,y+5);
        glVertex2f(x+TILE_SIZE-5,y+TILE_SIZE-5); glVertex2f(x+5,y+TILE_SIZE-5);
        glEnd();
        break;
    }
    }
}

// ============================================================
//  BACKGROUND
// ============================================================
float bgTime = 0.0f;

void drawBackground() {
    bgTime += 0.010f;

    // Gradient sky
    glBegin(GL_QUADS);
    glColor3f(0.02f, 0.04f, 0.12f);
    glVertex2f(0, WINDOW_HEIGHT); glVertex2f(WINDOW_WIDTH, WINDOW_HEIGHT);
    glColor3f(0.05f, 0.11f, 0.22f);
    glVertex2f(WINDOW_WIDTH, 0); glVertex2f(0, 0);
    glEnd();

    // Subtle aurora bands (blended)
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    struct { float cy, r, g, b; } bands[] = {
        {470, 0.0f, 0.65f, 0.85f},
        {370, 0.15f, 0.35f, 0.75f},
        {280, 0.0f, 0.45f, 0.65f},
    };
    for (auto& b : bands) {
        float cy = b.cy + 16.0f * sinf(bgTime * 0.6f + b.cy * 0.012f);
        float a  = 0.06f + 0.03f * sinf(bgTime + b.cy * 0.02f);
        glColor4f(b.r, b.g, b.b, a);
        glBegin(GL_QUADS);
        glVertex2f(0, cy-28); glVertex2f(WINDOW_WIDTH, cy-28);
        glVertex2f(WINDOW_WIDTH, cy+28); glVertex2f(0, cy+28);
        glEnd();
    }
    glDisable(GL_BLEND);

    // Background ice spire silhouettes
    glColor3f(0.07f, 0.13f, 0.24f);
    float spires[][3] = {
        {15,28,75},{60,18,50},{100,32,95},{170,22,65},
        {240,28,85},{315,18,55},{385,38,115},{460,22,70},
        {530,28,90},{600,18,60},{660,32,80},{725,26,68},{768,20,48}
    };
    for (auto& s : spires) {
        glBegin(GL_TRIANGLES);
        glVertex2f(s[0],       80);
        glVertex2f(s[0]+s[1],  80);
        glVertex2f(s[0]+s[1]*0.5f, 80+s[2]);
        glEnd();
    }
}

// ============================================================
//  PLAYER DRAWING  – uses glPushMatrix / glScalef
// ============================================================
void drawPlayer(float x, float y) {
    float sc = playerScale;
    float w = (isRotated ? PLAYER_HEIGHT : PLAYER_WIDTH) * sc;
    float h = (isRotated ? PLAYER_WIDTH : PLAYER_HEIGHT) * sc;

    float bobY = (isMoving && isGrounded && (animFrame==1||animFrame==3)) ? 3.0f*sc : 0.0f;

    // Reset modelview so the matrix is clean before push
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glPushMatrix();

    // Center pivot
    glTranslatef(x + w*0.5f, y + h*0.5f + bobY, 0.0f);
    glScalef(sc, sc, 1.0f);

    if (isSheared) {
        GLfloat shearMatrix[16] = {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.5f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        };
        glMultMatrixf(shearMatrix);
    }

    if (isRotated) {
        glRotatef(-90.0f, 0.0f, 0.0f, 1.0f);
    }

    float hw = PLAYER_WIDTH  * 0.5f;
    float hh = PLAYER_HEIGHT * 0.5f;

    // Body
    glColor3f(0.0f, 0.85f, 1.0f);
    glBegin(GL_QUADS);
    glVertex2f(-hw,-hh); glVertex2f(hw,-hh);
    glVertex2f( hw, hh); glVertex2f(-hw, hh);
    glEnd();

    // Visor
    glColor3f(1.0f, 0.9f, 0.0f);
    glBegin(GL_QUADS);
    if (facingRight) {
        glVertex2f(hw-12,-hh+16); glVertex2f(hw-2,-hh+16);
        glVertex2f(hw-2, -hh+26); glVertex2f(hw-12,-hh+26);
    } else {
        glVertex2f(-hw+2, -hh+16); glVertex2f(-hw+12,-hh+16);
        glVertex2f(-hw+12,-hh+26); glVertex2f(-hw+2, -hh+26);
    }
    glEnd();

    // Boots
    glColor3f(0.0f, 0.2f, 0.6f);
    glBegin(GL_QUADS);
    float lx = (isMoving && animFrame==1) ? 4.0f : 0.0f;
    float rx = (isMoving && animFrame==3) ? 4.0f : 0.0f;
    glVertex2f(-hw+2,-hh);          glVertex2f(-hw+10,-hh);
    glVertex2f(-hw+10,-hh+6+lx);   glVertex2f(-hw+2, -hh+6+lx);
    glVertex2f( hw-10,-hh);         glVertex2f( hw-2, -hh);
    glVertex2f( hw-2, -hh+6+rx);   glVertex2f( hw-10,-hh+6+rx);
    glEnd();

    glPopMatrix();

    // Restore modelview identity after pop
    glLoadIdentity();
}

// ============================================================
//  HUD  – title only, no scale bar, no MINI label
// ============================================================
void drawHUD() {
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glColor3f(0.65f, 0.92f, 1.0f);
    glRasterPos2f(10, WINDOW_HEIGHT - 20);
    const char* title = "LEVEL 3 - KAIZO CAVERN  |  S: Shrink  R: Rotate  E: Shear";
    for (const char* c = title; *c; c++)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
}

// ============================================================
//  DISPLAY
// ============================================================
void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    drawBackground();

    for (int row = 0; row < ROWS; row++)
        for (int col = 0; col < COLS; col++) {
            float px = col * TILE_SIZE;
            float py = WINDOW_HEIGHT - (row+1) * TILE_SIZE;
            drawTile(px, py, levelMap[row][col]);
        }

    drawPlayer(playerX, playerY);
    drawHUD();

    glutSwapBuffers();
}

// ============================================================
//  INIT
// ============================================================
void init() {
    glClearColor(0,0,0,1);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutCreateWindow("Level 2 - Ice Cavern");
    init();
    glutDisplayFunc(display);
    glutKeyboardFunc(handleKeyDown);
    glutKeyboardUpFunc(handleKeyUp);
    glutTimerFunc(16, updatePhysics, 0);
    glutMainLoop();
    return 0;
}
