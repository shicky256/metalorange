#include <stdio.h>
#include <string.h>

#include "debug.h"
#include "sprite.h"

#define BUFFER_LEN (256)

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
		switch(line[0]) {
			// sprite directory
			case 's':
				sprite_process(&line[2], "out.spr");
				break;
			
			// tile graphics
			case 't':
				break;
			
			// tilemap
			case 'm':
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