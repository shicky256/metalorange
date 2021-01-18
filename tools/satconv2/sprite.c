#include <byteswap.h>
#include <dirent.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "qdbmp.h"
#include "sprite.h"

#define PALETTE_SIZE (16)
static uint32_t **palette_list;
static int palette_cursor;

typedef struct {
	int x;
	int y;
	int pal;
	uint8_t *graphics;
} IMAGE_INFO;

static IMAGE_INFO *info_list;
static int info_cursor;

void sprite_convert(BMP *tile) {
	uint32_t palette_buffer[PALETTE_SIZE];
	uint8_t r;
	uint8_t g;
	uint8_t b;
	
	IMAGE_INFO *info = &info_list[info_cursor];
	info_cursor++;
		
	info->x = BMP_GetWidth(tile);
	info->y = BMP_GetHeight(tile);
	
	// read in the palette data
	int color;
	for (int i = 0; i < PALETTE_SIZE; i++) {
		BMP_GetPaletteColor(tile, i, &r, &g, &b);
		color = r | (g << 8) | (b << 16);
		palette_buffer[i] = bswap_32(color);
	}
	// check for a matching palette
	int palno = -1;
	for (int i = 0; i < palette_cursor; i++) {
		if (memcmp(palette_buffer, palette_list[i], PALETTE_SIZE * sizeof(uint32_t)) == 0) {
			palno = i;
			break;
		}
	}
	
	// add one if there isn't one
	if (palno == -1) {
		uint32_t *palette = palette_list[palette_cursor];
		memcpy(palette, palette_buffer, PALETTE_SIZE * sizeof(uint32_t));
		info->pal = palette_cursor;
		palette_cursor++;
	}
	else {
		info->pal = palno;
	}
	
	// read in the graphics data
	info->graphics = malloc((info->x >> 1) * info->y);
	uint8_t index1;
	uint8_t index2;
	uint8_t val;
	for (int y = 0; y < info->y; y++) {
		// 4bpp so we only count through half the x vals
		// and combine them together
		for (int x = 0; x < (info->x >> 1); x++) {
			// printf("x: %d, y: %d\n", x, y);
			BMP_GetPixelIndex(tile, x << 1, y, &index1);
			BMP_GetPixelIndex(tile, (x << 1) + 1, y, &index2);
			val = ((index1 & 0xF) << 4) | (index2 & 0xF);
			info->graphics[(y * (info->x >> 1)) + x] = val;
		}
	}
}

#define FILENAME_BUFLEN (256)

int sprite_process(char *dirname, char *outfile) {
	struct dirent *entry;
	DIR *dir_ptr;
	palette_cursor = 0;
	info_cursor = 0;
	char filename[FILENAME_BUFLEN];
	
	// get number of files in the directory
	int num_files = 0;
	dir_ptr = opendir(dirname);
	if (dir_ptr == NULL) {
		printf("Error: directory %s doesn't exist.\n", dirname);
		return 0;
	}
	while ((entry = readdir(dir_ptr)) != NULL) {
		if ((strcmp(entry->d_name, ".") == 0) || (strcmp(entry->d_name, "..") == 0)) {
			continue;
		}
		if (entry->d_type == DT_REG) {
			num_files++;
		}
	}
	closedir(dir_ptr);
	
	// allocate memory based on the number of files
	// the maximum number of palettes possible is one per frame
	palette_list = malloc(num_files * sizeof(uint32_t *));
	for (int i = 0; i < num_files; i++) {
		palette_list[i] = malloc(PALETTE_SIZE * sizeof(uint32_t));
	}
	info_list = malloc(num_files * sizeof(IMAGE_INFO));
	
	// read in all the files
	dir_ptr = opendir(dirname);
	if (dir_ptr == NULL) {
		printf("Error: directory %s doesn't exist.\n", dirname);
		return 0;
	}
	while ((entry = readdir(dir_ptr)) != NULL) {
		if ((strcmp(entry->d_name, ".") == 0) || (strcmp(entry->d_name, "..") == 0)) {
			continue;
		}
		if (entry->d_type == DT_REG) {
			printf("%s/%s\n", dirname, entry->d_name);
			int retval;
			if ((retval = snprintf(filename, FILENAME_BUFLEN, "%s/%s", dirname, entry->d_name)) > FILENAME_BUFLEN) {
				printf("Error: overran filename buffer by %d bytes. Increase FILENAME_BUFLEN define in sprite.c.\n", retval);
				closedir(dir_ptr);
				return 0;
			}
			DBG_PRINTF("Reading file %s\n", filename);
			BMP *tiledata = BMP_ReadFile(filename);
			// BMP_CHECK_ERROR(stdout, 0);
			if (BMP_GetDepth(tiledata) != 4) {
				printf("Error: file %s isn't 4bpp.\n", entry->d_name);
				BMP_Free(tiledata);
				closedir(dir_ptr);
				return 0;
			}
			sprite_convert(tiledata);
			BMP_Free(tiledata);
		}
	}
	closedir(dir_ptr);
	
	// write out the sprite data
	FILE *out = fopen(outfile, "wb");
	// saturn is big-endian so all the data over one byte must be byteswapped
	// write number of palettes
	int palette_cursor_be = bswap_32(palette_cursor);
	fwrite(&palette_cursor_be, sizeof(palette_cursor_be), 1, out);
	// write all the palettes (they're already byteswapped)
	for (int i = 0; i < palette_cursor; i++) {
		fwrite(palette_list[i], sizeof(uint32_t), PALETTE_SIZE, out);
	}
	// write number of sprites
	int num_files_be = bswap_32(num_files);
	fwrite(&num_files_be, sizeof(num_files_be), 1, out);
	// write IMAGE_INFO structs
	int tmp_x, tmp_y, tmp_pal;
	for (int i = 0; i < num_files; i++) {
		tmp_x = bswap_32(info_list[i].x);
		tmp_y = bswap_32(info_list[i].y);
		tmp_pal = bswap_32(info_list[i].pal);
		fwrite(&tmp_x, sizeof(tmp_x), 1, out);
		fwrite(&tmp_y, sizeof(tmp_y), 1, out);
		fwrite(&tmp_pal, sizeof(tmp_pal), 1, out);
		fwrite(info_list[i].graphics, sizeof(uint8_t), (info_list[i].x >> 1) * info_list[i].y, out);
	}
	fclose(out);
	return 1;
}