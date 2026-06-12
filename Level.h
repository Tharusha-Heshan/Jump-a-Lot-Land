#ifndef LEVEL_H
#define LEVEL_H

#include "Constants.h"

extern int levelMap[ROWS][COLS];
extern int currentActiveLevel;

extern float platform1X;
extern float platform1Y;

extern float platform2X;
extern float platform2Y;

extern float platform3X;
extern float platform3Y;

extern float platform1Speed;
extern float platform2Speed;
extern float platform3Speed;


void loadLevel(int levelID);
void drawTile(float x, float y, int type);
void drawLevel();
void updateMovingPlatform();
void updateMeteor();
bool checkMeteorCollision();
bool checkCollision(float x, float y, float width, float height);
bool checkLavaCollision(float x, float y, float width, float height);

#endif
