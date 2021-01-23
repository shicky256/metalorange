#include <stdio.h>
#include <string.h>

#include "debug.h"
#include "map.h"
#include "sprite.h"
#include "tile.h"

#define MIN(x, y) ((x) < (y) ? (x) : (y))

#define BUFFER_LEN (256)
#define ASCII_NUMBER_BASE (0x30)

int main(int argc, char **argv) {
	if (argc < 2) {
		printf("Usage: satconv [asset list]\n");
		return -1;
	}
	
	FILE *asset_list = fopen(argv[1], "r");
	if (asset_list == NULL) {
		printf("Error: couldn't open asset list.\n");
		return -2;
	}
	
	char line[BUFFER_LEN];
	while (fgets(line, BUFFER_LEN - 1, asset_list)) {
		line[strcspn(line, "\r\n")] = 0;
		DBG_PRINTF("%s\n", line);
		char *infile;
		char *ext;
		int ext_index;
		#define OUTFILE_LEN (32)
		char outfile[OUTFILE_LEN];
		switch(line[0]) {
			// sprite directory
			case 's':
				infile = &line[2];
				// cd-rom filesystem only allows 8.3 filenames
				ext_index = MIN(8, strlen(infile));
				memcpy(outfile, infile, ext_index);
				strcpy(outfile + ext_index, ".spr");
				printf("%s\n", outfile);
				sprite_process(infile, outfile);
				break;
			
			// tile graphics
			case 't':
				infile = &line[3];
				ext = strchr(infile, '.');
				ext_index = MIN(8, ext - infile);
				memcpy(outfile, infile, ext_index);
				strcpy(outfile + ext_index, ".tle");
				printf("%s\n", outfile);
				tile_process(infile, outfile, line[1] - ASCII_NUMBER_BASE);
				break;
			
			// tilemap
			case 'm':
				infile = &line[4];
				ext = strchr(infile, '.');
				ext_index = MIN(8, ext - infile);
				memcpy(outfile, infile, ext_index);
				strcpy(outfile + ext_index, ".map");
				printf("%s\n", outfile);
				map_process(infile, outfile, line[1] - ASCII_NUMBER_BASE, line[2] - ASCII_NUMBER_BASE);
				break;
			
			default:
				printf("Error: Invalid asset list format.\n");
				fclose(asset_list);
				return -3;
				
		}
	}
	
	fclose(asset_list);
	return 0;
}