#include "Player.h"
#include "Level3Logic.h"
#include "Constants.h"
#include <GL/freeglut.h>
#include <cmath>
#include <cstdlib>

// ======================
// Platforms
// ======================

float platform1X = 180;
float platform1Y = 170;
float platform1Speed = 1.0f;

float platform2X = 350;
float platform2Y = 260;
float platform2Speed = -1.2f;

float platform3X = 520;
float platform3Y = 330;
float platform3Speed = 1.5f;

// ======================
// Portal
// ======================

float portalX = 700.0f;
float portalY = 220.0f;
float portalBaseY = 220.0f;
float portalTime = 0.0f;

bool levelComplete = false;

// ======================
// Meteors
// ======================

struct Meteor
{
    float x;
    float y;
    float speed;
    bool active;
};
struct Star
{
    float x;
    float y;
};

const int STAR_COUNT = 100;

Star stars[STAR_COUNT];

const int METEOR_COUNT = 5;
Meteor meteors[METEOR_COUNT];

// ======================
// Init
// ======================

void resetLevel3State()
{
    levelComplete = false;

    // Meteors
    for(int i=0;i<METEOR_COUNT;i++)
    {
        meteors[i].active = true;
        meteors[i].x = rand() % WINDOW_WIDTH;
        meteors[i].y = WINDOW_HEIGHT + rand() % 1200;
        meteors[i].speed = 1 + rand() % 2;
    }

    // Stars
    for(int i=0;i<STAR_COUNT;i++)
    {
        stars[i].x = rand() % WINDOW_WIDTH;
        stars[i].y = rand() % WINDOW_HEIGHT;
    }
}

// ======================
// Update
// ======================

void updateLevel3(float dt)
{
    platform1X += platform1Speed;

    if(platform1X > 300)
        platform1Speed = -1;

    if(platform1X < 100)
        platform1Speed = 1;

    platform2X += platform2Speed;

    if(platform2X > 500)
        platform2Speed = -1.2f;

    if(platform2X < 200)
        platform2Speed = 1.2f;

    platform3X += platform3Speed;

    if(platform3X > 650)
        platform3Speed = -1.5f;

    if(platform3X < 400)
        platform3Speed = 1.5f;

    portalTime += dt;
    portalY = portalBaseY +
              sinf(portalTime * 2.0f) * 40.0f;
    for(int i=0;i<METEOR_COUNT;i++)
    {
        meteors[i].y -= meteors[i].speed;
        meteors[i].x += sin(glutGet(GLUT_ELAPSED_TIME) * 0.001f + i) * 0.3f;
        if(meteors[i].y < -20)
        {
            meteors[i].y = WINDOW_HEIGHT + rand()%300;
            meteors[i].x = rand()%WINDOW_WIDTH;
        }

        if(meteors[i].x < playerX + PLAYER_WIDTH &&
           meteors[i].x + 20 > playerX &&
           meteors[i].y < playerY + PLAYER_HEIGHT &&
           meteors[i].y + 20 > playerY)
        {
            respawnPlayer();
        }
    }

    if(checkPortalCollision() && !levelComplete)
    {
        levelComplete = true;
    }
}

// ======================
// Portal
// ======================

bool checkPortalCollision()
{
    if(playerX + PLAYER_WIDTH > portalX &&
       playerX < portalX + 40 &&
       playerY + PLAYER_HEIGHT > portalY &&
       playerY < portalY + 80)
    {
        return true;
    }

    return false;
}

bool isLevel3Complete()
{
    return levelComplete;
}

// ======================
// Platforms
// ======================

bool checkPlatformCollision(
    float x,
    float y,
    float width,
    float height)
{
    if(x + width > platform1X &&
       x < platform1X + TILE_SIZE * 2 &&
       y >= platform1Y + TILE_SIZE - 5 &&
       y <= platform1Y + TILE_SIZE + 10)
        return true;

    if(x + width > platform2X &&
       x < platform2X + TILE_SIZE * 2 &&
       y >= platform2Y + TILE_SIZE - 5 &&
       y <= platform2Y + TILE_SIZE + 10)
        return true;

    if(x + width > platform3X &&
       x < platform3X + TILE_SIZE * 2 &&
       y >= platform3Y + TILE_SIZE - 5 &&
       y <= platform3Y + TILE_SIZE + 10)
        return true;

    return false;
}

