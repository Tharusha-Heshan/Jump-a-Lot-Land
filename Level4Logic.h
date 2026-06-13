#ifndef LEVEL4LOGIC_H
#define LEVEL4LOGIC_H

void resetLevel4State();
void updateLevel4(float dt);
bool checkLevel4PortalCollision();
bool isLevel4Complete();
bool checkLevel4PlatformCollision(float x, float y, float width, float height);
void drawSkyWorld();

#endif
