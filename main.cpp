// ============================================================================
//  LEVEL 2 - CRYSTAL CAVERN  (Reborn Edition)
//  A modern, juicy 2D platformer.  GLUT + OpenGL, single file.
//
//  Controls:
//     A / D  or  <- / ->   : Move
//     W / SPACE / UP        : Jump  (press again in the air = DOUBLE JUMP)
//     J / L                 : DASH  (air dash through gaps)
//     S / DOWN              : SHRINK toggle (become small to fit tight tunnels)
//     R                     : Restart from last checkpoint
//     ENTER                 : Play again (after finishing)
//
//  Build (msys2 mingw64):
//     g++ level2.cpp -o level2.exe -O2 -lfreeglut -lopengl32 -lglu32 -lwinmm -lgdi32
// ============================================================================

#include <GL/freeglut.h>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <vector>
#include <string>
#include <random>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE 0x809D
#endif

// Camera and Window Globals
float cameraX = 0.0f;
float cameraY = 0.0f;

void handleKeyDown(unsigned char key, int x, int y) {
    keyStates[key] = true;
    if (key == '1') loadLevel(1);
    if (key == '3') loadLevel(3);

// ----------------------------------------------------------------------------
//  Tunables
// ----------------------------------------------------------------------------
static const int   WINW = 1000, WINH = 640;
static const float TILE = 40.0f;
static const float DT   = 1.0f / 60.0f;

static const float GRAVITY        = 2600.0f;
static const float MAXFALL        = 1500.0f;
static const float GROUND_ACCEL   = 6000.0f;
static const float AIR_ACCEL      = 3600.0f;
static const float MOVE_MAX       = 330.0f;
static const float GROUND_FRICTION= 4600.0f;
static const float AIR_FRICTION   = 500.0f;

static const float JUMP_VEL       = 830.0f;
static const float DJUMP_VEL      = 730.0f;
static const int   MAX_JUMPS      = 2;
static const float COYOTE         = 0.10f;
static const float JUMP_BUFFER    = 0.12f;
static const float CUT_JUMP       = 0.45f;   // release-to-cut jump height

static const float WALL_SLIDE     = 150.0f;
static const float WALL_JUMP_VX   = 470.0f;
static const float WALL_JUMP_VY   = 800.0f;
static const float WALL_LOCK      = 0.12f;   // air-control lock after wall jump

static const float DASH_SPEED     = 1180.0f;
static const float DASH_TIME      = 0.15f;
static const float DASH_CD        = 0.40f;
static const int   MAX_DASH       = 1;

static const float NW = 34, NH = 48;   // normal size
static const float SW = 24, SH = 26;   // small size

static const float CRUMBLE_DELAY   = 0.40f;
static const float CRUMBLE_RESPAWN = 2.2f;
static const float DEATH_Y         = -70.0f;

// ----------------------------------------------------------------------------
//  Small helpers
// ----------------------------------------------------------------------------
static std::mt19937 rng(1337u);
static float frand(float a, float b){
    return a + (b - a) * (float)(rng() / (double)rng.max());
}
static float clampf(float v, float lo, float hi){ return v < lo ? lo : (v > hi ? hi : v); }
static float lerpf(float a, float b, float t){ return a + (b - a) * t; }

struct Rect { float x, y, w, h; };
static bool overlap(const Rect& a, const Rect& b){
    return a.x < b.x + b.w && a.x + a.w > b.x &&
           a.y < b.y + b.h && a.y + a.h > b.y;
}

// ----------------------------------------------------------------------------
//  World objects
// ----------------------------------------------------------------------------
struct Spike { Rect box; int dir; };                 // dir 0 = up, 1 = down
struct Coin  { float x, y; bool got; };
struct Check { float x, y; bool active; };
struct Plat  { float x, y, w, h; float ax, ay, range, speed; int axis; float phase; float dx, dy; };
struct Blade { float x, y, r, ang, spin; float ax, ay, range, speed; int axis; float phase; };
struct Crumble { float x, y, w, h; int state; float t; };  // 0 solid,1 shaking,2 gone
struct Particle { float x, y, vx, vy, life, max, size, r, g, b; };

static std::vector<Rect>     solids;
static std::vector<Spike>    spikes;
static std::vector<Coin>     coins;
static std::vector<Check>    checks;
static std::vector<Plat>     plats;
static std::vector<Blade>    blades;
static std::vector<Crumble>  crumbs;
static std::vector<Particle> parts;

struct BgCrystal { float x, y, s, layer; };
static std::vector<BgCrystal> bg;

static Rect  goal;
static float levelWidth = 0;
static int   coinsTotal = 0;

// ----------------------------------------------------------------------------
//  Player
// ----------------------------------------------------------------------------
struct Player {
    float x, y, w, h, vx, vy;
    bool  onGround, wallL, wallR;
    int   jumpsLeft;
    bool  small;
    float scaleAnim;        // visual lerp toward small/normal
    float coyote, jumpBuf;
    float dashTime, dashCD; int dashLeft; bool dashing; float dashDir;
    float wallLock;
    int   facing;
    float sqx, sqy;         // squash & stretch
    bool  alive; float deadT;
    float spawnX, spawnY;
    int   carrier;          // moving platform being ridden, -1 none
    bool  blockedFlash;     // tried to grow but blocked
};
static Player P;

// ----------------------------------------------------------------------------
//  Game state
// ----------------------------------------------------------------------------
static int   gameState = 0;      // 0 play, 1 win
static int   deaths     = 0;
static int   coinsGot   = 0;
static float gameTime   = 0;
static float gTime      = 0;      // animation clock
static float camX       = 0;
static float shake      = 0;
static float introT     = 4.0f;   // title fade

// input
static bool key[256]   = {false};
static bool spec[512]  = {false};
static bool pJump=false, pDash=false, pShrink=false, pRestart=false, pEnter=false;

// ============================================================================
//  Particles
// ============================================================================
static void burst(float x, float y, int n, float r, float g, float b, float spd, float life){
    for(int i=0;i<n;i++){
        float a = frand(0, 2*M_PI), s = frand(spd*0.3f, spd);
        Particle p;
        p.x=x; p.y=y; p.vx=cosf(a)*s; p.vy=sinf(a)*s;
        p.life=p.max=frand(life*0.5f, life);
        p.size=frand(2,5); p.r=r; p.g=g; p.b=b;
        parts.push_back(p);
    }
}
static void dust(float x, float y, int n){
    for(int i=0;i<n;i++){
        Particle p;
        p.x=x+frand(-8,8); p.y=y; p.vx=frand(-60,60); p.vy=frand(30,140);
        p.life=p.max=frand(0.2f,0.45f); p.size=frand(2,4);
        p.r=0.8f; p.g=0.9f; p.b=1.0f;
        parts.push_back(p);
    }
}

// ============================================================================
//  Level construction
// ============================================================================
static void clearLevel(){
    solids.clear(); spikes.clear(); coins.clear(); checks.clear();
    plats.clear(); blades.clear(); crumbs.clear(); parts.clear();
    levelWidth = 0; coinsTotal = 0;
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

static void buildLevel(){
    clearLevel();
    const float T = TILE;
    auto SOLID = [&](float tx,float ty,float tw,float th){
        solids.push_back({tx*T, ty*T, tw*T, th*T});
        if((tx+tw)*T > levelWidth) levelWidth = (tx+tw)*T;
    };
    auto SPK = [&](float tx,float ty,int n,int dir){
        for(int i=0;i<n;i++)
            spikes.push_back({{(tx+i)*T+5, ty*T, 30, 26}, dir});
    };
    auto COIN = [&](float tx,float ty){
        coins.push_back({tx*T+T*0.5f, ty*T+T*0.5f, false}); coinsTotal++;
    };
    auto CHK = [&](float tx,float ty){ checks.push_back({tx*T, ty*T, false}); };
    auto HPLAT = [&](float tx,float ty,float tw,float range,float speed){
        Plat p; p.x=tx*T; p.y=ty*T; p.w=tw*T; p.h=0.6f*T;
        p.ax=tx*T; p.ay=ty*T; p.range=range; p.speed=speed; p.axis=0; p.phase=0; p.dx=p.dy=0;
        plats.push_back(p);
    };
    auto VPLAT = [&](float tx,float ty,float tw,float range,float speed){
        Plat p; p.x=tx*T; p.y=ty*T; p.w=tw*T; p.h=0.6f*T;
        p.ax=tx*T; p.ay=ty*T; p.range=range; p.speed=speed; p.axis=1; p.phase=0; p.dx=p.dy=0;
        plats.push_back(p);
    };
    auto BLADE_ = [&](float tx,float ty,float r,float spin){
        Blade b; b.x=tx*T; b.y=ty*T; b.r=r; b.ang=0; b.spin=spin;
        b.ax=tx*T; b.ay=ty*T; b.range=0; b.speed=0; b.axis=0; b.phase=0; blades.push_back(b);
    };
    auto VBLADE = [&](float tx,float ty,float r,float spin,float range,float speed){
        Blade b; b.x=tx*T; b.y=ty*T; b.r=r; b.ang=0; b.spin=spin;
        b.ax=tx*T; b.ay=ty*T; b.range=range; b.speed=speed; b.axis=1; b.phase=0; blades.push_back(b);
    };
    auto CRUMB = [&](float tx,float tw){
        crumbs.push_back({tx*T, 60.0f, tw*T, 20.0f, 0, 0});
    };

    // floor stand height = y 80 (2 tiles).  Pits = gaps with spikes at the bottom.
    // Every required jump is <=4 tiles across or <=2 tiles up (reachable by a
    // single jump).  Double-jump / dash are safety + for the high bonus coins.

    // ---- Room A : intro (move + jump) -----------------------------------
    SOLID(0,0,28,2);
    P.spawnX = 3*T; P.spawnY = 80;
    COIN(8,3); COIN(11,3);
    SOLID(15,3,3,1);  COIN(16,5);            // small step
    SOLID(21,3,3,1);  COIN(22,5);
    CHK(26,2);
    SPK(28,0,3,0);                           // pit 28-31

    // ---- Room B : double-jump for high coins ----------------------------
    SOLID(31,0,13,2);                        // floor 31-44
    SOLID(34,4,2,1);  COIN(35,6);            // high bonus
    SOLID(38,5,2,1);  COIN(39,7);
    CHK(42,2);
    SPK(44,0,4,0);                           // pit 44-48

    // ---- Room C : steps ------------------------------------------------
    SOLID(48,0,10,2);                        // floor 48-58
    SOLID(50,3,2,1);  COIN(51,5);
    SOLID(53,5,2,1);  COIN(54,7);
    CHK(56,2);
    SPK(58,0,4,0);                           // pit 58-62

    // ---- Room D : SHRINK tunnel ----------------------------------------
    SOLID(62,0,14,2);                        // floor 62-76
    // low ceiling over 65..73 : floor(80) -> ceiling bottom(118) = 38px gap.
    // normal player (48 tall) cannot fit; small player (26 tall) can.
    SOLID(65,2.95f,8,7);                     // ceiling block, bottom y=118
    COIN(67,2.3f); COIN(70,2.3f); COIN(72,2.3f);
    CHK(63,2);
    CHK(74,2);
    SPK(76,0,4,0);                           // pit 76-80

    // ---- Room E : staircase climb --------------------------------------
    SOLID(80,0,3,2);                         // floor 80-83
    SOLID(84,3,2,1);                         // top y160
    SOLID(87,5,2,1);                         // top y240
    SOLID(90,4,2,1);                         // top y200 (step down)
    COIN(85,8); COIN(88,10);                 // high bonus coins
    SOLID(92,0,6,2);                         // floor 92-98
    CHK(94,2);

    // ---- Room F : moving platforms over a spike pit ---------------------
    SPK(98,0,14,0);                          // big pit 98-112
    HPLAT(98.5f, 2.6f, 3, 110, 1.4f);        // x~3940 y104, swings horizontally
    VPLAT(103.0f,2.6f, 3,  80, 1.7f);        // x4120 y104, rides up/down
    HPLAT(107.5f,2.8f, 3, 110, 1.5f);        // x4300 y112
    BLADE_(105,8,24, 4.0f);                  // spinning hazard up high (threat)
    SOLID(112,0,9,2);                        // landing floor 112-121
    COIN(115,4); COIN(117,4);
    CHK(114,2);

    // ---- Room G : crumbling bridge + blades -----------------------------
    SPK(121,0,12,0);                         // pit 121-133
    CRUMB(122,2); CRUMB(125,2); CRUMB(128,2); CRUMB(131,2);
    VBLADE(124,4,24, 5.0f, 110, 2.4f);
    VBLADE(130,4,24,-5.0f, 110, 2.0f);
    SOLID(133,0,8,2);                        // floor 133-141
    CHK(135,2);

    // ---- Room H : final gauntlet ---------------------------------------
    SPK(141,0,4,0);                          // pit 141-145
    SOLID(143,4,2,1);  COIN(143,6);          // optional high coin
    SPK(145,0,4,0);                          // pit 145-149 with moving blade
    VBLADE(147,4,24, 6.0f, 140, 2.8f);
    SOLID(149,0,14,2);                       // final floor 149-163
    COIN(153,4); COIN(155,4); COIN(157,4);
    goal = { 159*T, 80, 2*T, 3*T };          // crystal portal

    levelWidth = (goal.x + 6*T);

    // ---- decorative background crystals ---------------------------------
    bg.clear();
    for(int i=0;i<70;i++){
        BgCrystal c;
        c.x = frand(0, levelWidth);
        c.y = frand(60, WINH-40);
        c.s = frand(18, 70);
        c.layer = frand(0.25f, 0.6f);
        bg.push_back(c);
    }
}

// ============================================================================
//  Player reset / death
// ============================================================================
static void respawn(){
    P.x = P.spawnX; P.y = P.spawnY;
    P.vx = P.vy = 0;
    P.small = false; P.scaleAnim = 1.0f;
    P.w = NW; P.h = NH;
    P.onGround = false; P.wallL = P.wallR = false;
    P.jumpsLeft = MAX_JUMPS;
    P.coyote = P.jumpBuf = 0;
    P.dashTime = 0; P.dashCD = 0; P.dashLeft = MAX_DASH; P.dashing = false; P.dashDir = 0;
    P.wallLock = 0; P.facing = 1;
    P.sqx = P.sqy = 1.0f;
    P.alive = true; P.deadT = 0; P.carrier = -1; P.blockedFlash = false;
}
static void killPlayer(){
    if(!P.alive) return;
    P.alive = false; P.deadT = 0.45f;
    deaths++;
    shake = 0.55f;
    burst(P.x + P.w*0.5f, P.y + P.h*0.5f, 28, 1.0f, 0.35f, 0.30f, 420, 0.7f);
}
static void resetGame(){
    deaths = 0; coinsGot = 0; gameTime = 0; gameState = 0;
    introT = 3.0f; shake = 0; camX = 0;
    buildLevel();
    for(auto& c : coins)  c.got = false;
    for(auto& c : checks) c.active = false;
    for(auto& c : crumbs){ c.state = 0; c.t = 0; }
    respawn();
}

// ============================================================================
//  Collision helpers
// ============================================================================
struct Col { Rect r; int plat; int crum; };
static std::vector<Col> cols;

static void buildColliders(){
    cols.clear();
    for(auto& s : solids) cols.push_back({s, -1, -1});
    for(size_t i=0;i<crumbs.size();i++)
        if(crumbs[i].state != 2)
            cols.push_back({{crumbs[i].x,crumbs[i].y,crumbs[i].w,crumbs[i].h}, -1, (int)i});
    for(size_t i=0;i<plats.size();i++)
        cols.push_back({{plats[i].x,plats[i].y,plats[i].w,plats[i].h}, (int)i, -1});
}

// can the player fit (no solid overlap) with a box?  (solids + closed ceilings only)
static bool fits(float x, float y, float w, float h){
    Rect b{x,y,w,h};
    for(auto& s : solids) if(overlap(b,s)) return false;
    return true;
}

// ============================================================================
//  Update
// ============================================================================
static void doJump(int kind){
    // kind: 0 ground, 1 air/double, 2 wall
    if(kind==2){
        int dir = P.wallL ? 1 : -1;
        P.vy = WALL_JUMP_VY;
        P.vx = dir * WALL_JUMP_VX;
        P.wallLock = WALL_LOCK;
        P.jumpsLeft = MAX_JUMPS - 1;
        P.facing = dir;
        dust(P.x + (dir>0?0:P.w), P.y + P.h*0.5f, 8);
    } else if(kind==0){
        P.vy = JUMP_VEL;
        P.jumpsLeft = MAX_JUMPS - 1;
        dust(P.x + P.w*0.5f, P.y, 8);
    } else {
        P.vy = DJUMP_VEL;
        P.jumpsLeft--;
        burst(P.x + P.w*0.5f, P.y + 4, 12, 0.6f, 0.85f, 1.0f, 240, 0.35f);
    }
    P.sqx = 0.7f; P.sqy = 1.35f;
    P.coyote = 0; P.jumpBuf = 0; P.onGround = false;
}

static void toggleShrink(){
    if(!P.small){
        // shrink: keep feet, recentre x
        P.x += (P.w - SW) * 0.5f;
        P.w = SW; P.h = SH; P.small = true;
        burst(P.x + P.w*0.5f, P.y + P.h*0.5f, 14, 1.0f, 0.85f, 0.4f, 220, 0.3f);
    } else {
        float nx = P.x - (NW - SW) * 0.5f;
        if(fits(nx, P.y, NW, NH)){
            P.x = nx; P.w = NW; P.h = NH; P.small = false;
            burst(P.x + P.w*0.5f, P.y + P.h*0.5f, 14, 0.6f, 0.9f, 1.0f, 220, 0.3f);
        } else {
            P.blockedFlash = true;           // not enough room
        }
    }
}

static void update(){
    gTime += DT;
    if(shake > 0) shake -= DT;

    // ---- input edges --------------------------------------------------
    bool L = key['a'] || spec[GLUT_KEY_LEFT];
    bool R = key['d'] || spec[GLUT_KEY_RIGHT];
    bool jumpH   = key['w'] || key[' '] || spec[GLUT_KEY_UP];
    bool dashH   = key['j'] || key['l'];
    bool shrinkH = key['s'] || spec[GLUT_KEY_DOWN];
    bool restH   = key['r'];
    bool entH    = key[13]  || key[10];

    bool jumpE   = jumpH   && !pJump;
    bool jumpRel = !jumpH  && pJump;
    bool dashE   = dashH   && !pDash;
    bool shrinkE = shrinkH && !pShrink;
    bool restE   = restH   && !pRestart;
    bool entE    = entH    && !pEnter;
    pJump=jumpH; pDash=dashH; pShrink=shrinkH; pRestart=restH; pEnter=entH;

    // ---- particles (always) -------------------------------------------
    for(size_t i=0;i<parts.size();){
        Particle& p = parts[i];
        p.life -= DT;
        if(p.life <= 0){ parts[i] = parts.back(); parts.pop_back(); continue; }
        p.vy -= 600*DT; p.x += p.vx*DT; p.y += p.vy*DT;
        i++;
    }

    // ---- win screen ---------------------------------------------------
    if(gameState == 1){
        if(entE){ resetGame(); }
        return;
    }

    if(introT > 0) introT -= DT;
    gameTime += DT;

    // ---- moving platforms (advance, compute delta) --------------------
    for(auto& p : plats){
        float oldx=p.x, oldy=p.y;
        p.phase += p.speed * DT;
        float off = sinf(p.phase) * p.range;
        if(p.axis==0){ p.x = p.ax + off; p.y = p.ay; }
        else         { p.y = p.ay + off; p.x = p.ax; }
        p.dx = p.x - oldx; p.dy = p.y - oldy;
    }
    // ---- moving blades -------------------------------------------------
    for(auto& b : blades){
        b.ang += b.spin * DT;
        if(b.range > 0){
            b.phase += b.speed * DT;
            float off = sinf(b.phase) * b.range;
            if(b.axis==0) b.x = b.ax + off; else b.y = b.ay + off;
        }
    }

    if(P.alive){
        // restart to checkpoint
        if(restE){ respawn(); }

        // ride the platform we stood on
        if(P.onGround && P.carrier >= 0 && P.carrier < (int)plats.size()){
            P.x += plats[P.carrier].dx;
            P.y += plats[P.carrier].dy;
        }

        if(shrinkE) toggleShrink();

        // ---- horizontal control --------------------------------------
        float desire = (R?1.0f:0.0f) - (L?1.0f:0.0f);
        if(desire != 0) P.facing = (desire>0?1:-1);

        if(P.dashing){
            P.vx = P.dashDir * DASH_SPEED;
            P.vy = 0;
            P.dashTime -= DT;
            if((gTime*60.0f) - floorf(gTime*60.0f) < 0.5f)
                burst(P.x+P.w*0.5f, P.y+P.h*0.5f, 2, 0.5f,0.9f,1.0f, 60, 0.25f);
            if(P.dashTime <= 0){ P.dashing = false; P.vx *= 0.45f; }
        } else {
            float accel = P.onGround ? GROUND_ACCEL : AIR_ACCEL;
            if(P.wallLock > 0){ P.wallLock -= DT; accel *= 0.25f; }
            P.vx += desire * accel * DT;
            float mx = MOVE_MAX * (P.small ? 1.10f : 1.0f);
            P.vx = clampf(P.vx, -mx, mx);
            if(desire == 0){
                float fr = (P.onGround ? GROUND_FRICTION : AIR_FRICTION) * DT;
                if(P.vx > 0) P.vx = (P.vx>fr? P.vx-fr : 0);
                else         P.vx = (P.vx<-fr? P.vx+fr : 0);
            }
        }

        // ---- jump buffer / coyote ------------------------------------
        if(jumpE) P.jumpBuf = JUMP_BUFFER;
        if(P.jumpBuf > 0) P.jumpBuf -= DT;
        if(P.coyote > 0)  P.coyote -= DT;

        bool wallContact = (P.wallL && L) || (P.wallR && R);
        if(P.jumpBuf > 0 && !P.dashing){
            if(P.coyote > 0)            doJump(0);
            else if(wallContact && !P.onGround) doJump(2);
            else if(P.jumpsLeft > 0)    doJump(1);
        }
        // variable jump height: releasing the button early cuts the rise
        if(jumpRel && P.vy > 0) P.vy *= CUT_JUMP;

        // ---- dash ----------------------------------------------------
        if(dashE && !P.dashing && P.dashCD <= 0 && P.dashLeft > 0){
            P.dashing = true; P.dashTime = DASH_TIME;
            P.dashDir = (desire != 0) ? desire : P.facing;
            P.dashCD = DASH_CD; P.dashLeft--;
            P.sqx = 1.4f; P.sqy = 0.7f;
            burst(P.x+P.w*0.5f, P.y+P.h*0.5f, 14, 0.5f,0.9f,1.0f, 260, 0.3f);
        }
        if(P.dashCD > 0) P.dashCD -= DT;

        // ---- gravity & wall slide ------------------------------------
        if(!P.dashing){
            P.vy -= GRAVITY * DT;
            if(P.vy < -MAXFALL) P.vy = -MAXFALL;
            if(wallContact && P.vy < 0 && !P.onGround)
                if(P.vy < -WALL_SLIDE) P.vy = -WALL_SLIDE;
        }

        // ---- integrate with sub-stepping -----------------------------
        buildColliders();
        float total = fabsf(P.vx*DT) + fabsf(P.vy*DT);
        int steps = (int)ceilf(total / (TILE*0.4f));
        if(steps < 1) steps = 1; if(steps > 10) steps = 10;
        float sdt = DT / steps;

        bool landed=false, ceiled=false, stoodCrumb=false; int carrier=-1; int crumbIdx=-1;
        P.wallL = P.wallR = false;

        for(int s=0;s<steps;s++){
            // X
            P.x += P.vx * sdt;
            for(auto& c : cols){
                Rect pr{P.x,P.y,P.w,P.h};
                if(overlap(pr, c.r)){
                    if(P.vx > 0){ P.x = c.r.x - P.w; P.wallR = true; }
                    else if(P.vx < 0){ P.x = c.r.x + c.r.w; P.wallL = true; }
                    if(!P.dashing) P.vx = 0; else { P.dashing=false; P.vx=0; }
                }
            }
            // Y
            P.y += P.vy * sdt;
            for(auto& c : cols){
                Rect pr{P.x,P.y,P.w,P.h};
                if(overlap(pr, c.r)){
                    if(P.vy <= 0){
                        P.y = c.r.y + c.r.h; landed = true; P.vy = 0;
                        carrier = c.plat; if(c.crum>=0){ stoodCrumb=true; crumbIdx=c.crum; }
                    } else {
                        P.y = c.r.y - P.h; ceiled = true; P.vy = 0;
                    }
                }
            }
        }

        if(landed){
            if(!P.onGround){
                // landing impact
                dust(P.x+P.w*0.5f, P.y, 10);
                P.sqx = 1.3f; P.sqy = 0.7f;
                if(P.vy==0) {}
            }
            P.onGround = true;
            P.jumpsLeft = MAX_JUMPS;
            P.dashLeft  = MAX_DASH;
            P.coyote    = COYOTE;
            P.carrier   = carrier;
        } else {
            P.onGround = false;
            P.carrier  = -1;
        }
        (void)ceiled;

        // crumble trigger
        if(stoodCrumb && crumbs[crumbIdx].state == 0){
            crumbs[crumbIdx].state = 1; crumbs[crumbIdx].t = CRUMBLE_DELAY;
        }

        // squash/stretch relax
        P.sqx = lerpf(P.sqx, 1.0f, 0.20f);
        P.sqy = lerpf(P.sqy, 1.0f, 0.20f);
        P.scaleAnim = lerpf(P.scaleAnim, P.small ? 0.0f : 1.0f, 0.25f);

        // ---- hazards -------------------------------------------------
        Rect pr{P.x+3, P.y+2, P.w-6, P.h-4};
        for(auto& sp : spikes) if(overlap(pr, sp.box)){ killPlayer(); break; }
        if(P.alive){
            float pcx = P.x+P.w*0.5f, pcy = P.y+P.h*0.5f;
            for(auto& b : blades){
                float dx=pcx-b.x, dy=pcy-b.y;
                float rr = b.r + (P.small?SH:NH)*0.28f;
                if(dx*dx+dy*dy < rr*rr){ killPlayer(); break; }
            }
        }
        if(P.alive && P.y < DEATH_Y) killPlayer();

        // ---- pickups / checkpoints / goal ----------------------------
        if(P.alive){
            Rect pb{P.x,P.y,P.w,P.h};
            for(auto& c : coins){
                if(!c.got){
                    Rect cb{c.x-14, c.y-14, 28, 28};
                    if(overlap(pb, cb)){
                        c.got = true; coinsGot++;
                        burst(c.x, c.y, 12, 0.55f, 0.90f, 1.0f, 220, 0.4f);
                    }
                }
            }
            for(auto& c : checks){
                Rect cb{c.x, c.y, TILE, 3*TILE};
                if(overlap(pb, cb) && !c.active){
                    c.active = true; P.spawnX = c.x; P.spawnY = c.y;
                    burst(c.x+TILE*0.5f, c.y+TILE, 16, 0.5f, 1.0f, 0.7f, 240, 0.5f);
                }
            }
            if(overlap(pb, goal)){
                gameState = 1;
                burst(goal.x+goal.w*0.5f, goal.y+goal.h*0.5f, 50, 0.6f, 0.95f, 1.0f, 480, 1.0f);
            }
        }
    } else {
        // dead: brief freeze then respawn
        P.deadT -= DT;
        if(P.deadT <= 0) respawn();
    }

    // ---- crumble timers ----------------------------------------------
    for(auto& c : crumbs){
        if(c.state == 1){ c.t -= DT; if(c.t <= 0){ c.state = 2; c.t = CRUMBLE_RESPAWN; } }
        else if(c.state == 2){ c.t -= DT; if(c.t <= 0){ c.state = 0; } }
    }

    // ---- camera -------------------------------------------------------
    float target = P.x + P.w*0.5f - WINW*0.42f;
    target = clampf(target, 0, (levelWidth>WINW? levelWidth-WINW : 0));
    camX = lerpf(camX, target, 0.12f);

    glutPostRedisplay();
}

// ============================================================================
//  Rendering helpers
// ============================================================================
static void fillRect(float x,float y,float w,float h){
    glBegin(GL_QUADS);
        glVertex2f(x,y); glVertex2f(x+w,y); glVertex2f(x+w,y+h); glVertex2f(x,y+h);
    glEnd();
}
static void corner(float cx,float cy,float r,float a0,float a1){
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx,cy);
    int seg=8; for(int i=0;i<=seg;i++){ float a=a0+(a1-a0)*i/seg; glVertex2f(cx+cosf(a)*r,cy+sinf(a)*r);}
    glEnd();
}
static void roundRect(float x,float y,float w,float h,float r){
    if(r>w*0.5f) r=w*0.5f; if(r>h*0.5f) r=h*0.5f;
    fillRect(x+r,y,w-2*r,h);
    fillRect(x,y+r,r,h-2*r);
    fillRect(x+w-r,y+r,r,h-2*r);
    corner(x+r,y+r,r,(float)M_PI,(float)(1.5*M_PI));
    corner(x+w-r,y+r,r,(float)(1.5*M_PI),(float)(2*M_PI));
    corner(x+w-r,y+h-r,r,0,(float)(0.5*M_PI));
    corner(x+r,y+h-r,r,(float)(0.5*M_PI),(float)M_PI);
}
static void disc(float cx,float cy,float r,int seg){
    glBegin(GL_TRIANGLE_FAN); glVertex2f(cx,cy);
    for(int i=0;i<=seg;i++){ float a=2*M_PI*i/seg; glVertex2f(cx+cosf(a)*r,cy+sinf(a)*r);} glEnd();
}
static void glow(float cx,float cy,float r,float cr,float cg,float cb,float a){
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glBegin(GL_TRIANGLE_FAN);
    glColor4f(cr,cg,cb,a); glVertex2f(cx,cy);
    glColor4f(cr,cg,cb,0);
    int seg=22; for(int i=0;i<=seg;i++){ float ang=2*M_PI*i/seg; glVertex2f(cx+cosf(ang)*r,cy+sinf(ang)*r);}
    glEnd();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}
static void text(float x,float y,void* font,const char* s){
    glRasterPos2f(x,y);
    for(const char* c=s;*c;c++) glutBitmapCharacter(font,*c);
}
static float textW(void* font,const char* s){
    return (float)glutBitmapLength(font,(const unsigned char*)s);
}
static void textC(float cx,float y,void* font,const char* s){ text(cx-textW(font,s)*0.5f,y,font,s); }

static void setProj(float ox){
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    glOrtho(ox, ox+WINW, 0, WINH, -1, 1);
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
}

// original-style block: blue tile grid with cyan borders + bright top edge + dots
static void drawBlock(const Rect& s){
    float x=s.x, y=s.y, w=s.w, h=s.h;
    // gradient fill
    glBegin(GL_QUADS);
        glColor3f(0.17f,0.29f,0.58f); glVertex2f(x,y);   glVertex2f(x+w,y);
        glColor3f(0.27f,0.43f,0.80f); glVertex2f(x+w,y+h); glVertex2f(x,y+h);
    glEnd();
    // cyan grid lines on every tile boundary
    glColor4f(0.33f,0.78f,1.0f,0.85f);
    glBegin(GL_LINES);
    for(float gx=x; gx<=x+w+0.5f; gx+=TILE){ glVertex2f(gx,y); glVertex2f(gx,y+h); }
    for(float gy=y; gy<=y+h+0.5f; gy+=TILE){ glVertex2f(x,gy); glVertex2f(x+w,gy); }
    glEnd();
    // bright cyan top edge
    glColor3f(0.55f,0.92f,1.0f); fillRect(x, y+h-4, w, 4);
    // two little marks per cell
    glColor4f(0.55f,0.75f,0.95f,0.9f);
    for(float cx=x+TILE*0.5f; cx<x+w; cx+=TILE)
        for(float cy=y+TILE*0.5f; cy<y+h; cy+=TILE){
            fillRect(cx-8,cy-2,4,4); fillRect(cx+4,cy-2,4,4);
        }
}

static void drawSpike(const Spike& sp){
    float x=sp.box.x, y=sp.box.y, w=sp.box.w, h=sp.box.h;
    // sharp crystalline spikes (danger by shape), cyan/white to match theme
    glColor3f(0.60f,0.88f,1.0f);
    glBegin(GL_TRIANGLES);
    if(sp.dir==0){ glVertex2f(x,y); glVertex2f(x+w,y); glVertex2f(x+w*0.5f,y+h); }
    else         { glVertex2f(x,y+h); glVertex2f(x+w,y+h); glVertex2f(x+w*0.5f,y); }
    glEnd();
    glColor3f(0.92f,1.0f,1.0f);
    glBegin(GL_TRIANGLES);
    if(sp.dir==0){ glVertex2f(x+w*0.35f,y); glVertex2f(x+w*0.65f,y); glVertex2f(x+w*0.5f,y+h); }
    else         { glVertex2f(x+w*0.35f,y+h); glVertex2f(x+w*0.65f,y+h); glVertex2f(x+w*0.5f,y); }
    glEnd();
}

static void drawCoin(const Coin& c){
    float yo = sinf(gTime*2.2f + c.x*0.02f)*3.0f;
    float s = 10;
    // cyan crystal shard
    glColor3f(0.50f,0.88f,1.0f);
    glBegin(GL_QUADS);
        glVertex2f(c.x, c.y+yo+s); glVertex2f(c.x+s, c.y+yo);
        glVertex2f(c.x, c.y+yo-s); glVertex2f(c.x-s, c.y+yo);
    glEnd();
    glColor3f(0.92f,1.0f,1.0f);
    float s2=s*0.45f;
    glBegin(GL_QUADS);
        glVertex2f(c.x, c.y+yo+s2); glVertex2f(c.x+s2, c.y+yo);
        glVertex2f(c.x, c.y+yo-s2); glVertex2f(c.x-s2, c.y+yo);
    glEnd();
    glColor3f(0.33f,0.78f,1.0f);
    glBegin(GL_LINE_LOOP);
        glVertex2f(c.x, c.y+yo+s); glVertex2f(c.x+s, c.y+yo);
        glVertex2f(c.x, c.y+yo-s); glVertex2f(c.x-s, c.y+yo);
    glEnd();
}

static void drawBlade(const Blade& b){
    // flat cyan 4-pointed star (crystal blade) with white core
    const int pts=4;
    float ro=b.r, ri=b.r*0.34f;
    glColor3f(0.60f,0.90f,1.0f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(b.x,b.y);
    for(int i=0;i<=pts*2;i++){
        float a=b.ang + (float)M_PI*i/pts;
        float rr=(i%2==0)?ro:ri;
        glVertex2f(b.x+cosf(a)*rr, b.y+sinf(a)*rr);
    }
    glEnd();
    glColor3f(0.92f,1.0f,1.0f); disc(b.x,b.y,b.r*0.20f,12);
}

static void drawCrumble(const Crumble& c){
    if(c.state==2){
        // gone: faint outline only
        glColor4f(0.30f,0.55f,0.85f,0.18f);
        glBegin(GL_LINE_LOOP);
            glVertex2f(c.x,c.y); glVertex2f(c.x+c.w,c.y);
            glVertex2f(c.x+c.w,c.y+c.h); glVertex2f(c.x,c.y+c.h);
        glEnd();
        return;
    }
    float jx=0, jy=0;
    if(c.state==1){ jx=frand(-2,2); jy=frand(-2,2); }   // shaking before it drops
    // duller fragile block (so it reads as different from solid blocks)
    glColor3f(0.18f,0.28f,0.46f); fillRect(c.x+jx,c.y+jy,c.w,c.h);
    glColor4f(0.40f,0.72f,0.95f,0.9f);
    glBegin(GL_LINE_LOOP);
        glVertex2f(c.x+jx,c.y+jy); glVertex2f(c.x+jx+c.w,c.y+jy);
        glVertex2f(c.x+jx+c.w,c.y+jy+c.h); glVertex2f(c.x+jx,c.y+jy+c.h);
    glEnd();
    glBegin(GL_LINES);   // crack
        glVertex2f(c.x+jx+c.w*0.5f, c.y+jy+c.h); glVertex2f(c.x+jx+c.w*0.4f, c.y+jy);
    glEnd();
    glColor3f(0.50f,0.85f,1.0f); fillRect(c.x+jx,c.y+jy+c.h-3,c.w,3);
}

static void drawPlat(const Plat& p){
    glColor3f(0.20f,0.40f,0.72f); fillRect(p.x,p.y,p.w,p.h);
    glColor4f(0.33f,0.78f,1.0f,0.95f);
    glBegin(GL_LINE_LOOP);
        glVertex2f(p.x,p.y); glVertex2f(p.x+p.w,p.y);
        glVertex2f(p.x+p.w,p.y+p.h); glVertex2f(p.x,p.y+p.h);
    glEnd();
    glColor3f(0.55f,0.92f,1.0f); fillRect(p.x,p.y+p.h-4,p.w,4);
    glColor3f(0.55f,0.75f,0.95f);
    fillRect(p.x+p.w*0.5f-8,p.y+p.h*0.5f-2,4,4);
    fillRect(p.x+p.w*0.5f+4,p.y+p.h*0.5f-2,4,4);
}

static void drawGoal(){
    float x=goal.x, y=goal.y, w=goal.w, h=goal.h;
    // EXIT portal: dark body, shimmering cyan lines, bright frame
    glColor3f(0.08f,0.20f,0.42f); fillRect(x,y,w,h);
    glColor4f(0.45f,0.88f,1.0f,0.75f);
    glBegin(GL_LINES);
    for(int i=0;i<4;i++){
        float fx = x + w*(0.2f+0.2f*i) + sinf(gTime*3.0f+i)*3.0f;
        glVertex2f(fx, y+5); glVertex2f(fx, y+h-5);
    }
    glEnd();
    glColor3f(0.55f,0.92f,1.0f);
    glLineWidth(3.0f);
    glBegin(GL_LINE_LOOP);
        glVertex2f(x,y); glVertex2f(x+w,y); glVertex2f(x+w,y+h); glVertex2f(x,y+h);
    glEnd();
    glLineWidth(2.0f);
}

static void drawCheck(const Check& c){
    float cx=c.x+TILE*0.5f, base=c.y;
    glColor3f(0.40f,0.72f,0.95f); fillRect(cx-2, base, 3, 72);    // pole
    if(c.active) glColor3f(0.55f,0.92f,1.0f);                      // lit = bright cyan
    else         glColor3f(0.22f,0.40f,0.58f);                     // unlit = dim
    float wav = c.active ? sinf(gTime*5)*3 : 0;
    glBegin(GL_TRIANGLES);
        glVertex2f(cx+1, base+72); glVertex2f(cx+1, base+52);
        glVertex2f(cx+24+wav, base+62);
    glEnd();
}

static void drawPlayer(){
    float x=P.x, y=P.y, w=P.w, h=P.h;

    // dash afterimages (flat, faint)
    if(P.dashing){
        for(int i=1;i<=3;i++){
            glColor4f(0.40f,0.78f,1.0f, 0.18f - i*0.05f);
            fillRect(x - P.dashDir*i*9, y, w, h);
        }
    }

    // original-style player: blue square, lighter band, cyan outline, bright top
    float r,g,b;
    if(P.small){ r=0.45f; g=0.85f; b=1.00f; }     // small = bright cyan
    else       { r=0.22f; g=0.45f; b=0.90f; }     // normal = blue
    if(P.blockedFlash){ r=1.0f; g=0.45f; b=0.45f; P.blockedFlash=false; }

    glColor3f(r,g,b); fillRect(x,y,w,h);
    glColor3f(clampf(r+0.30f,0,1), clampf(g+0.30f,0,1), clampf(b+0.08f,0,1));
    fillRect(x, y+h*0.52f, w, h*0.20f);            // lighter stripe band
    glColor3f(0.55f,0.92f,1.0f); fillRect(x, y+h-3, w, 3);  // bright top
    glColor3f(0.40f,0.85f,1.0f);                    // cyan outline
    glBegin(GL_LINE_LOOP);
        glVertex2f(x,y); glVertex2f(x+w,y); glVertex2f(x+w,y+h); glVertex2f(x,y+h);
    glEnd();
}

// ============================================================================
//  Display
// ============================================================================
static void drawBackground(){
    setProj(0);
    // flat dark-navy fill
    glColor3f(0.039f,0.055f,0.118f); fillRect(0,0,WINW,WINH);
    // faint horizontal scan grid (original look)
    glColor4f(0.20f,0.40f,0.72f,0.20f);
    glBegin(GL_LINES);
    for(float y=0; y<=WINH; y+=30){ glVertex2f(0,y); glVertex2f(WINW,y); }
    glEnd();
    // very faint vertical grid
    glColor4f(0.20f,0.40f,0.72f,0.07f);
    glBegin(GL_LINES);
    for(float x=0; x<=WINW; x+=30){ glVertex2f(x,0); glVertex2f(x,WINH); }
    glEnd();
}

static void display(){
    glClear(GL_COLOR_BUFFER_BIT);

<<<<<<< HEAD
    // Call external render functions
    drawLevel();
    drawPlayer();
=======
    drawBackground();

    // world projection (steady camera - flat original style, no shake)
    setProj(camX);

    for(auto& s : solids)  drawBlock(s);
    for(auto& c : crumbs)  drawCrumble(c);
    for(auto& p : plats)   drawPlat(p);
    for(auto& c : checks)  drawCheck(c);
    for(auto& c : coins)   if(!c.got) drawCoin(c);
    for(auto& sp: spikes)  drawSpike(sp);
    drawGoal();
    for(auto& b : blades)  drawBlade(b);

    // particles (flat little squares - no glow)
    for(auto& p : parts){
        float a = p.life / p.max;
        glColor4f(p.r,p.g,p.b,a);
        fillRect(p.x-p.size*0.5f, p.y-p.size*0.5f, p.size, p.size);
    }

    if(P.alive) drawPlayer();

    // ---- HUD (original cyan-on-navy style) ---------------------------
    setProj(0);
    char buf[128];
    void* F  = GLUT_BITMAP_9_BY_15;
    void* Fs = GLUT_BITMAP_8_BY_13;
    int mm=(int)gameTime/60, ss=(int)gameTime%60;

    glColor3f(0.45f,0.85f,1.0f);
    text(14, WINH-20, F, "LEVEL 2 - Crystal Cavern");
    text(14, WINH-38, F, "Goal: reach the EXIT portal on the right ->");

    snprintf(buf,sizeof(buf),"Crystals %d/%d   Deaths %d   Time %02d:%02d",
             coinsGot, coinsTotal, deaths, mm, ss);
    glColor3f(0.40f,0.72f,0.95f);
    text(14, WINH-58, Fs, buf);

    if(P.small){
        glColor3f(0.70f,0.95f,1.0f);
        text(14, WINH-78, F, "[SCALED: SMALL]");
    }

    // controls legend (bottom)
    glColor3f(0.45f,0.78f,1.0f);
    text(12, 10, Fs,
        "[A]/[D] Move   [W] Jump (press twice = Double Jump)   [J] Dash   [S] Scale Toggle   [R] Restart");

    // win overlay
    if(gameState == 1){
        glColor4f(0.02f,0.04f,0.10f,0.72f); fillRect(0,0,WINW,WINH);
        glColor3f(0.55f,0.92f,1.0f);
        textC(WINW*0.5f, WINH*0.60f, F, "LEVEL COMPLETE!");
        glColor3f(0.45f,0.85f,1.0f);
        textC(WINW*0.5f, WINH*0.52f, F, "You reached the crystal portal.");
        snprintf(buf,sizeof(buf),"Crystals %d/%d     Deaths %d     Time %02d:%02d",
                 coinsGot, coinsTotal, deaths, mm, ss);
        glColor3f(0.40f,0.72f,0.95f);
        textC(WINW*0.5f, WINH*0.45f, Fs, buf);
        glColor3f(0.55f,0.90f,1.0f);
        textC(WINW*0.5f, WINH*0.37f, F, "Press [ENTER] to play again");
    }
>>>>>>> 17a125d00950c327919eafb5ec96464c12ede78c

    glutSwapBuffers();
}

// ============================================================================
//  GLUT plumbing
// ============================================================================
static void reshape(int w,int h){ glViewport(0,0,w,h); }
static void timer(int){ update(); glutTimerFunc(16, timer, 0); }

<<<<<<< HEAD
    loadLevel(1);
}
=======
static void kDown(unsigned char k,int,int){ if(k<256) key[(unsigned char)tolower(k)]=true; }
static void kUp  (unsigned char k,int,int){ if(k<256) key[(unsigned char)tolower(k)]=false; }
static void sDown(int k,int,int){ if(k<512) spec[k]=true; }
static void sUp  (int k,int,int){ if(k<512) spec[k]=false; }
>>>>>>> 17a125d00950c327919eafb5ec96464c12ede78c

int main(int argc, char** argv){
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_MULTISAMPLE);
    glutInitWindowSize(WINW, WINH);
    glutCreateWindow("LEVEL 2 - Crystal Cavern");

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glLineWidth(2.0f);

    resetGame();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(kDown);
    glutKeyboardUpFunc(kUp);
    glutSpecialFunc(sDown);
    glutSpecialUpFunc(sUp);
    glutIgnoreKeyRepeat(1);
    glutTimerFunc(16, timer, 0);

#ifdef GLUT_ACTION_ON_WINDOW_CLOSE
    glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
#endif

    glutMainLoop();
    return 0;
}
