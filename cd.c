#include <sega_gfs.h>
#include "print.h"

// max num files that can be opened simultaneously
#define MAX_OPEN        1

// max number of files per directory
#define MAX_DIR         100

// size of one sector in bytes
#define SECT_SIZE       2048

Uint32 lib_work[GFS_WORK_SIZE(MAX_OPEN) / sizeof(Uint32)]; //library work area

GfsDirTbl directory_table; //directory info management struct
GfsDirName dirname[MAX_DIR]; //list of all filenames

void cd_init(void) {
    GFS_DIRTBL_TYPE(&directory_table) = GFS_DIR_NAME;
    GFS_DIRTBL_DIRNAME(&directory_table) = dirname;
    GFS_DIRTBL_NDIR(&directory_table) = MAX_DIR;
    GFS_Init(MAX_OPEN, lib_work, &directory_table);
}

/**
 * read data off the cd's root directory
 * filename: duh
 * dataBuf: where you want to copy the data to
 * read_size: # of bytes to read 
 */
void cd_load(char *filename, void *dataBuf, int read_size) {
    GfsHn gfs = GFS_Open(GFS_NameToId(filename));
    //make sure we read at least one sector
    GFS_Fread(gfs, read_size < SECT_SIZE ? 1 : (read_size >> 11) + 1, dataBuf, read_size);
    GFS_Close(gfs);
}


Sint32 cd_load_nosize(char *filename, void *dataBuf) {
    Sint32 filesize;
    GfsHn gfs = GFS_Open(GFS_NameToId(filename));
    GFS_GetFileInfo(gfs, NULL, NULL, &filesize, NULL);
    //make sure we read at least one sector
    GFS_Fread(gfs, filesize < SECT_SIZE ? 1 : (filesize >> 11) + 1, dataBuf, filesize);
    GFS_Close(gfs);
    return filesize;
}
