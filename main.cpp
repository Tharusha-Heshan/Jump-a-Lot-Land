#include <GL/freeglut.h>
#include "Constants.h"
#include "Level.h"
#include "Player.h"
#include "Level3Logic.h"
#include <cstdio>
#include <cmath>

bool keyStates[256] = { false };
bool winMessageShown = false;

// Camera and Window Globals
float cameraX = 0.0f;
float cameraY = 0.0f;

void handleKeyDown(unsigned char key, int x, int y) {
    keyStates[key] = true;
    if (key == '1') loadLevel(1);
    if (key == '3') loadLevel(3);
}

void handleKeyUp(unsigned char key, int x, int y) {
    keyStates[key] = false;
}

void updateCamera() {
    // Keep player in center of screen
    float targetX = playerX - (WINDOW_WIDTH / 2.0f);
    float targetY = playerY - (WINDOW_HEIGHT / 2.0f);

    // Smoothing (Optional: remove the 0.1f factor if you want instant snap)
    cameraX += (targetX - cameraX) * 0.1f;
    cameraY += (targetY - cameraY) * 0.1f;

    // Clamp camera to level edges so you don't see outside the map
    if (cameraX < 0) cameraX = 0;
    if (cameraY < 0) cameraY = 0;
}

void updatePhysicsLoop(int value) {
    updatePlayerPhysics();
    updateCamera();

    if(currentActiveLevel == 3) {
        updateLevel3(0.016f);
        if(isLevel3Complete() && !winMessageShown) {
            printf("LEVEL COMPLETE!\n");
            winMessageShown = true;
        }
    }

    glutPostRedisplay();
    glutTimerFunc(16, updatePhysicsLoop, 0);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    // Call external render functions
    drawLevel();
    drawPlayer();

    glutSwapBuffers();
}

void init() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT);

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
