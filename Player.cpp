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

       if(currentActiveLevel == 3)
        playerVy = 8.5f;      // Higher moon jump
    else
        playerVy = JUMP_FORCE;
        isGrounded = false;
    }

    playerX += playerVx;
    if (checkCollision(playerX, playerY, PLAYER_WIDTH, PLAYER_HEIGHT)) {
        playerX -= playerVx;
    }

   float gravity = GRAVITY;

        if(currentActiveLevel == 3)
        {
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

    if (checkLavaCollision(playerX, playerY, PLAYER_WIDTH, PLAYER_HEIGHT) || playerY < 0) {
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
    if(currentActiveLevel == 3)
    {
    //add later
    }
}

void drawPlayer() {
    float bobOffsetY = 0.0f;
    if (isMoving && isGrounded && (animFrame == 1 || animFrame == 3)) {
        bobOffsetY = 3.0f;
    }

    glColor3f(0.0f, 0.85f, 1.0f);
    glBegin(GL_QUADS);
    glVertex2f(playerX, playerY + bobOffsetY);
    glVertex2f(playerX + PLAYER_WIDTH, playerY + bobOffsetY);
    glVertex2f(playerX + PLAYER_WIDTH, playerY + PLAYER_HEIGHT + bobOffsetY);
    glVertex2f(playerX, playerY + PLAYER_HEIGHT + bobOffsetY);
    glEnd();

    glColor3f(1.0f, 0.9f, 0.0f);
    glBegin(GL_QUADS);
    if (facingRight) {
        glVertex2f(playerX + 12, playerY + 16 + bobOffsetY);
        glVertex2f(playerX + 22, playerY + 16 + bobOffsetY);
        glVertex2f(playerX + 22, playerY + 26 + bobOffsetY);
        glVertex2f(playerX + 12, playerY + 26 + bobOffsetY);
    } else {
        glVertex2f(playerX + 2, playerY + 16 + bobOffsetY);
        glVertex2f(playerX + 12, playerY + 16 + bobOffsetY);
        glVertex2f(playerX + 12, playerY + 26 + bobOffsetY);
        glVertex2f(playerX + 2, playerY + 26 + bobOffsetY);
    }
    glEnd();

    glColor3f(0.0f, 0.2f, 0.6f);
    glBegin(GL_QUADS);
    float leftLegExt = (isMoving && animFrame == 1) ? 4.0f : 0.0f;
    glVertex2f(playerX + 2, playerY);
    glVertex2f(playerX + 10, playerY);
    glVertex2f(playerX + 10, playerY + 6 + leftLegExt);
    glVertex2f(playerX + 2, playerY + 6 + leftLegExt);

    float rightLegExt = (isMoving && animFrame == 3) ? 4.0f : 0.0f;
    glVertex2f(playerX + 14, playerY);
    glVertex2f(playerX + 22, playerY);
    glVertex2f(playerX + 22, playerY + 6 + rightLegExt);
    glVertex2f(playerX + 14, playerY + 6 + rightLegExt);
    glEnd();
}
