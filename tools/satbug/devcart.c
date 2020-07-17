/*
    devcart.c: dev cart comms stuff

    Copyright © 2020 Nathan Misner
    Copyright © 2012, 2013, 2015 Anders Montonen
    All rights reserved.
    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    Redistributions of source code must retain the above copyright notice, this
    list of conditions and the following disclaimer.

    Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.
    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include "ftdi.h"

#include "crc.h"
#include "devcart.h"

static unsigned char send_buf[2*WRITE_PAYLOAD_SIZE];
static unsigned char recv_buf[2*READ_PAYLOAD_SIZE];
ftdi_context_t device = {0};

int devcart_download(const char *pFilename, const unsigned int address,
                      const unsigned int size)
{
    unsigned char  *pFileBuffer = NULL;
    FILE           *File = NULL;
    unsigned int    received = 0;
    int             status = -1;
    crc_t           readChecksum, calcChecksum;
    struct timeval      before, after;
    signed long long    timedelta;

    pFileBuffer = (unsigned char*)malloc(size);
    if (pFileBuffer != NULL)
    {
        gettimeofday(&before, NULL);
        send_buf[0] = FUNC_DOWNLOAD; /* Client function */
        send_buf[1] = (unsigned char)(address >> 24);
        send_buf[2] = (unsigned char)(address >> 16);
        send_buf[3] = (unsigned char)(address >> 8);
        send_buf[4] = (unsigned char)(address);
        send_buf[5] = (unsigned char)(size >> 24);
        send_buf[6] = (unsigned char)(size >> 16);
        send_buf[7] = (unsigned char)(size >> 8);
        send_buf[8] = (unsigned char)(size);

        status = ftdi_write_data(&device, send_buf, 9);
        if (status < 0)
        {
            printf("Send download command error: %s\n",
                   ftdi_get_error_string(&device));
            goto DownloadError;
        }

        while (size - received > 0)
        {
            status = ftdi_read_data(&device, &pFileBuffer[received], size);
            if (status < 0)
            {
                printf("Read data error: %s\n",
                       ftdi_get_error_string(&device));
                goto DownloadError;
            }

            received += status;
        }

        // The transfer may timeout, so loop until a byte
        // is received or an error occurs.
        do
        {
            status = ftdi_read_data(&device, (unsigned char*)&readChecksum, 1);
            if (status < 0)
            {
                printf("Read data error: %s\n",
                       ftdi_get_error_string(&device));
                goto DownloadError;
            }
        } while (status == 0);

        gettimeofday(&after, NULL);
        timedelta = (signed long long) after.tv_sec * 1000000ll +
                    (signed long long) after.tv_usec -
                    (signed long long) before.tv_sec * 1000000ll -
                    (signed long long) before.tv_usec;
        printf("Transfer time %f\n", timedelta/1000000.0f);
        printf("Transfer speed %f K/s\n", (size/1024.0f)/(timedelta/1000000.0f));

        calcChecksum = crc_init();
        calcChecksum = crc_update(calcChecksum, pFileBuffer, size);
        calcChecksum = crc_finalize(calcChecksum);

        if (readChecksum != calcChecksum)
        {
            printf("Checksum error (%0x, should be %0x)\n",
                   calcChecksum, readChecksum);
            status = -1;
            goto DownloadError;
        }

        File = fopen(pFilename, "wb");
        if (File == NULL)
        {
            printf("Error creating output file\n");
            status = -1;
        }
        else
        {
            fwrite(pFileBuffer, 1, size, File);
            fclose(File);
        }

DownloadError:
        free(pFileBuffer);
    }

    return status < 0 ? 0 : 1;
}

/* Sending the write command and data separately is inefficient,
   but simplifies the code. The alternative is to copy also the data
   into the sendbuffer. */
