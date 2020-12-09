#include <SEGA_DMA.H>
#include <SEGA_MTH.H>
#include <SEGA_SCL.H>
#define _SPR2_
#include <SEGA_SPR.H>

#include "ball.h"
#include "capsule.h"
#include "cd.h"
#include "game.h"
#include "graphicrefs.h"
#include "laser.h"
#include "level.h"
#include "print.h"
#include "release.h"
#include "scroll.h"
#include "sound.h"
#include "sprite.h"
#include "spritecode/explosion.h"
#include "vblank.h"

typedef enum {
    STATE_GAME_INIT = 0,
    STATE_GAME_FADEIN,
    STATE_GAME_PLAY,
    STATE_GAME_LOSS,
} GAME_STATE;

static int state = STATE_GAME_INIT;
static int frames = 0;

//nbg0 tilemap width
#define MAP_WIDTH (64)

//# tiles per row in the 
#define TILES_WIDTH (18)

static int score = 0;
//character number for score text
#define SCORE_CHARNO ((((30 * TILES_WIDTH) + 2) * 2) + SCROLL_A1_OFFSET)
//where score is on HUD
#define SCORE_AREA ((2 * MAP_WIDTH) + 33)
#define SCORE_DIGITS (7)

#define START_LIVES (5)
static int lives = START_LIVES; //how many extra lives you have
//character num for life icon
#define LIFE_CHARNO (((30 * TILES_WIDTH) * 2) + SCROLL_A1_OFFSET)
//where life area is on HUD
#define LIFE_AREA ((26 * MAP_WIDTH) + 34)

typedef enum {
    P_NONE = 0,
    P_TURBO,
    P_BIT,
    P_DISRUPTION,
    P_LASER,
    P_ILLUSION,
    P_GIGABALL,
    P_BARRIER
} POWERUP;

#define NUM_POWERUPS (P_BARRIER)

#define POWERUP_EMPTY_CHARNO ((((21 * TILES_WIDTH) + 4) * 2) + SCROLL_A1_OFFSET)
#define POWERUP_FULL_CHARNO ((((26 * TILES_WIDTH) + 4) * 2) + SCROLL_A1_OFFSET)
#define POWERUP_AREA ((21 * MAP_WIDTH) + 30)

//which powerup you have selected in the HUD
int powerup_cursor = P_NONE;

// --- turbo powerup ---
#define MAX_TURBO (3)
int turbo = 0; // turbo speed
#define TURBO_CHARNO ((((34 * TILES_WIDTH) + 7) * 2) + SCROLL_A1_OFFSET)
// number of characters in the "TURBO" hud text
#define TURBO_TEXTLEN (5)
#define TURBO_AREA ((27 * MAP_WIDTH) + 2)

// --- bit powerup ---
#define BIT_WIDTH (MTH_FIXED(9))
int bit_ship = 0; // 1 if bit is enabled
SPRITE_INFO *bit_left = NULL;
SPRITE_INFO *bit_right = NULL;

// --- laser powerup ---
#define MAX_LASER_MAX (8)
int laser_max = 0;

// --- illusion powerup ---
// 0 if off, 1 if on
int illusion = 0;
// number of trailing ships
#define ILLUSION_NUM (3)
// number of frames between each ship
#define ILLUSION_FRAMEDELAY (4)
// number of frames to keep track of
#define ILLUSION_ARRLEN (ILLUSION_NUM * ILLUSION_FRAMEDELAY)
// struct for illusion buffer
typedef struct {
    Fixed32 x;
    Fixed32 y;
} ILLUSION_SHIP;

int illusion_charno = 0;
int illusion_cursor = 0; // cursor in illusion array (ring buffer)
ILLUSION_SHIP illusion_arr[ILLUSION_ARRLEN];

