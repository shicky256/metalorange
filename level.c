#include <SEGA_MTH.H>
#include "game.h"
#include "level.h"
#include "print.h"
#include "sprite.h"

typedef enum {
    NON = -1,
    RED = 0, //red
    BGE = 1, //beige
    GRY = 2, //gray
    GLD = 3, //gold
    BLU = 4, //blue
    ORN = 5, //orange
    WHT = 6, //white
    PUR = 7, //purple
} BLOCK;

typedef struct {
    Fixed32 x;
    Fixed32 y;
    Uint16 tile_no;
} BLOCK_SPR;

//first block tile number in vdp1 memory
static int block_base;

#define BLOCK_WIDTH (MTH_FIXED(16))
#define BLOCK_HEIGHT (MTH_FIXED(8))
#define LEVEL_WIDTH (13)

#define MAX_BLOCKS (LEVEL_WIDTH * 15)
//array holding block sprites
static BLOCK_SPR block_arr[MAX_BLOCKS];
//position for loading blocks into array
static int block_cursor;
//array to hold currently moving row of tiles during start of level animation
static BLOCK_SPR anim_row[LEVEL_WIDTH];
//number of blocks in the row
static int anim_cursor;
//which row of blocks to animate down
static int row_num;
//what y pos the animating row is at
static Fixed32 row_y;

//level format: 13 wide by however many necessary deep 

Sint8 level0_blocks[] = {
    GLD, GLD, GLD, GLD, GLD, GLD, GLD, GLD, GLD, GLD, GLD, GLD, GLD,
    NON, NON, NON, NON, NON, NON, NON, NON, NON, NON, NON, NON, NON,
    RED, RED, RED, RED, RED, RED, RED, RED, RED, RED, RED, RED, RED,
    BGE, GRY, BGE, GRY, BGE, GRY, BGE, GRY, BGE, GRY, BGE, GRY, BGE,
    GRY, BGE, GRY, BGE, GRY, BGE, GRY, BGE, GRY, BGE, GRY, BGE, GRY,
    BGE, ORN, BGE, ORN, BGE, ORN, BGE, ORN, BGE, ORN, BGE, ORN, BGE,
    ORN, BLU, ORN, BLU, ORN, BLU, ORN, BLU, ORN, BLU, ORN, BLU, ORN,
    BLU, WHT, BLU, WHT, BLU, WHT, BLU, WHT, BLU, WHT, BLU, WHT, BLU,
    WHT, PUR, WHT, PUR, WHT, PUR, WHT, PUR, WHT, PUR, WHT, PUR, WHT,
    PUR, PUR, PUR, PUR, PUR, PUR, PUR, PUR, PUR, PUR, PUR, PUR, PUR,
};

Sint8 *level_blocks[] = {level0_blocks};
int level_lengths[] = {(int)sizeof(level0_blocks)};

void level_load(Uint16 base, int num) {
    block_base = base;
    //set row cursor to bottom row
    row_num = (level_lengths[num] / LEVEL_WIDTH) - 1;
    anim_cursor = -1;
    block_cursor = 0;
}

void level_disp() {
    SPRITE_INFO spr;

    //scroll each row down, then move to the next row until we've put all rows onscreen
    if (row_num >= 0) {
        //if current row hasn't yet been loaded, load it
        if (anim_cursor == -1) {
            anim_cursor = 0;
            for (int i = 0; i < LEVEL_WIDTH; i++) {
                Sint8 tile_num = level0_blocks[row_num * LEVEL_WIDTH + i];
                if (tile_num >= 0) {
                    BLOCK_SPR *block = &anim_row[anim_cursor++];
                    block->x = MTH_Mul(MTH_FIXED(i), BLOCK_WIDTH) + LEFT_WALL;
                    block->y = 0;
                    block->tile_no = tile_num + block_base;
                }
            }
            row_y = 0;
        }
        //otherwise move the row down
        row_y += MTH_FIXED(3);
        if (row_y >= MTH_Mul(MTH_FIXED(row_num), BLOCK_HEIGHT)) {
            row_y = MTH_Mul(MTH_FIXED(row_num), BLOCK_HEIGHT);
            //copy from row arr to main arr
            for (int i = 0; i < anim_cursor; i++) {
                anim_row[i].y = row_y;
                block_arr[block_cursor++] = anim_row[i];
            }
            row_num--;
            anim_cursor = -1;
        }
        else {
            for (int i = 0; i < anim_cursor; i++) {
                anim_row[i].y = row_y;
                sprite_make(anim_row[i].tile_no, anim_row[i].x, anim_row[i].y, &spr);
                sprite_draw(&spr);
            }
        }
    }
    print_num(block_cursor, 0, 0);

    //draw all the blocks in the block arr
    for (int i = 0; i < block_cursor; i++) {
        sprite_make(block_arr[i].tile_no, block_arr[i].x, block_arr[i].y, &spr);
        sprite_draw(&spr);
    }
}
