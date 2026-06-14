#include <GL/freeglut.h>
#include "Constants.h"
#include "Level.h"
#include "Player.h"
#include "Level3Logic.h"
#include "Level4Logic.h"
#include "Level5.h"
#include "Level5Logic.h"
#include <cstdio>

bool keyStates[256] = { false };
bool winMessageShown = false;

void handleKeyDown(unsigned char key, int x, int y) {
    keyStates[key] = true;

    if (key == '1') {
        loadLevel(1);
        winMessageShown = false;
    }
    if (key == '2') {
        loadLevel(2);
        winMessageShown = false;
    }
    if (key == '3') {
        loadLevel(3);
        winMessageShown = false;
    }
    if (key == '4') {
        loadLevel(4);
        winMessageShown = false;
    }
    if (key == '5') {
        loadLevel(5);
        winMessageShown = false;
        currentActiveLevel = 5;

        // Copy the Level 5 layout into the main active map
        for (int r = 0; r < ROWS; r++) {
            for (int c = 0; c < COLS; c++) {
                levelMap[r][c] = level5Data[r][c];
            }
        }

        // Reset the unique Level 5 elements like the snakes and pendulums
        resetLevel5State();

        // Reset your player back to the starting position
        respawnPlayer();
        winMessageShown = false;
    }
}

void handleKeyUp(unsigned char key, int x, int y) {
    keyStates[key] = false;
}

void updatePhysicsLoop(int value) {
    // ONLY update player physics if no full-screen UI overlay is active
    if (!playerDiedUI && !levelCompleteUI) {
        updatePlayerPhysics();
    }

    // Always update level elements so timers and lava animations keep running
    updateLevelElements();

    // Level 3 Updates
    if(currentActiveLevel == 3)
    {
        updateLevel3(0.016f);
        if(isLevel3Complete() && !winMessageShown)
        {
            printf("LEVEL 3 COMPLETE!\n");
            winMessageShown = true;
        }
    }
    // Level 4 Updates
    else if(currentActiveLevel == 4)
    {
        updateLevel4(0.016f);
        if(isLevel4Complete() && !winMessageShown)
        {
            printf("SKY LEVEL COMPLETE!\n");
            winMessageShown = true;
        }
    }
    // Level 5 Updates (Cleaned up the duplicate block here)
    else if(currentActiveLevel == 5)
    {
        updateLevel5(0.016f);
        if(isLevel5Complete() && !winMessageShown)
        {
            printf("LEVEL 5 COMPLETE! You escaped the jungle!\n");
            winMessageShown = true;
        }
    }

    glutPostRedisplay();
    glutTimerFunc(16, updatePhysicsLoop, 0);
}

void drawString(float x, float y, void* font, const char* string) {
    glRasterPos2f(x, y); // Sets the starting position for the text
    while (*string) {
        glutBitmapCharacter(font, *string);
        string++;
    }
}

void drawUIGuide() {
    // 1. Draw a dark semi-transparent background bar at the bottom of the screen
    glColor4f(0.0f, 0.0f, 0.0f, 0.6f); // Black with 60% opacity
    glBegin(GL_QUADS);
        glVertex2f(0, 0);
        glVertex2f(WINDOW_WIDTH, 0);
        glVertex2f(WINDOW_WIDTH, 35); // Bar height of 35 pixels
        glVertex2f(0, 35);
    glEnd();

    // 2. Draw the text guide in bright white
    glColor3f(1.0f, 1.0f, 1.0f);

    // Position the text slightly padded inside the bar
    // Change this line in your drawUIGuide function:
    drawString(20.0f, 12.0f, GLUT_BITMAP_HELVETICA_10, "CONTROLS: WASD to Move   |   PRESS [1] - [5] to Load Levels");
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    // 1. Draw level-specific backgrounds FIRST
    if (currentActiveLevel == 5) {
        drawJungleBackground();
    }

    // 2. Draw standard map tiles
    drawLevel();

    // 3. Draw level-specific foreground animations LAST
    if (currentActiveLevel == 5) {
        drawJungleWorld();
    }

    drawPlayer();
    drawLevelUI();

    // 4. Draw the UI Guide last so it overlays on top of everything nicely
    drawUIGuide();

    glutSwapBuffers();
}

void init() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT);

    // Added blending setup here to make the transparency in drawUIGuide work smoothly!
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    loadLevel(1);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutCreateWindow("Ice and Fire Pixel Art Game Engine");
    init();

    glutDisplayFunc(display);
    glutKeyboardFunc(handleKeyDown);
    glutKeyboardUpFunc(handleKeyUp);
    glutTimerFunc(16, updatePhysicsLoop, 0);

    glutMainLoop();
    return 0;
}