//tile numbers for the powerup names
Uint32 powerup_names[] = {
    ((31 * TILES_WIDTH) * 2) + SCROLL_A1_OFFSET,
    (((31 * TILES_WIDTH) + 7) * 2) + SCROLL_A1_OFFSET,
    ((32 * TILES_WIDTH) * 2) + SCROLL_A1_OFFSET,
    (((32 * TILES_WIDTH) + 7) * 2) + SCROLL_A1_OFFSET,
    ((33 * TILES_WIDTH) * 2) + SCROLL_A1_OFFSET,
    (((33 * TILES_WIDTH) + 7) * 2) + SCROLL_A1_OFFSET,
    ((34 * TILES_WIDTH) * 2) + SCROLL_A1_OFFSET,
};

#define POWERUP_NAME_AREA ((23 * MAP_WIDTH) + 35)
#define POWERUP_NAME_LEN (7)

typedef enum {
    STATE_CHIP_NONE = 0,
    STATE_CHIP_HAND,
    STATE_CHIP_BURNT,
    STATE_CHIP_SBLINK,
    STATE_CHIP_DBLINK
} CHIP_STATE;

static int chip_state = STATE_CHIP_NONE;
static int chip_frames = 0;
static int chip_cursor;

#define CHIP_XPIXELS (96)
#define CHIP_YPIXELS (80)
#define CHIP_XTILES (CHIP_XPIXELS / 8)
#define CHIP_YTILES (CHIP_YPIXELS / 8)
#define IMAGE_XTILES (18)

#define CHIPFRAME_SIZE (96 * 80)
#define CHIP_HAND_NUMFRAMES (4)
static int chip_hand_frames[CHIP_HAND_NUMFRAMES] = {1, 2, 3, 4};
static int chip_hand_timings[CHIP_HAND_NUMFRAMES] = {7, 6, 6, 35};

#define CHIP_BURNT_NUMFRAMES (6)
static int chip_burnt_frames[CHIP_BURNT_NUMFRAMES] = {7, 8, 7, 8, 7, 8};
static int chip_burnt_timings[CHIP_BURNT_NUMFRAMES] = {11, 6, 11, 6, 11, 6};

#define CHIP_BLINK_NUMFRAMES (3)
static int chip_blink_frames[CHIP_BLINK_NUMFRAMES] = {5, 6, 0};
static int chip_blink_timings[CHIP_BLINK_NUMFRAMES] = {11, 4, 4};
static int chip_blinktimer;

#define SHIP_CHARNO (font_num)
//start of ship's idle animation
#define SHIP_IDLE (SHIP_CHARNO + 13)

SHIP_SPRITE ship_sprite;
//ship's leftmost pixel
Fixed32 ship_left;
//ship's rightmost pixel
Fixed32 ship_right;
//ship's y pos during normal gameplay
#define SHIP_POS (MTH_FIXED(200))
#define SHIP_SPEED (MTH_FIXED(4))
//all the ship's animations take 6 frames
#define SHIP_TIMING (6)
static int ship_frames;
//where ship starts a new life
#define SHIP_STARTX (MTH_FIXED(102))
#define SHIP_STARTY (MTH_FIXED(240))

// character numbers
int bit_charno;
int laser_charno;
int ball_charno;
int explosion_charno;
int block_charno;
int capsule_charno;

