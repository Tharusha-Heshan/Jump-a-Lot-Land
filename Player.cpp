#include "Player.h"
#include "Level.h"
#include "Constants.h"
#include <GL/freeglut.h>

float playerX = 50.0f;
float playerY = 250.0f;

// Variables to track where the player should restart
float spawnX = 50.0f;
float spawnY = 250.0f;

const float PLAYER_WIDTH = 24.0f;
const float PLAYER_HEIGHT = 32.0f;

float playerVx = 0.0f;
float playerVy = 0.0f;
const float MOVE_SPEED = 4.0f;
const float GRAVITY = -0.35f;
const float JUMP_FORCE = 7.5f;

bool isGrounded = false;
bool facingRight = true;
bool isMoving = false;

int animFrame = 0;
int frameTicks = 0;

// Update the spawn coordinates and instantly move the player there
void setSpawnPoint(float startX, float startY) {
    spawnX = startX;
    spawnY = startY;
    respawnPlayer();
}

// Reset character to the active spawn point
void respawnPlayer() {
    playerX = spawnX;
    playerY = spawnY;
    playerVx = 0.0f;
    playerVy = 0.0f;
}

void updatePlayerPhysics() {
    if (keyStates['a'] || keyStates['A']) {
        playerVx = -MOVE_SPEED;
        facingRight = false;
        isMoving = true;
    } else if (keyStates['d'] || keyStates['D']) {
        playerVx = MOVE_SPEED;
        facingRight = true;
        isMoving = true;
    } else {
        playerVx = 0.0f;
        isMoving = false;
    }

    if ((keyStates['w'] || keyStates['W']) && isGrounded) {
        if (currentActiveLevel == 3) {
            playerVy = 8.5f;      // Higher moon jump
        } else {
            playerVy = JUMP_FORCE;
        }
        isGrounded = false;
    }

    // Check current player center grid tile AND the tile right below their feet
    int checkCol = (int)((playerX + PLAYER_WIDTH / 2.0f) / TILE_SIZE);
    int checkRow = (int)((WINDOW_HEIGHT - (playerY + PLAYER_HEIGHT / 2.0f)) / TILE_SIZE);
    int rowBelow = checkRow + 1; // Grid cell directly below feet

    if (checkCol >= 0 && checkCol < COLS) {
        bool hittingFlagTile = false;

        if (checkRow >= 0 && checkRow < ROWS && levelMap[checkRow][checkCol] == 14) hittingFlagTile = true;
        if (rowBelow >= 0 && rowBelow < ROWS && levelMap[rowBelow][checkCol] == 14) hittingFlagTile = true;

        if (hittingFlagTile && !flagTriggered && !playerDiedUI) {
            flagTriggered = true;
            levelCompleteUI = true;
        }
    }

    playerX += playerVx;
    if (checkCollision(playerX, playerY, PLAYER_WIDTH, PLAYER_HEIGHT)) {
        playerX -= playerVx;
    }

    // Bounce pad collision
    if (isTouchingBouncePad(playerX, playerY, PLAYER_WIDTH, PLAYER_HEIGHT)) {
        playerVy = 10.0f;
        isGrounded = false;
    }

    float gravity = GRAVITY;
    if (currentActiveLevel == 3) {
        gravity = -0.20f;    // Moon gravity
    }

    playerVy += gravity;
    playerY += playerVy;

    if (checkCollision(playerX, playerY, PLAYER_WIDTH, PLAYER_HEIGHT)) {
        if (playerVy < 0) {
            isGrounded = true;
        }
        playerY -= playerVy;
        playerVy = 0.0f;
    } else {
        isGrounded = false;
    }

    // Hazard collision
    if (checkLavaCollision(playerX, playerY, PLAYER_WIDTH, PLAYER_HEIGHT) || playerY < 0) {
        if (!playerDiedUI && !levelCompleteUI) {
            playerDiedUI = true;
        }
    }

    if (isMoving && isGrounded) {
        frameTicks++;
        if (frameTicks >= 8) {
            animFrame = (animFrame + 1) % 4;
            frameTicks = 0;
        }
    } else {
        animFrame = 0;
    }
}

// Helper function to keep drawing code remarkably clean
void drawQuad(float x1, float y1, float x2, float y2) {
    glBegin(GL_QUADS);
    glVertex2f(x1, y1);
    glVertex2f(x2, y1);
    glVertex2f(x2, y2);
    glVertex2f(x1, y2);
    glEnd();
}

