#ifndef PLAYER_H
#define PLAYER_H

extern float playerX;
extern float playerY;

extern const float PLAYER_WIDTH;
extern const float PLAYER_HEIGHT;

void setSpawnPoint(float startX, float startY);
void respawnPlayer();
void updatePlayerPhysics();
void drawPlayer();

#endif