static inline void game_init() {
    Uint8 *game_buf = (Uint8 *)LWRAM;
    //blank display
    scroll_lores();
    //set nbg0 to a 8x8 tilemap
    scroll_charsize(0, SCL_CHAR_SIZE_1X1);
    scroll_mapsize(0, SCL_PN2WORD);
    //fade out game
    SCL_SetColOffset(SCL_OFFSET_A, SCL_SPR | SCL_NBG0 | SCL_NBG1 | SCL_NBG2 | SCL_NBG3, -255, -255, -255);
    //set sprite priority
    SCL_SetPriority(SCL_SPR,  7);
    //wipe tilemaps
    scroll_clearmaps();
    //reset bg positions
    scroll_set(0, MTH_FIXED(0), MTH_FIXED(0));
    scroll_set(1, MTH_FIXED(0), MTH_FIXED(0));
    scroll_set(2, MTH_FIXED(0), MTH_FIXED(0));
    scroll_set(3, MTH_FIXED(0), MTH_FIXED(0));
    //load tiles for hud
    cd_load_nosize(game_tiles_name, game_buf);
    //write them to the screen
    char *tile_ptr = (char *)SCL_VDP2_VRAM_A1;
    DMA_CpuMemCopy1(tile_ptr, game_buf, game_tiles_num * 64);
    //load palette for hud
    SCL_SetColRam(SCL_NBG0, 0, 256, game_tiles_pal);
    //load map
    cd_load_nosize(game_name, game_buf);
    Uint32 *game_map = MAP_PTR32(0);
    Uint32 *map_buf = (Uint32 *)LWRAM;
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 64; j++) {
            if (i < game_height && j < game_width) {
                game_map[i * MAP_WIDTH + j] = SCROLL_A1_OFFSET + map_buf[i * game_width + j];
            } 
            else {
                game_map[i * MAP_WIDTH + j] = 0;
            }
        }
    }
    //load tiles for stars
    cd_load_nosize(gamestar_name, game_buf);
    //write them to screen
    tile_ptr = (char *)SCL_VDP2_VRAM_B1;
    DMA_CpuMemCopy1(tile_ptr, game_buf, gamestar_num * 128);
    //load palette for stars
    SCL_SetColRam(SCL_NBG1, 0, 16, gamestar_pal);
    //load maps
    cd_load_nosize(gamenear_name, game_buf);
    DMA_CpuMemCopy1(MAP_PTR(1), game_buf, 32 * 32 * 2);
    cd_load_nosize(gamemid_name, game_buf);
    DMA_CpuMemCopy1(MAP_PTR(2), game_buf, 32 * 32 * 2);
    cd_load_nosize(gamefar_name, game_buf);
    DMA_CpuMemCopy1(MAP_PTR(3), game_buf, 32 * 32 * 2);
    //fade in
    SclRgb start, end;
    start.red = start.green = start.blue = -255;
    end.red = end.green = end.blue = 0;
    SCL_SetAutoColOffset(SCL_OFFSET_A, 1, 30, &start, &end);
    //load sprite frames
    cd_load_nosize(ship_name, game_buf);
    SCL_SetColRam(SCL_SPR, 16, 16, ship_pal);
    SPR_2ClrAllChar();
    //load font
    print_load();

    Uint16 spr_charno = SHIP_CHARNO;
    Uint16 spr_palno = 16;
    //load ship sprites
    for (int i = 0; i < ship_num; i++) {
        SPR_2SetChar(i + spr_charno, COLOR_0, spr_palno, ship_width, ship_height, (char *)game_buf + (i * ship_size));
    }
    spr_charno += ship_num;
    // load one extra ship sprite with 1/2 transparency enabled for illusion
    SPR_2SetChar(spr_charno, COLOR_0, spr_palno | (1 << 11) | (1 << 12), ship_width, ship_height, 
        (char *)game_buf + ((SHIP_IDLE - SHIP_CHARNO) * ship_size));
    illusion_charno = spr_charno;
    spr_charno++;
    spr_palno += 16;

    //load bit (powerup for ship width)
    bit_charno = spr_charno;
    cd_load_nosize(bit_name, game_buf);
    SCL_SetColRam(SCL_SPR, spr_palno, 16, bit_pal);
    for (int i = 0; i < bit_num; i++) {
        SPR_2SetChar(i + spr_charno, COLOR_0, spr_palno, bit_width, bit_height, (char *)game_buf + (i * bit_size));
    }
    spr_charno += bit_num;
    spr_palno += 16;

    //load ship's laser
    laser_charno = spr_charno;
    cd_load_nosize(laser_name, game_buf);
    SCL_SetColRam(SCL_SPR, spr_palno, 16, laser_pal);
    for (int i = 0; i < laser_num; i++) {
        SPR_2SetChar(i + spr_charno, COLOR_0, spr_palno, laser_width, laser_height, (char *)game_buf + (i * laser_size));
    }
    spr_charno += laser_num;
    spr_palno += 16;

    //load ball
    ball_charno = spr_charno;
    cd_load_nosize(ball_name, game_buf);
    SCL_SetColRam(SCL_SPR, spr_palno, 16, ball_pal);
    for (int i = 0; i < ball_num; i++) {
        SPR_2SetChar(i + spr_charno, COLOR_0, spr_palno, ball_width, ball_height, (char *)game_buf + (i * ball_size));
    }
    spr_charno += ball_num;
    spr_palno += 16;

    //load ship explosion frames
    explosion_charno = spr_charno;
    cd_load_nosize(explosion_name, game_buf);
    SCL_SetColRam(SCL_SPR, spr_palno, 16, explosion_pal);
    for (int i = 0; i < explosion_num; i++) {
        SPR_2SetChar(i + spr_charno, COLOR_0, spr_palno, explosion_width, explosion_height, (char *)game_buf + (i * explosion_size));
    }
    spr_charno += explosion_num;
    spr_palno += 16;

    //load block effects (explosion & shine)
    block_charno = spr_charno;
    cd_load_nosize(beffect_name, game_buf);
    SCL_SetColRam(SCL_SPR, spr_palno, 16, beffect_pal);
    for (int i = 0; i < beffect_num; i++) {
        SPR_2SetChar(i + spr_charno, COLOR_0, spr_palno, beffect_width, beffect_height, (char *)game_buf + (i * beffect_size));
    }
    spr_charno += beffect_num;
    spr_palno += 16;

    //load blocks
    cd_load_nosize(blocks1_name, game_buf);
    SCL_SetColRam(SCL_SPR, spr_palno, 16, blocks1_pal);
    for (int i = 0; i < blocks1_num; i++) {
        SPR_2SetChar(i + spr_charno, COLOR_0, spr_palno, blocks1_width, blocks1_height, (char *)game_buf + (i * blocks1_size));
    }
    spr_charno += blocks1_num;
    spr_palno += 16;
    cd_load_nosize(blocks2_name, game_buf);
    SCL_SetColRam(SCL_SPR, spr_palno, 16, blocks2_pal);
    for (int i = 0; i < blocks2_num; i++) {
        SPR_2SetChar(i + spr_charno, COLOR_0, spr_palno, blocks2_width, blocks2_height, (char *)game_buf + (i * blocks2_size));
    }
    spr_charno += blocks2_num;
    spr_palno += 16;

    //load powerup capsule
    capsule_charno = spr_charno;
    cd_load_nosize(capsule_name, game_buf);
    SCL_SetColRam(SCL_SPR, spr_palno, 16, capsule_pal);
    for (int i = 0; i < capsule_num; i++) {
        SPR_2SetChar(i + spr_charno, COLOR_0, spr_palno, capsule_width, capsule_height, (char *)game_buf + (i * capsule_size));
    }
    spr_charno += capsule_num;
    spr_palno += 16;

    //load chip's animation frames into lwram
    cd_load_nosize(chipgame_name, game_buf);
    sound_cdda(5, 1);
}

