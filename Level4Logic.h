#ifndef LEVEL4LOGIC_H
#define LEVEL4LOGIC_H

void updateLevel4(float dt);

void drawSkyWorld();

bool checkLevel4PlatformCollision(float x,
                                  float y,
                                  float width,
                                  float height);

bool checkLevel4PortalCollision();

void resetLevel4State();

bool isLevel4Complete();

#endif
