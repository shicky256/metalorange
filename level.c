#include <sega_xpt.h>

#include "ball.h"
#include "capsule.h"
#include "circle.h"
#include "game.h"
#include "laser.h"
#include "level.h"
#include "print.h"
#include "sound.h"
#include "sprite.h"

#define EXPLOSION_CHARNO (0)
#define EXPLOSION_NUM (3)
//number of frames between each explosion frame 
#define EXPLOSION_TIMING (6)

#define SHINE_CHARNO (EXPLOSION_NUM)
#define SHINE_NUM (5)
#define SHINE_TIMING (6)


#define BLOCK_START (EXPLOSION_NUM + SHINE_NUM)
typedef enum {
    NON = -1,
    RED = BLOCK_START, //red
    BGE, //beige
    GRY, //gray
    GLD, //gold
    BLU, //blue
    ORN, //orange
    WHT, //white
    PUR, //purple
    CR1, //cracked 1 (before it's cracked)
    CR2, //cracked 2 (actually cracked)
} BLOCK;

typedef enum {
    STATE_NORM = 0,
    STATE_EXPLODE,
    STATE_SHINEON, //shine flickers so use two states to represent this
    STATE_SHINEOFF
} BLOCK_STATE;

typedef struct {
    Fixed32 x; //x position onscreen
    Fixed32 y; //y position onscreen
    Uint16 tile_no; //tile number
    Uint16 overlay_no; //tile number of overlay sprite
    int index; //where it is in the block array
    int anim_timer;
    int state;
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
// level currently loaded
static int current_level;

// timer for spawning circle
#define CIRCLE_SPAWNRATE (1200)
static int circle_timer;
#define CIRCLE_XPOS1 (MTH_FIXED(20))
#define CIRCLE_XPOS2 (MTH_FIXED(180))
static Fixed32 circle_xpos;

// number of blocks left on the stage
int level_blocksleft;

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

Sint8 level1_blocks[] = {
    NON, NON, NON, NON, NON, NON, NON, NON, NON, NON, NON, NON, NON,
    NON, NON, NON, NON, NON, NON, NON, NON, NON, NON, NON, NON, NON,
    GRY, GRY, GRY, GRY, GRY, GRY, GRY, GRY, GRY, GRY, GRY, GRY, GRY,
    BGE, BGE, BGE, BGE, BGE, BGE, BGE, BGE, BGE, BGE, BGE, BGE, BGE,
    WHT, WHT, WHT, WHT, WHT, WHT, WHT, WHT, WHT, WHT, WHT, WHT, WHT,
    NON, NON, NON, NON, NON, NON, NON, NON, NON, NON, NON, NON, NON,
    NON, CR1, BLU, BLU, CR1, BLU, CR1, BLU, CR1, BLU, BLU, CR1, NON,
    NON, CR1, WHT, WHT, CR1, WHT, GLD, WHT, CR1, WHT, WHT, CR1, NON,
    NON, CR1, GLD, CR1, CR1, GRY, CR1, GRY, CR1, CR1, GLD, CR1, NON,
    NON, GRY, GRY, GRY, GRY, GRY, GRY, GRY, GRY, GRY, GRY, GRY, NON,
};


Sint8 *level_blocks[] = {
    level0_blocks,
    level1_blocks,
};

int level_count = sizeof(level_blocks) / sizeof(level_blocks[0]);

int level_lengths[] = {
    (int)sizeof(level0_blocks),
    (int)sizeof(level1_blocks),
};

void level_load(Uint16 base, int num) {
    block_base = base;
    //set row cursor to bottom row
    current_level = num;
    row_num = (level_lengths[num] / LEVEL_WIDTH) - 1;
    anim_cursor = -1;
    block_cursor = 0;
    level_blocksleft = 0;

    if (num == 1) {
        circle_timer = CIRCLE_SPAWNRATE;
        circle_xpos = CIRCLE_XPOS1;
    }
}

static int level_breakable(Sint8 block_num) {
    return block_num != GLD;
}

static void level_addblock(BLOCK_SPR *block) {
    BLOCK_SPR *curr_block = &(block_arr[block_cursor]);
    curr_block->x = block->x;
    curr_block->y = block->y;
    curr_block->tile_no = block->tile_no;
    curr_block->index = block_cursor++;
    curr_block->anim_timer = 0;
    //golden blocks shine when they're added
    if (block->tile_no == GLD) {
        curr_block->state = STATE_SHINEON;
    }
    else {
        curr_block->state = STATE_NORM;
        curr_block->overlay_no = 0;
    }

    if (level_breakable(block->tile_no)) {
        level_blocksleft++;
    }
}

//removes a block from the level block array by swapping the last block and the one to remove
static void level_removeblock(BLOCK_SPR *block) {
    int index = block->index;
    block_cursor--;
    level_blocksleft--;
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

// routine run every frame by normal block
static void level_normalblock(BLOCK_SPR *block) {
    int collision = 0;
    // check all balls
    for (int i = 0; i < ball_count; i++) {
        // left collision
        if (level_pixelinside(block->x, block->y, ball_sprites[i].x + BALL_LSENSORX, ball_sprites[i].y + BALL_LSENSORY)) {
            ball_bounce(&ball_sprites[i], DIR_LEFT, level_breakable(block->tile_no));
            collision = 1;
            break;
        }
        // right collision
        else if (level_pixelinside(block->x, block->y, ball_sprites[i].x + BALL_RSENSORX, ball_sprites[i].y + BALL_RSENSORY)) {
            ball_bounce(&ball_sprites[i], DIR_RIGHT, level_breakable(block->tile_no));
            collision = 1;
            break;
        }
        // top collision
        else if (level_pixelinside(block->x, block->y, ball_sprites[i].x + BALL_TSENSORX, ball_sprites[i].y + BALL_TSENSORY)) {
            ball_bounce(&ball_sprites[i], DIR_UP, level_breakable(block->tile_no));
            collision = 1;
            break;
        }
        // bottom collision
        else if (level_pixelinside(block->x, block->y, ball_sprites[i].x + BALL_BSENSORX, ball_sprites[i].y + BALL_BSENSORY)) {
            ball_bounce(&ball_sprites[i], DIR_DOWN, level_breakable(block->tile_no));
            collision = 1;
            break;
        }

        // check for circle collision
        for (int j = 0; j < circle_cursor; j++) {
            if ((ball_sprites[i].x >= circle_sprites[j].x) && (ball_sprites[i].x < (circle_sprites[j].x + CIRCLE_WIDTH))) {
                if ((ball_sprites[i].y >= circle_sprites[j].y) && (ball_sprites[i].y < (circle_sprites[j].y + CIRCLE_HEIGHT))) {
                    circle_explode(&circle_sprites[j]);
                }
            }
        }
    }
    // check all lasers
    SPRITE_INFO *laser = laser_head;
    while (laser != NULL) {
        // laser is 2px wide
        if ((level_pixelinside(block->x, block->y, laser->x, laser->y)) ||
            (level_pixelinside(block->x, block->y, laser->x + MTH_FIXED(1), laser->y))) {
            laser_remove(laser);
            collision = 1;
            break;
        }

        // check for circle collision
        for (int j = 0; j < circle_cursor; j++) {
            if ((laser->x >= circle_sprites[j].x) && (laser->x < (circle_sprites[j].x + CIRCLE_WIDTH))) {
                if ((laser->y >= circle_sprites[j].y) && (laser->y < (circle_sprites[j].y + CIRCLE_HEIGHT))) {
                    circle_explode(&circle_sprites[j]);
                }
            }
        }
        laser = ((LASER_DATA *)laser->data)->next;
    }

    // if there's a collision, make the block explode
    if (collision) {
        // gold blocks can't be broken
        switch(block->tile_no) {
            case GLD:
                block->state = STATE_SHINEON;
                block->anim_timer = 0;
                sound_play(SOUND_GOLD);
                break;

            case CR1:
                if (block->state == STATE_NORM) {
                    block->state = STATE_SHINEON;
                    block->anim_timer = 0;
                    sound_play(SOUND_CRACK);
                }
                break;

            case CR2:
            // fall through to destroy if the block is done with the "breaking" animation
            if (block->state != STATE_NORM) {
                break;
            }

            default:
                block->state = STATE_EXPLODE;
                block->tile_no = EXPLOSION_CHARNO;
                block->anim_timer = 0;
                score += 50;
                sound_play(SOUND_BLOCK);
                if (((MTH_GetRand() >> 16) % 10) < CAPSULE_PROBABILITY) {
                    capsule_add(block->x, block->y); // drop capsule
                }
                break;
        }
    }

    // check all circles
    for (int i = 0; i < circle_cursor; i++) {
        if (level_pixelinside(block->x, block->y, circle_sprites[i].x + CIRCLE_LSENSORX, circle_sprites[i].y + CIRCLE_LSENSORY)) {
            circle_bounce(&circle_sprites[i], DIR_LEFT);
        }

        if (level_pixelinside(block->x, block->y, circle_sprites[i].x + CIRCLE_RSENSORX, circle_sprites[i].y + CIRCLE_RSENSORY)) {
            circle_bounce(&circle_sprites[i], DIR_RIGHT);
        }

        if (level_pixelinside(block->x, block->y, circle_sprites[i].x + CIRCLE_TSENSORX, circle_sprites[i].y + CIRCLE_TSENSORY)) {
            circle_bounce(&circle_sprites[i], DIR_UP);
        }

        if (level_pixelinside(block->x, block->y, circle_sprites[i].x + CIRCLE_BSENSORX, circle_sprites[i].y + CIRCLE_BSENSORY)) {
            circle_bounce(&circle_sprites[i], DIR_DOWN);
        }
    }
}

//routine run when a block explodes
static int level_explodeblock(BLOCK_SPR *block) {
    block->overlay_no = 0;
    block->anim_timer++;
    if (block->anim_timer >= EXPLOSION_TIMING) {
        block->tile_no++;
        block->anim_timer = 0;
        if (block->tile_no - EXPLOSION_CHARNO >= EXPLOSION_NUM) {
            level_removeblock(block);
            return 1;
        }
    }
    return 0;
}

//routine run to do the shine effect on a block
static void level_shineblock(BLOCK_SPR *block) {
    // for cracked block, show whole block during SHINEON and 
    // cracked block during SHINEOFF
    if (block->tile_no == CR2) {
        block->tile_no = CR1;
    }
    block->anim_timer++;
    block->overlay_no = (block->anim_timer / SHINE_TIMING) + SHINE_CHARNO;
    if ((block->overlay_no - SHINE_CHARNO) >= SHINE_NUM) {
        block->state = STATE_NORM;
        block->overlay_no = 0;
        block->anim_timer = 0;
        if (block->tile_no == CR1) {
            block->tile_no = CR2;
        }
    }
    else {
        block->state = STATE_SHINEOFF;
        if (block->tile_no == CR1) {
            block->tile_no = CR2;
        }
    }
}

int level_doneload() {
    return row_num == -1;
}

void level_disp() {
    SPRITE_INFO spr;

    //scroll each row down, then move to the next row until we've put all rows onscreen
    if (row_num >= 0) {
        //if current row hasn't yet been loaded, load it
        if (anim_cursor == -1) {
            anim_cursor = 0;
            for (int i = 0; i < LEVEL_WIDTH; i++) {
                Sint8 *level_ptr = level_blocks[current_level];
                Sint8 tile_num = level_ptr[row_num * LEVEL_WIDTH + i];
                if (tile_num >= 0) {
                    BLOCK_SPR *block = &anim_row[anim_cursor++];
                    block->x = MTH_Mul(MTH_FIXED(i), BLOCK_WIDTH) + LEFT_WALL + MTH_FIXED(1);
                    block->y = 0;
                    block->tile_no = tile_num;
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
                sprite_make(anim_row[i].tile_no + (Uint16)block_base, anim_row[i].x, anim_row[i].y, &spr);
                sprite_draw(&spr);
            }
        }
    }
    else if (game_playing() && current_level == 1) {
        circle_timer++;
        if (circle_timer >= CIRCLE_SPAWNRATE) {
            circle_timer = 0;
            circle_add(circle_xpos, MTH_FIXED(0));
            if (circle_xpos == CIRCLE_XPOS1) {
                circle_xpos = CIRCLE_XPOS2;
            }
            else {
                circle_xpos = CIRCLE_XPOS1;
            }
        }
    }
    // print_num(block_cursor, 0, 0);

    //draw all the blocks in the block arr
    for (int i = 0; i < block_cursor; i++) {
        switch(block_arr[i].state) {
            case STATE_SHINEOFF:
                block_arr[i].overlay_no = 0;
                block_arr[i].anim_timer++;
                block_arr[i].state = STATE_SHINEON;
                //fall through
            case STATE_NORM:
                level_normalblock(&block_arr[i]);
                break;
    
            case STATE_EXPLODE:
                // if block gets removed from the array, skip to the next block
                if (level_explodeblock(&block_arr[i])) {
                    i--;
                    continue;
                }
                break;

            case STATE_SHINEON:
                level_shineblock(&block_arr[i]);
                level_normalblock(&block_arr[i]);
                break;
        }

        sprite_make(block_arr[i].tile_no + (Uint16)block_base, block_arr[i].x, block_arr[i].y, &spr);
        sprite_draw(&spr);
        if (block_arr[i].overlay_no != 0) {
            sprite_make(block_arr[i].overlay_no + (Uint16)block_base, block_arr[i].x, block_arr[i].y, &spr);
            sprite_draw(&spr);
        }
    }
}