static void game_loadchip(int num) {
    char *frame_ptr = ((char *)LWRAM) + (num * (CHIP_XPIXELS * CHIP_YPIXELS));
    char *tile_ptr = (char *)SCL_VDP2_VRAM_A1 + ((64 * IMAGE_XTILES) * 4) + (64 * 4); //start at 6 rows + 6 tiles in

    int offset;
    for (int i = 0; i < CHIP_YTILES; i++) {
        offset = i * (64 * IMAGE_XTILES);
        for (int j = 0; j < 64 * CHIP_XTILES; j++) {
            tile_ptr[offset++] = frame_ptr[j + (i * 64 * CHIP_XTILES)];
        }
    }
}

static void game_chipset(int state) {
    chip_cursor = 0;
    chip_frames = 0;
    chip_state = state;
    switch(state) {
        case STATE_CHIP_NONE:
            game_loadchip(0);
            break;

        case STATE_CHIP_HAND:
            game_loadchip(chip_hand_frames[0]);
            break;

        case STATE_CHIP_BURNT:
            game_loadchip(chip_burnt_frames[0]);
            break;

        case STATE_CHIP_SBLINK:
        case STATE_CHIP_DBLINK:
            game_loadchip(chip_blink_frames[0]);
            break;
    }
}

static void game_chipanim() {
    switch (chip_state) {
        case STATE_CHIP_HAND: //chip does her hand thing here
            chip_frames++;
            if (chip_frames >= chip_hand_timings[chip_cursor]) {
                chip_frames = 0;
                chip_cursor++;
                if (chip_cursor >= CHIP_HAND_NUMFRAMES) {
                    game_chipset(STATE_CHIP_NONE);
                    break;
                }
                game_loadchip(chip_hand_frames[chip_cursor]);
            }
            break;

        case STATE_CHIP_BURNT:
            chip_frames++;
            if (chip_frames >= chip_burnt_timings[chip_cursor]) {
                chip_frames = 0;
                chip_cursor++;
                if (chip_cursor >= CHIP_BURNT_NUMFRAMES) {
                    chip_state = STATE_CHIP_NONE; //we want to leave chip on the burnt frame
                    break;
                }
                game_loadchip(chip_burnt_frames[chip_cursor]);
            }
        break;

        case STATE_CHIP_SBLINK:
        case STATE_CHIP_DBLINK:
            chip_frames++;
            if (chip_frames >= chip_blink_timings[chip_cursor]) {
                chip_frames = 0;
                chip_cursor++;
                if (chip_cursor >= CHIP_BLINK_NUMFRAMES) {
                    if (chip_state == STATE_CHIP_DBLINK) {
                        game_chipset(STATE_CHIP_SBLINK);
                    }
                    else {
                        game_chipset(STATE_CHIP_NONE);
                    }
                    break;
                }
                game_loadchip(chip_blink_frames[chip_cursor]);
            }
            break;
    }
}

