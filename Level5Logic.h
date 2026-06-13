#ifndef LEVEL5LOGIC_H
#define LEVEL5LOGIC_H

void updateLevel5(float dt);

void drawJungleBackground();

void drawJungleWorld();

bool checkPlatform5Collision(float x,
                              float y,
                              float width,
                              float height);

bool checkPortal5Collision();

void resetLevel5State();

bool isLevel5Complete();

#endif
