#include <SEGA_DMA.H>
#include <SEGA_MTH.H>
#include <SEGA_SCL.H>
#define _SPR2_
#include <SEGA_SPR.H>

#include "cd.h"
#include "game.h"
#include "graphicrefs.h"
#include "print.h"
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

static int score = 0;
//character number for score text
#define SCORE_CHARNO ((542 * 2) + SCROLL_A1_OFFSET)
//where score is on HUD
#define SCORE_AREA ((2 * 64) + 33)
#define SCORE_DIGITS (7)

#define START_LIVES (5)
static int lives = START_LIVES; //how many extra lives you have
//character num for life icon
#define LIFE_CHARNO ((540 * 2) + SCROLL_A1_OFFSET)
//where life area is on HUD
#define LIFE_AREA ((26 * 64) + 34)

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
static SPRITE_INFO *ship_sprite;
//ship's y pos during normal gameplay
#define SHIP_POS (MTH_FIXED(200))
#define SHIP_SPEED (MTH_FIXED(4))
//all the ship's animations take 6 frames
#define SHIP_TIMING (6)
static int ship_frames;
//the area between where the sprite is and where the actual pixel graphics start
#define SHIP_XMARGIN (MTH_FIXED(4))
#define SHIP_YMARGIN (MTH_FIXED(8))
#define SHIP_WIDTH (MTH_FIXED(40))
#define SHIP_HEIGHT (MTH_FIXED(24))
//where ship starts a new life
#define SHIP_STARTX (MTH_FIXED(102))
#define SHIP_STARTY (MTH_FIXED(240))

//leftmost area of the playfield
#define LEFT_WALL (MTH_FIXED(16))
#define RIGHT_WALL (MTH_FIXED(224))
#define LEFT_BOUND (LEFT_WALL - SHIP_XMARGIN)
#define RIGHT_BOUND (RIGHT_WALL - MTH_FIXED(ship_width) + SHIP_XMARGIN)

#define BALL_CHARNO (SHIP_CHARNO + ship_num)
#define MAX_BALLS (20)
static SPRITE_INFO *ball_sprites[MAX_BALLS];
static int num_balls;
#define BALL_MARGIN (MTH_FIXED(2))
#define BALL_WIDTH (MTH_FIXED(8))
#define BALL_HEIGHT (MTH_FIXED(8))
#define BALL_SPEED (MTH_FIXED(3))
#define BALL_LBOUND (LEFT_WALL - BALL_MARGIN)
#define BALL_RBOUND (RIGHT_WALL - BALL_WIDTH + BALL_MARGIN)
#define BALL_STATE_INIT (0)
#define BALL_STATE_NORMAL (1)
#define BALL_SPAWN_XOFFSET (MTH_FIXED(16))
#define BALL_SPAWN_YOFFSET (MTH_FIXED(-2))


#define EXPLOSION_CHARNO (BALL_CHARNO + ball_num)

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
                game_map[i * 64 + j] = SCROLL_A1_OFFSET + map_buf[i * game_width + j];
            } 
            else {
                game_map[i * 64 + j] = 0;
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
    for (int i = 0; i < ship_num; i++) {
        SPR_2SetChar(i + SHIP_CHARNO, COLOR_0, 16, ship_width, ship_height, (char *)game_buf + (i * ship_size));
    }
    //load ball
    cd_load_nosize(ball_name, game_buf);
    SCL_SetColRam(SCL_SPR, 32, 16, ball_pal);
    for (int i = 0; i < ball_num; i++) {
        SPR_2SetChar(i + BALL_CHARNO, COLOR_0, 32, ball_width, ball_height, (char *)game_buf + (i * ball_size));
    }
    //load ship explosion frames
    cd_load_nosize(explosion_name, game_buf);
    SCL_SetColRam(SCL_SPR, 48, 16, explosion_pal);
    for (int i = 0; i < explosion_num; i++) {
        SPR_2SetChar(i + EXPLOSION_CHARNO, COLOR_0, 48, explosion_width, explosion_height, (char *)game_buf + (i * explosion_size));
    }
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
    if (ship_sprite->y > SHIP_POS) {
        ship_sprite->y -= MTH_FIXED(1.5);
    }
    else {
        ship_sprite->y = SHIP_POS;
    }
    //animate ship
    ship_frames++;
    if (ship_frames == SHIP_TIMING) {
        ship_frames = 0;
        ship_sprite->char_num++;
        // reset to idle animation
        if (ship_sprite->char_num == SHIP_CHARNO + ship_num) {
            ship_sprite->char_num = SHIP_IDLE;
        }
    }
}