static inline int game_setblinktimer() {
    return 180 + (MTH_GetRand() & 0x1f);
}

static void game_shipanim() {
    //-----ship animation stuff (keeping it stateless helps simplify things)
    //move ship when it spawns
    if (ship_sprite.y > SHIP_POS) {
        ship_sprite.y -= MTH_FIXED(1.5);
    }
    else {
        ship_sprite.y = SHIP_POS;
    }
    //animate ship
    ship_frames++;
    if (ship_frames == SHIP_TIMING) {
        ship_frames = 0;
        ship_sprite.char_num++;
        // reset to idle animation
        if (ship_sprite.char_num == SHIP_CHARNO + ship_num) {
            ship_sprite.char_num = SHIP_IDLE;
            ship_sprite.state = SHIP_STATE_NORM;
        }
    }
}

void game_incpowerup() {
    powerup_cursor++;
    if (powerup_cursor > NUM_POWERUPS) { // handle wraparound
        powerup_cursor = P_TURBO;
    }
}

// reset powerup state
void game_powerupreset() {
    // reset powerup meter
    powerup_cursor = 0;

    // no turbo
    turbo = 0;

    // no bit
    bit_ship = 0;
    // remove bit sprites
    if (bit_left != NULL) {
        sprite_delete(bit_left);
        bit_left = NULL;
    }
    if (bit_right != NULL) {
        sprite_delete(bit_right);
        bit_right = NULL;
    }
    // no laser
    laser_max = 0;
    // no illusion
    illusion = 0;
}

void game_loss() {
    game_chipset(STATE_CHIP_BURNT);
    frames = 0;
    state = STATE_GAME_LOSS;
    explosion_make(explosion_charno, ship_sprite.x, ship_sprite.y);
    game_powerupreset();
    ship_sprite.x = SHIP_STARTX;
    ship_sprite.y = SHIP_STARTY;
}

