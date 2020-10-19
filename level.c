#include <SEGA_MTH.H>

#include "ball.h"
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
    Fixed32 x; //x position onscreen
    Fixed32 y; //y position onscreen
    Uint16 tile_no; //tile number
    int index; //where it is in the block array
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

static void level_addblock(BLOCK_SPR *block) {
    BLOCK_SPR *curr_block = &(block_arr[block_cursor]);
    curr_block->x = block->x;
    curr_block->y = block->y;
    curr_block->tile_no = block->tile_no;
    curr_block->index = block_cursor++;
}

//removes a block from the level block array by swapping the last block and the one to remove
static void level_removeblock(BLOCK_SPR *block) {
    int index = block->index;
    block_cursor--;
    //if we're not removing the last block and we have multiple blocks
    if ((block_cursor != index) && (block_cursor > 0)) {
        block_arr[index] = block_arr[block_cursor];
        block_arr[index].index = index;
    }
}

// checks if a given pixel is inside a block or not
static inline int level_pixelinside(Fixed32 block_x, Fixed32 block_y, Fixed32 pixel_x, Fixed32 pixel_y) {
    if ((pixel_x >= block_x) && (pixel_x < block_x + BLOCK_WIDTH) &&
        (pixel_y >= block_y) && (pixel_y < block_y + BLOCK_HEIGHT)) {

        return 1;
    }
    return 0;
}

//routine run every frame by normal block
static void level_normalblock(BLOCK_SPR *block) {
    for (int i = 0; i < ball_count; i++) {
        //left collision
        if (level_pixelinside(block->x, block->y, ball_sprites[i].x + BALL_LSENSORX, ball_sprites[i].y + BALL_LSENSORY)) {
            ball_bounce(&ball_sprites[i], DIR_LEFT);
            level_removeblock(block);
        }
        //right collision
        else if (level_pixelinside(block->x, block->y, ball_sprites[i].x + BALL_RSENSORX, ball_sprites[i].y + BALL_RSENSORY)) {
            ball_bounce(&ball_sprites[i], DIR_RIGHT);
            level_removeblock(block);
        }
        //top collision
        else if (level_pixelinside(block->x, block->y, ball_sprites[i].x + BALL_TSENSORX, ball_sprites[i].y + BALL_TSENSORY)) {
            ball_bounce(&ball_sprites[i], DIR_UP);
            level_removeblock(block);
        }
        //bottom collision
        else if (level_pixelinside(block->x, block->y, ball_sprites[i].x + BALL_TSENSORX, ball_sprites[i].y + BALL_TSENSORY)) {
            ball_bounce(&ball_sprites[i], DIR_DOWN);
            level_removeblock(block);
        }
    }
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
                    block->x = MTH_Mul(MTH_FIXED(i), BLOCK_WIDTH) + LEFT_WALL + MTH_FIXED(1);
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
                level_addblock(&anim_row[i]);
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
        level_normalblock(&block_arr[i]);

        sprite_make(block_arr[i].tile_no, block_arr[i].x, block_arr[i].y, &spr);
        sprite_draw(&spr);
    }
}
