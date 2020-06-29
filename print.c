#include "sprite.h"
#include "print.h"


//320x224, 8x16 font = 14 rows, 40 columns
#define ROWS 28
#define COLS 40
#define FONT_X 8
#define FONT_Y 8
Uint16 text[ROWS][COLS];


void print_init() {
	int i, j;
	for (i = 0; i < ROWS; i++) {
		for (j = 0; j < COLS; j++) {
			text[i][j] = 65535;
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
	while (ch[index]) {
		text[row][col + index] = (Uint16)ch[index] - 32;
		index++;
	}
}

void print_display() {
	int i, j;
	SPRITE_INFO text_spr;
	for (i = 0; i < ROWS; i++) {
		for (j = 0; j < COLS; j++) {
			if (text[i][j] != 65535) {
				sprite_make(text[i][j],
					MTH_IntToFixed(j << 3), // * FONT_X
					MTH_IntToFixed(i << 3), // * FONT_Y
					&text_spr);
				sprite_draw(&text_spr);
			}
		}
	}
}