int game_run() {
    // for illusion
    int top_index = 0;
    int mid_index = 0;
    int bot_index = 0;

    switch (state) {
        case STATE_GAME_INIT:
            game_init();
            game_chipset(STATE_CHIP_NONE);
            chip_blinktimer = game_setblinktimer();
            frames = 0;
            //init lives
            lives = START_LIVES;
            //init score
            score = 42069;
            //init the ship sprite
            ship_sprite.char_num = SHIP_CHARNO;
            ship_sprite.x = SHIP_STARTX;
            ship_sprite.y = SHIP_STARTY;
            ship_frames = 0;
            ship_sprite.state = SHIP_STATE_INIT;
            //init the ball handler
            ball_init(ball_charno);
            //init the capsule handler
            capsule_init(capsule_charno);
            // init laser handler
            laser_init(laser_charno);
            //add first ball
            ball_add(ship_sprite.x, ship_sprite.y, MTH_FIXED(45));
            //init powerup state
            game_powerupreset();
            level_load(block_charno, 0);

            state = STATE_GAME_FADEIN;
            break;
        
        case STATE_GAME_FADEIN:
            frames++;
            if (frames > 90) {
                game_chipset(STATE_CHIP_HAND);
                frames = 0;
                state = STATE_GAME_PLAY;
            }
            break;
        
        case STATE_GAME_PLAY:
            //handle chip animation stuff
            frames++;
            if (frames == chip_blinktimer) {
                frames = 0;
                chip_blinktimer = game_setblinktimer();
                //chip blinks twice 1/4 of the time
                if (((MTH_GetRand() >> 8) & 0xf) < 4) {
                    game_chipset(STATE_CHIP_DBLINK);
                }
                else {
                    game_chipset(STATE_CHIP_SBLINK);
                }
            }
            //handle input
            Fixed32 speed = SHIP_SPEED;
            if ((PadData1E & PAD_A) && (laser_count < laser_max)) {
                laser_add(ship_sprite.x + SHIP_LGUN, ship_sprite.y + SHIP_GUNY);
                laser_add(ship_sprite.x + SHIP_RGUN, ship_sprite.y + SHIP_GUNY);
            }

            if (PadData1 & PAD_C) {
                speed += MTH_IntToFixed(turbo);
            }

            if (PadID1 == ANALOGPAD_ID) {
                ship_sprite.dx = 0;
                //shoulder trigger controls
                Fixed32 left_trigger = MTH_Mul((PadAnalogL1 + 1) << 8, speed);
                ship_sprite.dx -= left_trigger;
                Fixed32 right_trigger = MTH_Mul((PadAnalogR1 + 1) << 8, speed);
                ship_sprite.dx += right_trigger;

                //analog stick controls
                //transform x pos by converting the 0-255 range of the analog x pos to be
                //MTH_FIXED(0) to MTH_FIXED(1), multiplying it by twice the max ship speed
                //(range: MTH_FIXED(0) to SHIP_SPEED * 2) and then subtracting SHIP_SPEED
                //(range: -SHIP_SPEED to SHIP_SPEED)
                Fixed32 movement = MTH_Mul(PadAnalogX1 << 8, (speed + MTH_FIXED(0.15)) << 1) - (speed + MTH_FIXED(0.15));
                ship_sprite.dx += movement;
                //limit speed
                if (ship_sprite.dx > speed) ship_sprite.dx = speed;
                if (ship_sprite.dx < -speed) ship_sprite.dx = -speed;
            }
            else {
                ship_sprite.dx = 0;
                if (PadData1 & PAD_L) ship_sprite.dx -= speed;
                if (PadData1 & PAD_R) ship_sprite.dx += speed;
            }
            ship_sprite.x += ship_sprite.dx;

            while (1) {
                // bit powerup gives ship extra width
                if (bit_ship) {
                    bit_left->x = ship_sprite.x - BIT_WIDTH;
                    bit_left->y = ship_sprite.y + SHIP_YMARGIN;
                    bit_right->x = ship_sprite.x + SHIP_WIDTH;
                    bit_right->y = ship_sprite.y + SHIP_YMARGIN;
                    ship_left = bit_left->x;
                    ship_right = bit_right->x + BIT_WIDTH;
                }
                else {
                    ship_left = ship_sprite.x + SHIP_XMARGIN;
                    ship_right = ship_sprite.x + (SHIP_WIDTH - SHIP_XMARGIN);
                }

                //ship boundaries
                if (ship_left < LEFT_WALL) {
                    ship_sprite.x += MTH_FIXED(1);
                }
                else if (ship_right > RIGHT_WALL) {
                    ship_sprite.x -= MTH_FIXED(1);
                }
                else {
                    break;
                }
            }
            // trail for illusion powerup
            illusion_arr[illusion_cursor].x = ship_sprite.x;
            illusion_arr[illusion_cursor].y = ship_sprite.y;
            illusion_cursor++;
            if (illusion_cursor >= ILLUSION_ARRLEN) {
                illusion_cursor = 0;
            }
            // calculate array indexes for each trailing ship, as well as "true" ship_left/ship_right
            if (illusion) {
                top_index = illusion_cursor - ILLUSION_FRAMEDELAY; // top
                if (top_index < 0) { top_index += ILLUSION_ARRLEN; }
                mid_index = top_index - ILLUSION_FRAMEDELAY;       // middle
                if (mid_index < 0) { mid_index += ILLUSION_ARRLEN; }
                bot_index = mid_index - ILLUSION_FRAMEDELAY;       // bottom
                if (bot_index < 0) { bot_index += ILLUSION_ARRLEN; }
                // left
                ship_left = MIN(ship_left, illusion_arr[top_index].x);
                ship_left = MIN(ship_left, illusion_arr[mid_index].x);
                ship_left = MIN(ship_left, illusion_arr[bot_index].x);
                // right
                ship_right = MAX(ship_right, illusion_arr[top_index].x + (SHIP_WIDTH - SHIP_XMARGIN));
                ship_right = MAX(ship_right, illusion_arr[mid_index].x + (SHIP_WIDTH - SHIP_XMARGIN));
                ship_right = MAX(ship_right, illusion_arr[bot_index].x + (SHIP_WIDTH - SHIP_XMARGIN));

            }

            //move all balls on screen
            ball_move();
            game_shipanim();
            break;
        
        //when player has lost, show burnt chip for a second
        //in order to make the player think about what he's done
        case STATE_GAME_LOSS:
            frames++;
            if (frames >= 60) {
                lives--;
                //TODO add game over code here
                game_chipset(STATE_CHIP_NONE);
                ship_sprite.char_num = SHIP_CHARNO;
                ship_sprite.state = SHIP_STATE_INIT;
                ball_add(ship_sprite.x, ship_sprite.y, MTH_FIXED(135));
                state = STATE_GAME_PLAY;
            }
        break;

    }
    //update score
    Uint32 *score_ptr = MAP_PTR32(0) + SCORE_AREA;
    int temp_score = score;
    if (temp_score > 9999999) {
        temp_score = 9999999;
    }
    for (int i = 0; i < SCORE_DIGITS; i++) {
        score_ptr[SCORE_DIGITS - i] = ((temp_score % 10) << 1) + SCORE_CHARNO;
        temp_score /= 10;
    }

    //update lives count
    Uint32 *life_ptr = MAP_PTR32(0) + LIFE_AREA;
    for (int i = 0; i < 16; i += 2) {
        Uint32 *curr_ptr = life_ptr;
        //move to second row if we've passed the first row
        if (i >= 8) {
            curr_ptr += (MAP_WIDTH - 8);
        }
        //show life icon
        if ((i >> 1) < lives) {
            curr_ptr[i] = LIFE_CHARNO;
            curr_ptr[i + 1] = LIFE_CHARNO + 2;
        }
        //clear life icon
        else {
            curr_ptr[i] = SCROLL_A1_OFFSET;
            curr_ptr[i + 1] = SCROLL_A1_OFFSET;
        }
    }

    //update powerups
    #if DEBUG
    if (PadData1E & PAD_X) {
        game_incpowerup();
    }
    #endif

    if (PadData1E & PAD_B) {
        switch (powerup_cursor) {
            case P_TURBO:
                if (turbo < MAX_TURBO) {
                    turbo++;
                    powerup_cursor = P_NONE;
                }
                break;
            
            case P_BIT:
                if (bit_ship == 0) {
                    // can't have bit and illusion on simultaneously
                    illusion = 0;
                    bit_ship = 1;
                    bit_left = sprite_next();
                    sprite_make(bit_charno, MTH_FIXED(-80), MTH_FIXED(-80), bit_left);
                    bit_right = sprite_next();
                    sprite_make(bit_charno, MTH_FIXED(-80), MTH_FIXED(-80), bit_right);
                    powerup_cursor = P_NONE;
                }
                break;

            case P_DISRUPTION:
                for (int i = 0; i < 5; i++) {
                    // range from 60 to 120 degrees
                    Fixed32 angle = MTH_GetRand() % MTH_FIXED(60);
                    angle += MTH_FIXED(60);
                    ball_add(ship_sprite.x + (SHIP_WIDTH >> 1), ship_sprite.y, angle);
                }
                powerup_cursor = P_NONE;
                break;

            case P_LASER:
                if (laser_max < MAX_LASER_MAX) {
                    laser_max += 4;
                    powerup_cursor = P_NONE;
                }
                break;

            case P_ILLUSION:
                if (illusion == 0) {
                    // can't have bit and illusion on simultaneously
                    bit_ship = 0;
                    if (bit_left != NULL) {
                        sprite_delete(bit_left);
                        bit_left = NULL;
                    }
                    if (bit_right != NULL) {
                        sprite_delete(bit_right);
                        bit_right = NULL;
                    }
                    illusion = 1;
                    powerup_cursor = P_NONE;
                }
                break;
        }
    }
    // draw powerup on HUD
    Uint32 *powerup_ptr = MAP_PTR32(0) + POWERUP_AREA;
    for (int i = 0; i < NUM_POWERUPS; i++) {
        if ((powerup_cursor - 1) == i) {
            powerup_ptr[(NUM_POWERUPS - 1 - i) * MAP_WIDTH] = POWERUP_FULL_CHARNO;
            powerup_ptr[((NUM_POWERUPS -1 - i) * MAP_WIDTH) + 1] = POWERUP_FULL_CHARNO + 2;

        }
        else {
            powerup_ptr[(NUM_POWERUPS - 1 - i) * MAP_WIDTH] = POWERUP_EMPTY_CHARNO;
            powerup_ptr[((NUM_POWERUPS - 1 - i) * MAP_WIDTH) + 1] = POWERUP_EMPTY_CHARNO + 2;
        }
    }
    powerup_ptr = MAP_PTR32(0) + POWERUP_NAME_AREA;
    for (int i = 0; i < POWERUP_NAME_LEN; i++) {
        if (powerup_cursor == P_NONE) {
            powerup_ptr[i] = SCROLL_A1_OFFSET;
        }
        else {
            powerup_ptr[i] = powerup_names[powerup_cursor - 1] + (i << 1);
        }
    }
    // make powerups impact game state
    Uint32 *turbo_ptr = MAP_PTR32(0) + TURBO_AREA;
    if (turbo) {
        for (int i = 0; i < TURBO_TEXTLEN + turbo; i++) {
            turbo_ptr[i] = TURBO_CHARNO + ((i < TURBO_TEXTLEN ? i : TURBO_TEXTLEN) << 1);
        }

    }
    else { // clear the text and graphics otherwise
        for (int i = 0; i < TURBO_TEXTLEN + MAX_TURBO; i++) {
            turbo_ptr[i] = SCROLL_A1_OFFSET;
        }
    }

    game_chipanim(); // draw chip
    level_disp(); // draw blocks
    capsule_run(); // draw capsules
    laser_move(); // draw laser

    // draw ship
    SPRITE_INFO spr;
    // if illusion is active, draw those ships back to front
    if (illusion) {
        // bottom
        sprite_make(illusion_charno, illusion_arr[bot_index].x, illusion_arr[bot_index].y, &spr);
        sprite_draw(&spr);
        // middle
        sprite_make(illusion_charno, illusion_arr[mid_index].x, illusion_arr[mid_index].y, &spr);
        sprite_draw(&spr);
        // top
        sprite_make(illusion_charno, illusion_arr[top_index].x, illusion_arr[top_index].y, &spr);
        sprite_draw(&spr);
    }
    sprite_make(ship_sprite.char_num, ship_sprite.x, ship_sprite.y, &spr);
    sprite_draw(&spr);

    // move star bg
    scroll_move(1, MTH_FIXED(0), MTH_FIXED(-4));
    scroll_move(2, MTH_FIXED(0), MTH_FIXED(-2));
    scroll_move(3, MTH_FIXED(0), MTH_FIXED(-1));
    return 0;
}
