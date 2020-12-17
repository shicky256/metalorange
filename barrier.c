#include <SEGA_MTH.H>

#include "ball.h"
#include "barrier.h"
#include "game.h"
#include "sound.h"
#include "sprite.h"

static int barrier_charno;

#define BARRIER_MAXLIFE (5)
// how many hits the barrier has left
int barrier_life;

#define BARRIER_FRAMES (8)
static int barrier_animcursor;
// frames per animation frame
#define BARRIER_TIMING (3)
static int barrier_timer;

// number of barrier sprites
#define BARRIER_NUM (13)
#define BARRIER_WIDTH (MTH_FIXED(16))
#define BARRIER_YPOS (MTH_FIXED(224))

void barrier_init(int charno) {
    barrier_charno = charno;
    barrier_life = 0;
    barrier_animcursor = 0;
    barrier_timer = 0;
}

void barrier_move() {
    if (barrier_life == 0) {
        return;
    }
    // handle ball collision
    for (int i = 0; i < ball_count; i++) {
        if (ball_sprites[i].y > BARRIER_YPOS) {
            ball_sprites[i].y = BARRIER_YPOS;
            ball_sprites[i].dy = -ball_sprites[i].dy;
            barrier_life--;
            sound_play(SOUND_START); // menu start sound is the same as barrier bounce sound
        }
    }

    barrier_timer++;
    if (barrier_timer >= BARRIER_TIMING) {
        barrier_timer = 0;
        barrier_animcursor += 2; // each sprite is 16x16, but the animation is in 32x16
        if (barrier_animcursor >= BARRIER_FRAMES) {
            barrier_animcursor = 0;
        }
    }
}

void barrier_draw() {
    SPRITE_INFO barrier_spr;
    int tileno = barrier_charno + barrier_animcursor;

    if (barrier_life == 0) {
        return; // don't draw the barrier if it's not there
    }
    if (barrier_life == 1) { // switch to red barrier if one hit is left
        tileno += BARRIER_FRAMES;
    }
    for (int i = 0; i < BARRIER_NUM; i++) {
        if (i & 1) {
            sprite_make(tileno + 1, LEFT_WALL + (i * BARRIER_WIDTH), BARRIER_YPOS, &barrier_spr);
        }
        else {
            sprite_make(tileno, LEFT_WALL + (i * BARRIER_WIDTH), BARRIER_YPOS, &barrier_spr);
        }
        sprite_draw(&barrier_spr);
    }
}
