/*
    Sega Saturn USB flash cart transfer utility
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
#include <signal.h>

#include <sys/time.h>

#include "devcart.h"
#include "server.h"

static void PrintUsage(const char *pProgname);
static void ParseNumericArg(const char *pArg, unsigned int *pResult);
static void Signal(int sig);

int main(int argc, char *argv[])
{
    int             ii = 1;
    int             function = 0, error = 0, server = 0;
    unsigned int    address = 0, length = 0;
    char           *pFilename = NULL;
    int             VID = 0x0403, PID = 0x6001;
    char           *pVID = NULL, *pPID = NULL;
    char           *server_dir = NULL;

    if ((pVID = getenv("VID")))
        sscanf(pVID, "%x", &VID);

    if ((pPID = getenv("PID")))
        sscanf(pPID, "%x", &PID);

    while (ii < argc && !error)
    {
        if (!strcmp(argv[ii], "-v") || !strcmp(argv[ii], "-V"))
        {
            if (argc < ii + 1)
            {
                error = 1;
            }
            else
            {
                sscanf(argv[ii+1], "%x", &VID);
                ii += 2;
            }
        }
        else if (!strcmp(argv[ii], "-p") || !strcmp(argv[ii], "-P"))
        {
            if (argc < ii + 1)
            {
                error = 1;
            }
            else
            {
                sscanf(argv[ii+1], "%x", &PID);
                ii += 2;
            }
        }
        else if (!strcmp(argv[ii], "-d") || !strcmp(argv[ii], "-D"))
        {
            if (argc < ii + 4)
            {
                error = 1;
            }
            else
            {
                pFilename = argv[ii+1];
                ParseNumericArg(argv[ii+2], &address);
                ParseNumericArg(argv[ii+3], &length);
                ii += 4;
                function = FUNC_DOWNLOAD;
            }
        }
        else if (!strcmp(argv[ii], "-u") || !strcmp(argv[ii], "-U"))
        {
            if (argc < ii + 3)
            {
                error = 1;
            }
            else
            {
                pFilename = argv[ii+1];
                ParseNumericArg(argv[ii+2], &address);
                ii += 3;
                function = FUNC_UPLOAD;
            }
        }
        else if (!strcmp(argv[ii], "-x") || !strcmp(argv[ii], "-X"))
        {
            if (argc < ii + 3)
            {
                error = 1;
            }
            else
            {
                pFilename = argv[ii+1];
                ParseNumericArg(argv[ii+2], &address);
                ii += 3;
                function = FUNC_EXEC;
            }
        }
        else if (!strcmp(argv[ii], "-s") || !strcmp(argv[ii], "-S"))
        {
            if (argc < ii + 1)
            {
                error = 1;
            }
            else
            {
                server = 1;
                server_dir = argv[ii + 1];
                ii += 2;
            }
        }
    }

    if (error || (!function))
    {
        PrintUsage(argv[0]);
    }
    else
    {
        if (devcart_init(VID, PID))
        {
            atexit(devcart_close);
            signal(SIGINT, Signal);
            switch (function)
            {
            case FUNC_DOWNLOAD:
                devcart_download(pFilename, address, length);
                break;
            case FUNC_UPLOAD:
                devcart_upload(pFilename, address);
                break;
            case 3:
                devcart_execute(pFilename, address);
                break;
            }

            if (server)
            {
                server_run(server_dir);
            }
        }
    }

    return 0;
}

static void ParseNumericArg(const char *pArg, unsigned int *pResult)
{
    if (!strncmp(pArg, "0x", 2) || !strncmp(pArg, "0X", 2))
        sscanf(pArg, "%x", pResult);
    else
        sscanf(pArg, "%u", pResult);
}

static void PrintUsage(const char *pProgname)
{
    printf("Sega Saturn USB flash cart transfer utility\n");
    printf("by Anders Montonen, 2012 and Nathan Misner, 2020\n");
    printf("Usage: %s [-options] [-commands]\n\n", pProgname);
    printf("Options:\n");
    printf("    -v  <VID>                     Device VID (Default 0x0403)\n");
    printf("    -p  <PID>                     Device PID (Default 0x6001)\n");
    printf("\n");
    printf("Commands:\n");
    printf("    -d  <file>  <address>  <size> Download data to file\n");
    printf("    -u  <file>  <address>         Upload data from file\n");
    printf("    -x  <file>  <address>         Upload program and execute\n");
    printf("    -s  <directory>               Start debug fileserver & console\n");
    printf("USB IDs are given in hexadecimal, other arguments in decimal\n");
    printf("or hexadecimal (preceded by '0x')\n");
}

static void Signal(int sig)
{
    exit(EXIT_FAILURE);
}

//static void DoConsole(void)
//{
//    int ii;
//    int status = 0;
//
//    while (status >= 0)
//    {
//        // Read data in smaller chunks
//        status = ftdi_read_data(&device, recv_buf, 62);
//        if (status < 0)
//        {
//            printf("Read data error: %s\n",
//                   ftdi_get_error_string(&device));
//        }
//        else
//        {
//            for (ii = 0; ii < status; ++ii)
//            {
//                if (isprint(recv_buf[ii]) || isblank(recv_buf[ii]))
//                {
//                    putchar(recv_buf[ii]);
//                }
//            }
//        }
//    }
//}
