#include "Level4.h"

// Tile legend:
//  0  = empty sky
// 20  = cloud platform (solid, walkable)
// 21  = cloud block (full cloud tile)
// 22  = sun tile (decorative / top row)
// 23  = wind hazard (deadly, like lava)
// 24  = deep sky / void (deadly, bottom row)

int level4Data[ROWS][COLS] =
{
    // Row 0 - top: sun in upper-right corner area
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 22, 22, 22, 0, 0, 0, 0},

    // Row 1
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 22, 22, 22, 0, 0, 0, 0},

    // Row 2 - high cloud platforms
    {0, 0, 21, 21, 21, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 21, 21, 0},

    // Row 3
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 21, 21, 0, 0, 0, 0, 0, 0, 0},

    // Row 4 - open sky
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 21, 21, 0, 0, 0},

    // Row 5 - mid cloud platforms
    {0, 21, 21, 0, 0, 0, 0, 21, 21, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},

    // Row 6
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 21, 21, 0, 0, 0, 0, 21, 0},

    // Row 7 - open gap with wind hazard strip
    {0, 0, 0, 0, 23, 23, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 23, 23, 0, 0},

    // Row 8 - lower cloud platforms
    {0, 0, 0, 21, 21, 0, 0, 0, 0, 0, 0, 21, 21, 21, 0, 0, 0, 0, 0, 0},

    // Row 9
    {0, 21, 0, 0, 0, 0, 0, 0, 21, 21, 0, 0, 0, 0, 0, 0, 0, 21, 21, 0},

    // Row 10 - wide platform area approaching ground clouds
    {0, 0, 0, 0, 0, 21, 21, 0, 0, 0, 0, 0, 0, 0, 21, 21, 0, 0, 0, 0},

    // Row 11 - dense cloud layer (ground)
    {21, 21, 21, 0, 0, 0, 0, 0, 0, 0, 21, 21, 21, 0, 0, 0, 0, 21, 21, 21},

    // Row 12 - solid cloud ground
    {20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20},

    // Row 13 - wind / storm hazard layer
    {23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23},

    // Row 14 - void / deep sky (instant death)
    {24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24}
};
