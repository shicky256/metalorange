#include <sega_cdc.h>
#include <sega_gfs.h>

#include "devcart.h"
#include "print.h"
#include "release.h"

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
    #if DEVCART_LOAD == 0

    GFS_DIRTBL_TYPE(&directory_table) = GFS_DIR_NAME;
    GFS_DIRTBL_DIRNAME(&directory_table) = dirname;
    GFS_DIRTBL_NDIR(&directory_table) = MAX_DIR;
    GFS_Init(MAX_OPEN, lib_work, &directory_table);
    #endif
}

/**
 * read data off the cd's root directory
 * filename: duh
 * dataBuf: where you want to copy the data to
 * returns number of bytes read
 */
Sint32 cd_load(char *filename, void *dataBuf) {
    #if DEVCART_LOAD
    return devcart_loadfile(filename, dataBuf);
    #else
    Sint32 filesize;
    GfsHn gfs = GFS_Open(GFS_NameToId((Sint8 *)filename));
    GFS_GetFileInfo(gfs, NULL, NULL, &filesize, NULL);
    //make sure we read at least one sector
    GFS_Fread(gfs, filesize < SECT_SIZE ? 1 : (filesize >> 11) + 1, dataBuf, filesize);
    GFS_Close(gfs);
    return filesize;
    #endif
}
