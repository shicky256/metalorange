#include <assert.h>
#include <sega_mth.h>

#include "capsule.h"
#include "game.h"
#include "scroll.h"
#include "sprite.h"

typedef struct {
    int anim_timer;
    SPRITE_INFO *prev;
    SPRITE_INFO *next;
} CAPSULE_DATA;

static_assert(sizeof(CAPSULE_DATA) <= SPRITE_DATA_SIZE, "Struct size too large");

//first capsule character in vdp1
static int capsule_charno;
//how many capsule frames there are
#define CAPSULE_CHARCOUNT (3)
//how fast the capsule moves
#define CAPSULE_SPEED (MTH_FIXED(1))
static int capsule_count;
static SPRITE_INFO *capsule_head;
#define CAPSULE_TIMING (16)

void capsule_init(int charno) {
    capsule_charno = charno;
    capsule_count = 0;
    capsule_head = NULL;
}

void capsule_remove(SPRITE_INFO *capsule) {
    CAPSULE_DATA *capsule_data = (CAPSULE_DATA *)capsule->data;

    if (capsule == capsule_head) {
        capsule_head = capsule_data->next;
    }

    if (capsule_data->next != NULL) {
        ((CAPSULE_DATA *)(capsule_data->next->data))->prev = capsule_data->prev;
    }

    if (capsule_data->prev != NULL) {
        ((CAPSULE_DATA *)(capsule_data->prev->data))->next = capsule_data->next;
    }
    capsule_count--;
    sprite_delete(capsule);
}

void capsule_removeall() {
    while (capsule_head != NULL) {
        capsule_remove(capsule_head);
    }
}

static void capsule_move(SPRITE_INFO *capsule) {
    if (!game_playing()) {
        return;
    }

    CAPSULE_DATA *capsule_data = (CAPSULE_DATA *)capsule->data;
    //make the capsule fall
    capsule->y += CAPSULE_SPEED;
    //animate it
    capsule_data->anim_timer++;
    if (capsule_data->anim_timer > CAPSULE_TIMING) {
        capsule_data->anim_timer = 0;
        capsule->char_num++;
        if (capsule->char_num - capsule_charno >= CAPSULE_CHARCOUNT) {
            capsule->char_num = capsule_charno;
        }
    }
    int collision = 0;
    if ((capsule->x + CAPSULE_WIDTH >= ship_left) &&
        (capsule->x <= ship_right) && 
        (capsule->y + CAPSULE_BOTTOM >= ship_sprite.y + SHIP_YMARGIN) &&
        (capsule->y < ship_sprite.y + SHIP_HEIGHT)) {
        collision = 1;
        game_incpowerup();
    }
    //remove the capsule if it's passed the bottom of the screen or is touching the player
    if ((capsule->y > MTH_FIXED(SCROLL_LORES_Y) || collision)) {
        capsule_remove(capsule);
    }
}

void capsule_add(Fixed32 x, Fixed32 y) {
    SPRITE_INFO *capsule = sprite_next();
    sprite_make(capsule_charno, x, y, capsule);
    CAPSULE_DATA *capsule_data = (CAPSULE_DATA *)(capsule->data);
    capsule_data->next = capsule_head;
    capsule_data->prev = NULL;
    if (capsule_head != NULL) {
        ((CAPSULE_DATA *)(capsule_head->data))->prev = capsule;
    }
    capsule_head = capsule;
    capsule_data->anim_timer = 0;
    capsule->iterate = capsule_move;
    capsule_count++;
}

