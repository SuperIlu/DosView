/*
MIT License

Copyright (c) 2023 Andre Seidelt <superilu@yahoo.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef __MAIN_H__
#define __MAIN_H__

#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include "allegro.h"

#define LOGSTREAM stdout  //!< output stream for logging on DOS

#ifdef DEBUG_ENABLED
//! printf-style debug message to logfile/console
#define DEBUGF(str, ...)                               \
    fprintf(LOGSTREAM, "[DEBUG] " str, ##__VA_ARGS__); \
    fflush(LOGSTREAM);

//! print debug message to logfile/console
#define DEBUG(str)                    \
    fputs("[DEBUG] " str, LOGSTREAM); \
    fflush(LOGSTREAM);

#else
#define DEBUGF(str, ...)
#define DEBUG(str)
#endif

#define PRINTERR(...)               \
    fprintf(stdout, ##__VA_ARGS__); \
    fflush(stdout);

extern int output_quality;

#endif  // __MAIN_H__
