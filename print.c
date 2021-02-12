#include <sega_scl.h>
#define _SPR2_
#include <sega_spr.h>

#include "cd.h"
#include "sprite.h"
#include "print.h"

#define ROWS 60
#define COLS 88
#define FONT_X 8
#define FONT_Y 8
Uint8 text[ROWS][COLS];

void print_load() {
	sprite_load("FONT.SPR", NULL);
}

void print_init() {
	int i, j;
	for (i = 0; i < ROWS; i++) {
		for (j = 0; j < COLS; j++) {
			text[i][j] = 255;
		}
	}
}

void print_num(Uint32 num, int row, int col) {
	int right_col = col + 9; //rightmost column
	int i;
	for (i = 0; i <= 9; i++) {
		text[row][right_col--] = (num % 10) + 16;
		num /= 10;
	}
}

void print_string(char *ch, int row, int col) {
	int index = 0;
	int col_bak = col;
	while (ch[index]) {
		if (ch[index] == '\n') {
			row++;
			col = col_bak;
		}
		else {
			text[row][col++] = ch[index] - 32;
		}
		index++;
	}
}

void print_display() {
	int i, j;
	SPRITE_INFO text_spr;
	for (i = 0; i < ROWS; i++) {
		for (j = 0; j < COLS; j++) {
			if (text[i][j] != 255) {
				sprite_make(text[i][j],
					MTH_IntToFixed(j << 3), // * FONT_X
					MTH_IntToFixed(i << 3), // * FONT_Y
					&text_spr);
				sprite_draw(&text_spr);
			}
		}
	}
}
