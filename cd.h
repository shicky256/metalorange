#ifndef CD_H
#define CD_H

#define LWRAM	(2097152)

//init cd stuff
void cd_init(void);

/**
 * read data off the cd's root directory
 * filename: duh
 * dataBuf: where to copy the data to
 * returns the loaded file's size
 */
Sint32 cd_load(char *filename, void *dataBuf);
#endif