static inline void game_addball() {
    for (int i = 0; i < MAX_BALLS; i++) {
        if (ball_sprites[i] == NULL) {
            ball_sprites[i] = sprite_next();
            num_balls++;
            //if spawning the first ball, attach it to the ship
            if (num_balls == 1) {
                //spawn ball offscreen
                sprite_make(BALL_CHARNO, MTH_FIXED(-80), MTH_FIXED(-80), ball_sprites[i]);
                ball_sprites[i]->state = BALL_STATE_INIT; //ball attached to ship
            }
            else {
                sprite_make(BALL_CHARNO, ship_sprite->x, ship_sprite->y, ball_sprites[i]);
                ball_sprites[i]->dx = BALL_SPEED;
                ball_sprites[i]->dy = -BALL_SPEED;
                ball_sprites[i]->state = BALL_STATE_NORMAL; //ball comes out of ship
            }
            break;
        }
    }
}

int game_run() {
    static int frames;
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
            ship_sprite = sprite_next();
            sprite_make(SHIP_CHARNO, SHIP_STARTX, SHIP_STARTY, ship_sprite);
            ship_frames = 0;
            //init the ball array
            num_balls = 0;
            for (int i = 0; i < MAX_BALLS; i++) {
                ball_sprites[i] = NULL;
            }
            //add first ball
            game_addball();

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
            if (PadID1 == ANALOGPAD_ID) {
                Fixed32 dx = 0;
                //shoulder trigger controls
                Fixed32 left_trigger = MTH_Mul((PadAnalogL1 + 1) << 8, SHIP_SPEED);
                dx -= left_trigger;
                Fixed32 right_trigger = MTH_Mul((PadAnalogR1 + 1) << 8, SHIP_SPEED);
                dx += right_trigger;

                //analog stick controls
                //transform x pos by converting the 0-255 range of the analog x pos to be
                //MTH_FIXED(0) to MTH_FIXED(1), multiplying it by twice the max ship speed
                //(range: MTH_FIXED(0) to SHIP_SPEED * 2) and then subtracting SHIP_SPEED
                //(range: -SHIP_SPEED to SHIP_SPEED)
                Fixed32 movement = MTH_Mul(PadAnalogX1 << 8, (SHIP_SPEED + MTH_FIXED(0.15)) << 1) - (SHIP_SPEED + MTH_FIXED(0.15));
                dx += movement;
                //limit speed
                if (dx > SHIP_SPEED) dx = SHIP_SPEED;
                if (dx < -SHIP_SPEED) dx = -SHIP_SPEED;
            }
            else {
                ship_sprite->dx = 0;
                if (PadData1 & PAD_L) {
                    ship_sprite->dx -= SHIP_SPEED;
                }
                if (PadData1 & PAD_R) {
                    ship_sprite->dx += SHIP_SPEED;
                }
            }
            ship_sprite->x += ship_sprite->dx;

            //ship boundaries
            if (ship_sprite->x < LEFT_BOUND) {
                ship_sprite->x = LEFT_BOUND;
            }
            if (ship_sprite->x > RIGHT_BOUND) {
                ship_sprite->x = RIGHT_BOUND;
            }
            
            //move all balls on screen
            for (int i = 0; i < MAX_BALLS; i++) {
                if (ball_sprites[i] != NULL) {
                    //when the player spawns, keep the ball attached to his ship
                    //until he presses A
                    if (ball_sprites[i]->state == BALL_STATE_INIT) {
                        ball_sprites[i]->x = ship_sprite->x + BALL_SPAWN_XOFFSET;
                        ball_sprites[i]->y = ship_sprite->y + BALL_SPAWN_YOFFSET;
                        if (ship_sprite->dx < 0) {
                            ball_sprites[i]->dx = -BALL_SPEED;
                            ball_sprites[i]->dy = -BALL_SPEED;
                        }
                        else {
                            ball_sprites[i]->dx = BALL_SPEED;
                            ball_sprites[i]->dy = -BALL_SPEED;
                        }

                        if (PadData1E & PAD_A) {
                            ball_sprites[i]->state = BALL_STATE_NORMAL;
                        }
                    }
                    else if (ball_sprites[i]->state == BALL_STATE_NORMAL) {
                        ball_sprites[i]->x += ball_sprites[i]->dx;
                        ball_sprites[i]->y += ball_sprites[i]->dy;
                        //left/right wall
                        if ((ball_sprites[i]->x <= BALL_LBOUND) || ball_sprites[i]->x >= BALL_RBOUND) {
                            ball_sprites[i]->dx = -ball_sprites[i]->dx;
                        }
                        //top of screen
                        if (ball_sprites[i]->y <= BALL_MARGIN) {
                            ball_sprites[i]->dy = -ball_sprites[i]->dy;
                        }
                        //bottom
                        if ((ball_sprites[i]->x + BALL_MARGIN >= ship_sprite->x + SHIP_XMARGIN) &&
                            (ball_sprites[i]->x + BALL_WIDTH - BALL_MARGIN <= ship_sprite->x + SHIP_WIDTH - SHIP_XMARGIN) && 
                            (ball_sprites[i]->y + BALL_HEIGHT - BALL_MARGIN >= ship_sprite->y + SHIP_YMARGIN) &&
                            (ball_sprites[i]->y + BALL_MARGIN < ship_sprite->y + SHIP_HEIGHT)) {
                            //calculate ball's offset from paddle center
                            Fixed32 ball_offset = (ball_sprites[i]->x + (BALL_WIDTH >> 1)) - (ship_sprite->x + (SHIP_WIDTH >> 1));
                            Fixed32 normalized_offset = MTH_Div(ball_offset, (SHIP_WIDTH >> 1));
                            Fixed32 ball_angle = MTH_Mul(normalized_offset, MTH_FIXED(75));
                            ball_sprites[i]->dx = MTH_Mul(MTH_Sin(ball_angle), BALL_SPEED);
                            ball_sprites[i]->dy = MTH_Mul(MTH_Cos(ball_angle), -BALL_SPEED);
                        }

                        if (ball_sprites[i]->y > MTH_FIXED(SCROLL_LORES_Y)) {
                            sprite_delete(ball_sprites[i]);
                            ball_sprites[i] = NULL;
                            num_balls--;

                             if (num_balls == 0) {
                                game_chipset(STATE_CHIP_BURNT);
                                frames = 0;
                                state = STATE_GAME_LOSS;
                                lives--;
                                explosion_make(EXPLOSION_CHARNO, ship_sprite->x, ship_sprite->y);
                                ship_sprite->x = SHIP_STARTX;
                                ship_sprite->y = SHIP_STARTY;
                            }
                        }
                    }
                }
            }
            game_shipanim();
            break;
        
        //when player has lost, show burnt chip for a second
        //in order to make the player think about what he's done
        case STATE_GAME_LOSS:
            frames++;
            if (frames >= 60) {
                game_chipset(STATE_CHIP_NONE);
                ship_sprite->char_num = SHIP_CHARNO;
                game_addball();
                state = STATE_GAME_PLAY;
            }
        break;

    }
    //update score
    Uint32 *score_ptr = MAP_PTR32(0) + SCORE_AREA;
    int temp_score = score;
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
            curr_ptr += (64 - 8);
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

    game_chipanim();

    scroll_move(1, MTH_FIXED(0), MTH_FIXED(-4));
    scroll_move(2, MTH_FIXED(0), MTH_FIXED(-2));
    scroll_move(3, MTH_FIXED(0), MTH_FIXED(-1));
    return 0;
}
