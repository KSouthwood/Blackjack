/***********************************************************************************
 *  MIT License                                                                    *
 *                                                                                 *
 *  Copyright (c) 2018 Keri Southwood-Smith                                        *
 *                                                                                 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy   *
 *  of this software and associated documentation files (the "Software"), to deal  *
 *  in the Software without restriction, including without limitation the rights   *
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell      *
 *  copies of the Software, and to permit persons to whom the Software is          *
 *  furnished to do so, subject to the following conditions:                       *
 *                                                                                 *
 *  The above copyright notice and this permission notice shall be included in all *
 *  copies or substantial portions of the Software.                                *
 *                                                                                 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR     *
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,       *
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE    *
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER         *
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,  *
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE  *
 *  SOFTWARE.                                                                      *
 ***********************************************************************************/

/*
 *  logger.c
 *
 *  Created on: Oct 24, 2018
 *      Author: Keri Southwood-Smith
 *
 *  Description:
 */


/************
 * INCLUDES *
 ************/
#include "logger.h"

/****************
 * DECLARATIONS *
 ****************/

/***************
 *  Summary: Set up the logging functions
 *
 *  Description: Initialize the logging functions, and set up the categories.
 *
 *  Parameter(s):
 *      N/A
 *
 *  Returns:
 *      N/A
 */
int init_zlog(char *conf_file, char *category)
{
    if (zlog_init(conf_file))
    {
        printf("zlog init failed\n");
        return -1;
    }

    zc = zlog_get_category(category);
    if (!zc)
    {
        printf("zlog get cat failed.\n");
        zlog_fini();
        return -2;
    }

    zlog_debug(zc, "");
    zlog_debug(zc, "******************************");
    zlog_debug(zc, "     Logging initialized.");
    zlog_debug(zc, "******************************");
    zlog_debug(zc, "");

    return 0;
}

void end_zlog(void)
{
    zlog_debug(zc, "");
    zlog_debug(zc, "******************************");
    zlog_debug(zc, "      Logging finished.");
    zlog_debug(zc, "******************************");
    zlog_debug(zc, "");

    zlog_fini();

    return;
}
