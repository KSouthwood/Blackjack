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
 *  logger.h
 *
 *  Created on: Oct 24, 2018
 *      Author: Keri Southwood-Smith
 *
 *  Description:
 */


#ifndef LOGGER_H_
#define LOGGER_H_

/************
 * INCLUDES *
 ************/
#include "zlog.h"

/***********
 * DEFINES *
 ***********/
zlog_category_t *zc;

#define zinfo(msg, ...) zlog_info(zc, msg, ## __VA_ARGS__)
#define zdebug(msg, ...) zlog_debug(zc, msg, ## __VA_ARGS__)
#define zerror(msg, ...) zlog_error(zc, msg, ## __VA_ARGS__)
#define log_call() zlog_info(zc, "--==> %s() called. <==--", __func__)

/****************
 * DECLARATIONS *
 ****************/
int init_zlog(char *conf_file, char *category);
void end_zlog(void);

#endif /* LOGGER_H_ */