// ======================
// Draw
// ======================

void drawMoonWorld()
{
    // Stars

    glColor3f(1.0f,1.0f,1.0f);

for(int i=0;i<STAR_COUNT;i++)
{
    if(i % 10 == 0)
        glPointSize(4);
    else
        glPointSize(2);

    glBegin(GL_POINTS);

    glVertex2f(
        stars[i].x,
        stars[i].y
    );

    glEnd();
}

            // Moon
        // ======================
        // Moon Glow
        // ======================

        glColor4f(0.8f,0.8f,0.9f,0.15f);

        glBegin(GL_POLYGON);

        for(int i=0;i<360;i++)
        {
            float a = i * 3.14159f / 180.0f;

            glVertex2f(
                520 + cos(a)*110,
                320 + sin(a)*110
            );
        }

        glEnd();

        // ======================
        // Moon Body
        // ======================

        glColor3f(0.92f,0.92f,0.85f);

        glBegin(GL_POLYGON);

        for(int i=0;i<360;i++)
        {
            float a = i * 3.14159f / 180.0f;

            glVertex2f(
                520 + cos(a)*80,
                320 + sin(a)*80
            );
        }

        glEnd();

        // ======================
        // Crater 1
        // ======================

        glColor3f(0.75f,0.75f,0.75f);

        glBegin(GL_POLYGON);

        for(int i=0;i<360;i++)
        {
            float a = i * 3.14159f / 180.0f;

            glVertex2f(
                500 + cos(a)*12,
                340 + sin(a)*12
            );
        }

        glEnd();

        // ======================
        // Crater 2
        // ======================

        glBegin(GL_POLYGON);

        for(int i=0;i<360;i++)
        {
            float a = i * 3.14159f / 180.0f;

            glVertex2f(
                545 + cos(a)*8,
                300 + sin(a)*8
            );
        }

        glEnd();

        // ======================
        // Crater 3
        // ======================

        glBegin(GL_POLYGON);

        for(int i=0;i<360;i++)
        {
            float a = i * 3.14159f / 180.0f;

            glVertex2f(
                540 + cos(a)*6,
                355 + sin(a)*6
            );
        }

        glEnd();
        // ======================
        // PLATFORM 1
        // ======================

        // Outer Metal Frame
        glColor3f(0.35f,0.35f,0.45f);

        glBegin(GL_QUADS);
        glVertex2f(platform1X,platform1Y);
        glVertex2f(platform1X+64,platform1Y);
        glVertex2f(platform1X+64,platform1Y+32);
        glVertex2f(platform1X,platform1Y+32);
        glEnd();

        // Inner Panel
        glColor3f(0.75f,0.75f,0.85f);

        glBegin(GL_QUADS);
        glVertex2f(platform1X+3,platform1Y+3);
        glVertex2f(platform1X+61,platform1Y+3);
        glVertex2f(platform1X+61,platform1Y+29);
        glVertex2f(platform1X+3,platform1Y+29);
        glEnd();

        // Blue Energy Lights
        glColor3f(0.0f,0.8f,1.0f);

        glBegin(GL_QUADS);
        glVertex2f(platform1X+5,platform1Y+5);
        glVertex2f(platform1X+10,platform1Y+5);
        glVertex2f(platform1X+10,platform1Y+10);
        glVertex2f(platform1X+5,platform1Y+10);

        glVertex2f(platform1X+54,platform1Y+5);
        glVertex2f(platform1X+59,platform1Y+5);
        glVertex2f(platform1X+59,platform1Y+10);
        glVertex2f(platform1X+54,platform1Y+10);
        glEnd();


        // ======================
        // PLATFORM 2
        // ======================

        glColor3f(0.35f,0.35f,0.45f);

        glBegin(GL_QUADS);
        glVertex2f(platform2X,platform2Y);
        glVertex2f(platform2X+64,platform2Y);
        glVertex2f(platform2X+64,platform2Y+32);
        glVertex2f(platform2X,platform2Y+32);
        glEnd();

        glColor3f(0.75f,0.75f,0.85f);

        glBegin(GL_QUADS);
        glVertex2f(platform2X+3,platform2Y+3);
        glVertex2f(platform2X+61,platform2Y+3);
        glVertex2f(platform2X+61,platform2Y+29);
        glVertex2f(platform2X+3,platform2Y+29);
        glEnd();


        // ======================
        // PLATFORM 3
        // ======================

        glColor3f(0.35f,0.35f,0.45f);

        glBegin(GL_QUADS);
        glVertex2f(platform3X,platform3Y);
        glVertex2f(platform3X+64,platform3Y);
        glVertex2f(platform3X+64,platform3Y+32);
        glVertex2f(platform3X,platform3Y+32);
        glEnd();

        glColor3f(0.75f,0.75f,0.85f);

        glBegin(GL_QUADS);
        glVertex2f(platform3X+3,platform3Y+3);
        glVertex2f(platform3X+61,platform3Y+3);
        glVertex2f(platform3X+61,platform3Y+29);
        glVertex2f(platform3X+3,platform3Y+29);
    // ======================
// Meteors
    // ======================

    for(int i=0;i<METEOR_COUNT;i++)
    {
        float mx = meteors[i].x;
        float my = meteors[i].y;

        // Fire Trail

        glColor3f(1.0f,0.5f,0.0f);

        glBegin(GL_TRIANGLES);

        glVertex2f(mx + 10, my + 10);
        glVertex2f(mx + 30, my + 5);
        glVertex2f(mx + 30, my + 15);

        glEnd();

        // Outer Rock

        glColor3f(0.45f,0.25f,0.15f);

        glBegin(GL_POLYGON);

        for(int j=0;j<12;j++)
        {
            float a = j * 6.28318f / 12.0f;

            glVertex2f(
                mx + 10 + cos(a)*10,
                my + 10 + sin(a)*10
            );
        }

        glEnd();

        // Inner Highlight

        glColor3f(0.75f,0.45f,0.20f);

        glBegin(GL_POLYGON);

        for(int j=0;j<12;j++)
        {
            float a = j * 6.28318f / 12.0f;

            glVertex2f(
                mx + 8 + cos(a)*5,
                my + 12 + sin(a)*5
            );
        }

        glEnd();
    }
    // ======================
// PORTAL GLOW
// ======================

    glColor4f(0.5f,0.0f,1.0f,0.3f);

    glBegin(GL_POLYGON);

    for(int i=0;i<40;i++)
    {
        float a = i * 6.28318f / 40.0f;

        glVertex2f(
            portalX + 20 + cos(a)*40,
            portalY + 40 + sin(a)*60
        );
    }

    glEnd();


    // ======================
    // PORTAL FRAME
    // ======================

    glColor3f(0.2f,0.2f,0.3f);

    glBegin(GL_QUADS);

    glVertex2f(portalX-5,portalY);
    glVertex2f(portalX+45,portalY);
    glVertex2f(portalX+45,portalY+80);
    glVertex2f(portalX-5,portalY+80);

    glEnd();


    // ======================
    // PORTAL CORE
    // ======================

    glColor3f(0.7f,0.0f,1.0f);

    glBegin(GL_POLYGON);

    for(int i=0;i<40;i++)
    {
        float a = i * 6.28318f / 40.0f;

        glVertex2f(
            portalX + 20 + cos(a)*18,
            portalY + 40 + sin(a)*35
        );
    }

    glEnd();


    // ======================
    // INNER ENERGY
    // ======================

    glColor3f(1.0f,0.6f,1.0f);

    glBegin(GL_POLYGON);

    for(int i=0;i<40;i++)
    {
        float a = i * 6.28318f / 40.0f;

        glVertex2f(
            portalX + 20 + cos(a)*10,
            portalY + 40 + sin(a)*20
        );
    }

    glEnd();
    }
