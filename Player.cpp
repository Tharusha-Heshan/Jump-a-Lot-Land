#include "Player.h"
#include "Level.h"
#include "Constants.h"
#include <GL/freeglut.h>

float playerX = 50.0f;
float playerY = 250.0f;

// New variables to track where the player should restart!
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
float playerScale = 1.0f;
bool sKeyWasDown = false;

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
    if (currentActiveLevel == 2) {
        bool sDown = keyStates['s'] || keyStates['S'];
        if (sDown && !sKeyWasDown) {
            playerScale = (playerScale == 1.0f) ? 0.5f : 1.0f;
        }
        sKeyWasDown = sDown;
    } else {
        playerScale = 1.0f;
    }

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

       if(currentActiveLevel == 3)
        playerVy = 8.5f;      // Higher moon jump
    else
        playerVy = JUMP_FORCE;
        isGrounded = false;
    }

    float cw = PLAYER_WIDTH * playerScale;
    float ch = PLAYER_HEIGHT * playerScale;

    playerX += playerVx;
    if (checkCollision(playerX, playerY, cw, ch)) {
        playerX -= playerVx;
    }

   float gravity = GRAVITY;

        if(currentActiveLevel == 3)
        {
            gravity = -0.20f;    // Moon gravity
        }

        playerVy += gravity;


    playerY += playerVy;

    if (checkCollision(playerX, playerY, cw, ch)) {
        if (playerVy < 0) {
            isGrounded = true;
        }
        playerY -= playerVy;
        playerVy = 0.0f;
    } else {
        isGrounded = false;
    }

    if (checkLavaCollision(playerX, playerY, cw, ch) || playerY < 0) {
        respawnPlayer();
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

void drawPlayer() {
    float bobOffsetY = 0.0f;
    if (isMoving && isGrounded && (animFrame == 1 || animFrame == 3)) {
        bobOffsetY = 3.0f;
    }

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glPushMatrix();
    glTranslatef(playerX, playerY, 0.0f);
    glScalef(playerScale, playerScale, 1.0f);

    glColor3f(0.0f, 0.85f, 1.0f);
    glBegin(GL_QUADS);
    glVertex2f(0, bobOffsetY);
    glVertex2f(PLAYER_WIDTH, bobOffsetY);
    glVertex2f(PLAYER_WIDTH, PLAYER_HEIGHT + bobOffsetY);
    glVertex2f(0, PLAYER_HEIGHT + bobOffsetY);
    glEnd();

    glColor3f(1.0f, 0.9f, 0.0f);
    glBegin(GL_QUADS);
    if (facingRight) {
        glVertex2f(12, 16 + bobOffsetY);
        glVertex2f(22, 16 + bobOffsetY);
        glVertex2f(22, 26 + bobOffsetY);
        glVertex2f(12, 26 + bobOffsetY);
    } else {
        glVertex2f(2, 16 + bobOffsetY);
        glVertex2f(12, 16 + bobOffsetY);
        glVertex2f(12, 26 + bobOffsetY);
        glVertex2f(2, 26 + bobOffsetY);
    }
    glEnd();

    glColor3f(0.0f, 0.2f, 0.6f);
    glBegin(GL_QUADS);
    float leftLegExt = (isMoving && animFrame == 1) ? 4.0f : 0.0f;
    glVertex2f(2, 0);
    glVertex2f(10, 0);
    glVertex2f(10, 6 + leftLegExt);
    glVertex2f(2, 6 + leftLegExt);

    float rightLegExt = (isMoving && animFrame == 3) ? 4.0f : 0.0f;
    glVertex2f(14, 0);
    glVertex2f(22, 0);
    glVertex2f(22, 6 + rightLegExt);
    glVertex2f(14, 6 + rightLegExt);
    glEnd();

    glPopMatrix();
}