int devcart_upload(const char *pFilename, const unsigned int Address)
{
    unsigned char      *pFileBuffer = NULL;
    unsigned int        size = -1, sent = 0;
    FILE               *File = NULL;
    int                 status = 0;
    crc_t               checksum = crc_init();
    struct timeval      before, after;
    signed long long    timedelta;

    File = fopen(pFilename, "rb");
    if (File == NULL)
    {
        printf("Can't open the file '%s'\n", pFilename);
    }
    else
    {
        fseek(File, 0, SEEK_END);
        size = ftell(File);
        fseek(File, 0, SEEK_SET);
        pFileBuffer = malloc(size);

        if (pFileBuffer == NULL)
        {
            printf("Memory allocation error\n");
        }
        else
        {
            fread(pFileBuffer, 1, size, File);
            checksum = crc_update(checksum, pFileBuffer, size);
            checksum = crc_finalize(checksum);

            gettimeofday(&before, NULL);
            send_buf[0] = FUNC_UPLOAD; /* Client function */
            send_buf[1] = (unsigned char)(Address >> 24);
            send_buf[2] = (unsigned char)(Address >> 16);
            send_buf[3] = (unsigned char)(Address >> 8);
            send_buf[4] = (unsigned char)Address;
            send_buf[5] = (unsigned char)(size >> 24);
            send_buf[6] = (unsigned char)(size >> 16);
            send_buf[7] = (unsigned char)(size >> 8);
            send_buf[8] = (unsigned char)size;
            status = ftdi_write_data(&device, send_buf, 9);

            if (status < 0)
            {
                printf("Send upload command error: %s\n",
                       ftdi_get_error_string(&device));
                goto UploadError;
            }

            while (size - sent > 0)
            {
                status = ftdi_write_data(&device, &pFileBuffer[sent], size-sent);
                if (status < 0)
                {
                    printf("Send data error: %s\n",
                           ftdi_get_error_string(&device));
                    goto UploadError;
                }

                sent += status;
            }

            send_buf[0] = (unsigned char)checksum;
            status = ftdi_write_data(&device, send_buf, 1);

            if (status < 0)
            {
                printf("Send checksum error: %s\n",
                       ftdi_get_error_string(&device));
                goto UploadError;
            }

            do
            {
                status = ftdi_read_data(&device, recv_buf, 1);
                if (status < 0)
                {
                    printf("Read upload result failed: %s\n",
                           ftdi_get_error_string(&device));
                    goto UploadError;
                }
            } while (status == 0);

            if (recv_buf[0] != 0)
            {
                status = -1;
            }

            gettimeofday(&after, NULL);
            timedelta = (signed long long) after.tv_sec * 1000000ll +
                        (signed long long) after.tv_usec -
                        (signed long long) before.tv_sec * 1000000ll -
                        (signed long long) before.tv_usec;
            printf("Transfer time %f\n", timedelta/1000000.0f);
            printf("Transfer speed %f K/s\n", (size/1024.0f)/(timedelta/1000000.0f));

UploadError:
            free(pFileBuffer);
        }
        fclose(File);
    }

    return status < 0 ? 0 : 1;
}

int devcart_execute(const char *pFilename, const unsigned int Address)
{
    int status = 0;
    if (devcart_upload(pFilename, Address))
    {
        send_buf[0] = FUNC_EXEC; /* Client function */
        send_buf[1] = (unsigned char)(Address >> 24);
        send_buf[2] = (unsigned char)(Address >> 16);
        send_buf[3] = (unsigned char)(Address >> 8);
        send_buf[4] = (unsigned char)Address;
        status = ftdi_write_data(&device, send_buf, 5);
        if (status < 0)
        {
            printf("Send execute error: %s\n",
                   ftdi_get_error_string(&device));
        }
    }

    return status < 0 ? 0 : 1;
}

int devcart_init(const int VID, const int PID)
{
    int status = ftdi_init(&device);
    int error = 0;

    if (status < 0)
    {
        printf("Init error: %s\n", ftdi_get_error_string(&device));
        error = 1;
    }
    else
    {
        status = ftdi_usb_open(&device, VID, PID);
        if (status < 0 && status != -5)
        {
            printf("Device open error: %s\n", ftdi_get_error_string(&device));
            error = 1;
        }
        else
        {
            status = ftdi_usb_purge_buffers(&device);
            if (status < 0)
            {
                printf("Purge buffers error: %s\n",
                       ftdi_get_error_string(&device));
                error = 1;
            }

            status = ftdi_read_data_set_chunksize(&device, USB_READPACKET_SIZE);
            if (status < 0)
            {
                printf("Set read chunksize error: %s\n",
                       ftdi_get_error_string(&device));
                error = 1;
            }

            status = ftdi_write_data_set_chunksize(&device, USB_WRITEPACKET_SIZE);
            if (status < 0)
            {
                printf("Set write chunksize error: %s\n",
                       ftdi_get_error_string(&device));
                error = 1;
            }

            status = ftdi_set_bitmode(&device, 0x0, BITMODE_RESET);
            if (status < 0)
            {
                printf("Bitmode configuration error: %s\n",
                       ftdi_get_error_string(&device));
                error = 1;
            }

            if (error)
            {
                ftdi_usb_close(&device);
            }
        }
    }

    return !error;
}

void devcart_close(void)
{
    int status = ftdi_usb_purge_buffers(&device);
    if (status < 0)
    {
        printf("Purge buffers error: %s\n",
               ftdi_get_error_string(&device));
    }

    ftdi_usb_close(&device);
}

