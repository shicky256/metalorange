#include <SEGA_MTH.H>
#include <SEGA_PER.H>
#include "crc.h"
#include "release.h"


#define USB_FLAGS (*(volatile Uint8*)(0x22200001))
#define USB_RXF     (1 << 0)
#define USB_TXE     (1 << 1)
#define USB_FIFO (*(volatile Uint8*)(0x22100001))

enum {
    FUNC_NULL = 0,
    FUNC_DOWNLOAD,
    FUNC_UPLOAD,
    FUNC_EXEC,
    FUNC_PRINT,
    FUNC_QUIT
};

static inline Uint8 devcart_getbyte(void) {
    while ((USB_FLAGS & USB_RXF) != 0);
    return USB_FIFO;
}

static Uint32 devcart_getdword(void) {
    Uint32 tmp = devcart_getbyte();
    tmp = (tmp << 8) | devcart_getbyte();
    tmp = (tmp << 8) | devcart_getbyte();
    tmp = (tmp << 8) | devcart_getbyte();

    return tmp;
}

static inline void devcart_putbyte(Uint8 byte) {
    while ((USB_FLAGS & USB_TXE) != 0);
    USB_FIFO = byte;
}

int devcart_loadfile(char *filename, void *dest) {
    Uint8 *ptr = (Uint8 *)dest;
    Uint8 letter;
    int len;
    crc_t readchecksum;
    crc_t checksum = crc_init();

    //tell server we want to download a file
    devcart_putbyte(FUNC_DOWNLOAD);
    //tell server the filename we want to download
    for (int i = 0;; i++) {
        letter = (Uint8)filename[i];
        devcart_putbyte(letter);
        if (letter == '\0') {
            break;
        }
    }

    //pc server will send "upload" command back, read that byte
    devcart_getbyte();
    //pc server sends address first, this is unnecessary since we're
    //specifying it
    devcart_getdword();
    len = (int)devcart_getdword(); //file length

    for (int i = 0; i < len; i++) {
        // inlining is 20K/s faster
        while ((USB_FLAGS & USB_RXF) != 0);
        ptr[i] = USB_FIFO;
    }

    readchecksum = devcart_getbyte();

    checksum = crc_update(checksum, ptr, len);
    checksum = crc_finalize(checksum);

    if (checksum != readchecksum) {
        devcart_putbyte(0x1);
    }
    else {
        devcart_putbyte(0);
    }

    return (int)len;
}

void devcart_printstr(char *string) {
    #if DEVCART_LOAD
    devcart_putbyte(FUNC_PRINT);
    for (int i = 0;; i++) {
        devcart_putbyte(string[i]);
        if (string[i] == '\0') {
            break;
        }
    }
    #endif
}

void devcart_reset() {
    // quit server program
    devcart_putbyte(FUNC_QUIT);
    // reset saturn
    PER_SMPC_SYS_RES();
}

