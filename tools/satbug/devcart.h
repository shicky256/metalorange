#ifndef DEVCART_H
#define DEVCART_H

/* Optimal payload/usb transfer size, see FTDI appnote */
#define USB_READPACKET_SIZE (64*1024)
#define USB_WRITEPACKET_SIZE (4*1024)
#define USB_PAYLOAD(x) ((x)-(((x)/64)*2))
#define READ_PAYLOAD_SIZE (USB_PAYLOAD(USB_READPACKET_SIZE))
#define WRITE_PAYLOAD_SIZE (USB_PAYLOAD(USB_WRITEPACKET_SIZE))

enum
{
    FUNC_NULL = 0,
    FUNC_DOWNLOAD,
    FUNC_UPLOAD,
    FUNC_EXEC,
    FUNC_PRINT,
    FUNC_QUIT
};

typedef struct ftdi_context ftdi_context_t;
extern ftdi_context_t device;

int devcart_download(const char *pFilename, const unsigned int Address,
                       const unsigned int Size);
int devcart_upload(const char *pFilename, const unsigned int Address);
int devcart_execute(const char *pFilename, const unsigned int Address);
int devcart_init(const int VID, const int PID);
void devcart_close(void);

#endif // DEVCART_H
