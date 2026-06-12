#ifndef LEVEL3LOGIC_H
#define LEVEL3LOGIC_H

void updateLevel3(float dt);

void drawMoonWorld();

bool checkPlatformCollision(float x,
                            float y,
                            float width,
                            float height);

bool checkPortalCollision();

void resetLevel3State();

bool isLevel3Complete();

#endif
