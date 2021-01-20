#include <byteswap.h>
#include <dirent.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "qdbmp.h"
#include "tile.h"

int tile_read(BMP *image, uint8_t *buffer, int width, int height, int start_x, int start_y) {
	uint8_t value;
	for (int y = 0; y < 8; y++) {
		for (int x = 0; x < 8; x++) {
			if (((x + start_x) > width) || ((y + start_y) > height)) {
				printf("Error: Image width %d or height %d not a multiple of 8/16.\n", width, height);
				return 0;
			}
			BMP_GetPixelIndex(image, x + start_x, y + start_y, &value);
			buffer[y * 8 + x] = value;
		}
	}
	return 1;
}

int tile_process(char *filename, char *outname, int type) {
	BMP *infile = BMP_ReadFile(filename);
	BMP_CHECK_ERROR(stdout, 0);
	int width = BMP_GetWidth(infile);
	int height = BMP_GetHeight(infile);
	uint8_t bpp = BMP_GetDepth(infile);
	
	if ((bpp != 4) && (bpp != 8)) {
		printf("Error: Image not indexed!\n");
		BMP_Free(infile);
		return 0;
	}
	uint32_t *palette = malloc((1 << bpp) * sizeof(uint32_t));
	int color;
	uint8_t r, g, b;
	for (int i = 0; i < (1 << bpp); i++) {
		BMP_GetPaletteColor(infile, i, &r, &g, &b);
		color = r | (g << 8) | (b << 16);
		palette[i] = bswap_32(color); // saturn is big endian
	}
	
	size_t image_data_size;
	if (bpp == 4) {
		image_data_size = (width >> 1) * height;
	}
	else {
		image_data_size = width * height;
	}
	uint8_t *image_data = malloc(image_data_size);
	
	int image_cursor = 0;
	uint8_t single_tile[256];
	uint8_t val;
	switch (type) {
		case FRAMEBUFFER:
			if (bpp != 8) {
				printf("Error: Only 8bpp framebuffer images supported.\n");
				free(palette);
				free(image_data);
				BMP_Free(infile);
				return 0;
			}
			for (int y = 0; y < height; y++) {
				for (int x = 0; x < width; x++) {
					BMP_GetPixelIndex(infile, x, y, &val);
					image_data[y * width + x] = val;
				}
			}
			break;
			
		case TILE_8X8:
			for (int y = 0; y < height; y += 8) {
				for (int x = 0; x < width; x += 8) {
					if (!tile_read(infile, single_tile, width, height, x, y)) {
						free(palette);
						free(image_data);
						BMP_Free(infile);
						return 0;
					}
					if (bpp == 4) {
						for (int i = 0; i < 64; i += 2) {
							image_data[image_cursor++] = ((single_tile[i] & 0xF) << 4) | (single_tile[i + 1] & 0xF);
						}
					}
					else {
						for (int i = 0; i < 64; i++) {
							image_data[image_cursor++] = single_tile[i];
						}
					}
				}
			}
			break;
			
		case TILE_16x16:
			for (int y = 0; y < height; y += 16) {
				for (int x = 0; x < width; x += 16) {
					int res = 1;
					// top left
					res &= tile_read(infile, single_tile, width, height, x, y);
					// top right
					res &= tile_read(infile, single_tile + 64, width, height, x + 8, y);
					// bottom left
					res &= tile_read(infile, single_tile + 128, width, height, x, y + 8);
					// bottom right
					res &= tile_read(infile, single_tile + 192, width, height, x + 8, y + 8);
					if (!res) {
						free(palette);
						free(image_data);
						BMP_Free(infile);
						return 0;
					}
					if (bpp == 4) {
						for (int i = 0; i < 256; i += 2) {
							image_data[image_cursor++] = ((single_tile[i] & 0xF) << 4) | (single_tile[i + 1] & 0xF);
						}
					}
					else {
						for (int i = 0; i < 256; i++) {
							image_data[image_cursor++] = single_tile[i];
						}
					}
				}
			}
			break;
			
		default:
			printf("Error: invalid tile graphics type.\n");
			free(palette);
			free(image_data);
			BMP_Free(infile);
			return 0;
	}
		
	FILE *outfile = fopen(outname, "wb");
	if (!outfile) {
		printf("Error: Couldn't open %s for writing!\n", outname);
		free(palette);
		free(image_data);
		BMP_Free(infile);
		return 0;
	}
	uint32_t out_bpp = bswap_32((int)bpp);
	fwrite(&out_bpp, sizeof(uint32_t), 1, outfile);
	fwrite(palette, sizeof(uint32_t), (1 << bpp), outfile);
	fwrite(image_data, sizeof(uint8_t), image_data_size, outfile);
	fclose(outfile);
	
	free(palette);
	free(image_data);
	BMP_Free(infile);
	return 1;
}