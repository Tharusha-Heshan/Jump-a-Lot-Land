#include <GL/freeglut.h>
#include "Constants.h"
#include "Level.h"
#include "Player.h"
#include "Level3Logic.h"
#include <cstdio>

bool keyStates[256] = { false };
bool winMessageShown = false;

void handleKeyDown(unsigned char key, int x, int y) {
    keyStates[key] = true;

    // Press 1 or 2 to swap worlds instantly
    if (key == '1') {
        loadLevel(1);
    }
    if (key == '2') {
        loadLevel(2);
    }
    if (key == '3') {
        loadLevel(3);
    }
}

void handleKeyUp(unsigned char key, int x, int y) {
    keyStates[key] = false;
}

void updatePhysicsLoop(int value) {
    updatePlayerPhysics();
    if(currentActiveLevel == 3)
    {
        updateLevel3(0.016f);
        if(isLevel3Complete() && !winMessageShown)
        {
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
    drawLevel(); // This now draws the dynamic background AND the tiles
    drawPlayer();

    glutSwapBuffers();
}

void init() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT);

    loadLevel(1); // Default to the Lava Level on boot
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
