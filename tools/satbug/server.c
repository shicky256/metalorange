/*
    server.c: file server & debug console

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

#define CMD_BUF_SIZE (512)
static unsigned char cmd_buf[CMD_BUF_SIZE];
static char filename_buf[FILENAME_MAX];
#define PATH_BUF_SIZE (512)
static char path_buf[PATH_BUF_SIZE];
//main server loop
void server_run(char *directory)
{
    int status = 0;
    int state = FUNC_NULL;
    int cmd_cursor;
    int filename_cursor;
    unsigned char curr_char;

    printf("Started server in %s\n", directory);

    while (status >= 0)
    {
        status = ftdi_read_data(&device, cmd_buf, CMD_BUF_SIZE);
        if (status < 0)
        {
            printf("Read error: %s\n", ftdi_get_error_string(&device));
        }
        else if (status > 0)
        {
            cmd_cursor = 0;
            while (cmd_cursor < status)
            {
            start_switch:
                switch (state)
                {
                case FUNC_NULL:
                    state = (int)cmd_buf[cmd_cursor++];
                    filename_cursor = 0; //reset all variables for other states
                    goto start_switch;
                    break;

                case FUNC_DOWNLOAD: ;
                    while (cmd_cursor < status)
                    {
                        curr_char = cmd_buf[cmd_cursor++];
                        filename_buf[filename_cursor++] = curr_char;
                        if (curr_char == '\0')
                        {
                            strncpy(path_buf, directory, PATH_BUF_SIZE);
                            strncat(path_buf, filename_buf, PATH_BUF_SIZE);

                            printf("Requested to upload %s\n", path_buf);
                            if (!devcart_upload(path_buf, 0))
                            {
                                printf("Error uploading file\n");
                            }
                            state = FUNC_NULL;
                            break;
                        }
                    }
                    break;

                case FUNC_PRINT:
                    while (cmd_cursor < status)
                    {
                        curr_char = cmd_buf[cmd_cursor++];
                        if (curr_char == '\0')
                        {
                            state = FUNC_NULL;
                            goto start_switch;
                            break;
                        }
                        putchar(curr_char);
                    }
                    break;
                }
            }
        }
    }
}