void drawPlayer() {
    float bobOffsetY = 0.0f;
    if (isMoving && isGrounded && (animFrame == 1 || animFrame == 3)) {
        bobOffsetY = 2.0f; // Slight bounce while running
    }

    glPushMatrix();

    // Move the matrix to the center-bottom of the player's bounding box
    glTranslatef(playerX + PLAYER_WIDTH / 2.0f, playerY + bobOffsetY, 0.0f);

    // If walking left, instantly mirror all drawing instructions symmetrically!
    if (!facingRight) {
        glScalef(-1.0f, 1.0f, 1.0f);
    }

    // Animation Angles based on state
    float rightLegAngle = 0.0f, leftLegAngle = 0.0f;
    float rightArmAngle = 0.0f, leftArmAngle = 0.0f;

    if (!isGrounded) {
        // JUMP POSE: One arm straight up, legs split
        rightArmAngle = 180.0f; // Fist up
        leftArmAngle = -30.0f;  // Arm tucked back
        rightLegAngle = 60.0f;  // Knee up
        leftLegAngle = -30.0f;  // Leg back
    } else if (isMoving) {
        // RUN POSE: Scissor kick the arms and legs based on animFrame
        if (animFrame == 1) {
            rightLegAngle = -45.0f; leftLegAngle = 45.0f;
            rightArmAngle = 45.0f;  leftArmAngle = -45.0f;
        } else if (animFrame == 3) {
            rightLegAngle = 45.0f;  leftLegAngle = -45.0f;
            rightArmAngle = -45.0f; leftArmAngle = 45.0f;
        }
    }

    // --- DRAWING ORDER: Back to Front (Z-Depth illusion) ---

    // 1. BACK ARM (Left Arm)
    glPushMatrix();
    glTranslatef(0.0f, 16.0f, 0.0f); // Anchor at shoulder
    glRotatef(leftArmAngle, 0.0f, 0.0f, 1.0f);
    glColor3f(0.15f, 0.55f, 0.25f); // Dark Green Sleeve
    drawQuad(-3.0f, -6.0f, 1.0f, 2.0f);
    glColor3f(0.40f, 0.25f, 0.15f); // Leather Bracer
    drawQuad(-4.0f, -10.0f, 2.0f, -6.0f);
    glPopMatrix();

    // 2. BACK LEG (Left Leg)
    glPushMatrix();
    glTranslatef(-3.0f, 10.0f, 0.0f); // Anchor at hip
    glRotatef(leftLegAngle, 0.0f, 0.0f, 1.0f);
    glColor3f(0.35f, 0.25f, 0.15f); // Brown Pant
    drawQuad(-3.0f, -6.0f, 3.0f, 2.0f);
    glColor3f(0.15f, 0.15f, 0.15f); // Dark Grey Boot
    drawQuad(-4.0f, -10.0f, 4.0f, -6.0f);
    glPopMatrix();

    // 3. MAIN BODY (Tunic and Belt)
    glColor3f(0.20f, 0.70f, 0.30f); // Bright Green Tunic
    drawQuad(-6.0f, 10.0f, 6.0f, 20.0f); // Main torso

    glColor3f(0.40f, 0.25f, 0.15f); // Leather Belt
    drawQuad(-6.0f, 12.0f, 6.0f, 15.0f);

    glColor3f(0.85f, 0.85f, 0.85f); // Silver Belt Buckle
    drawQuad(2.0f, 12.0f, 4.0f, 15.0f);

    // 4. HEAD (Face, Spiky Hair, Headband)
    glPushMatrix();
    glTranslatef(0.0f, 20.0f, 0.0f); // Anchor at neck

    glColor3f(1.0f, 0.85f, 0.70f); // Fair/Tan Skin
    drawQuad(-5.0f, 0.0f, 7.0f, 8.0f); // Face
    drawQuad(7.0f, 2.0f, 9.0f, 5.0f);  // Smaller Nose

    glColor3f(0.10f, 0.40f, 0.90f); // Bright Blue Eye
    drawQuad(4.0f, 3.0f, 6.0f, 6.0f);

    // Blonde Hair Base
    glColor3f(0.95f, 0.85f, 0.20f);
    drawQuad(-7.0f, -1.0f, -4.0f, 6.0f); // Back hair
    drawQuad(-5.0f, 8.0f, 8.0f, 11.0f);  // Top hair volume

    // Spiky Bangs
    glBegin(GL_TRIANGLES);
    glVertex2f(8.0f, 11.0f); glVertex2f(13.0f, 8.0f); glVertex2f(8.0f, 6.0f);
    glVertex2f(5.0f, 11.0f); glVertex2f(10.0f, 14.0f); glVertex2f(1.0f, 11.0f);
    glEnd();

    // Red Hero Headband
    glColor3f(0.85f, 0.15f, 0.20f);
    drawQuad(-6.0f, 6.0f, 8.0f, 8.0f); // Band around head

    // Headband Tails waving in the back
    glBegin(GL_TRIANGLES);
    glVertex2f(-6.0f, 8.0f); glVertex2f(-12.0f, 9.0f); glVertex2f(-6.0f, 6.0f);
    glVertex2f(-6.0f, 7.0f); glVertex2f(-14.0f, 4.0f); glVertex2f(-6.0f, 5.0f);
    glEnd();

    glPopMatrix();

    // 5. FRONT LEG (Right Leg)
    glPushMatrix();
    glTranslatef(3.0f, 10.0f, 0.0f); // Anchor at hip
    glRotatef(rightLegAngle, 0.0f, 0.0f, 1.0f);
    glColor3f(0.35f, 0.25f, 0.15f); // Brown Pant
    drawQuad(-3.0f, -6.0f, 3.0f, 2.0f);
    glColor3f(0.15f, 0.15f, 0.15f); // Dark Grey Boot
    drawQuad(-2.0f, -10.0f, 5.0f, -6.0f);
    glPopMatrix();

    // 6. FRONT ARM (Right Arm)
    glPushMatrix();
    glTranslatef(0.0f, 16.0f, 0.0f); // Anchor at shoulder
    glRotatef(rightArmAngle, 0.0f, 0.0f, 1.0f);
    glColor3f(0.20f, 0.70f, 0.30f); // Bright Green Sleeve
    drawQuad(-1.0f, -6.0f, 3.0f, 2.0f);
    glColor3f(0.40f, 0.25f, 0.15f); // Leather Bracer
    drawQuad(-2.0f, -10.0f, 4.0f, -6.0f);
    glPopMatrix();

    glPopMatrix(); // End overall player matrix
}
