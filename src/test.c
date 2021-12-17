#include <stdio.h>
#include <stdarg.h>
#include "test.h"
#include "console.h"

/* printf wrapper that prints text green */
void printf_success(const char *format, ...)
{
    va_list args;

    va_start(args, format);
    
    console_setcolor(CONSOLE_GREEN);
    vprintf(format, args);
    console_reset();

    va_end(args);
}

/* printf wrapper that prints text red */
void printf_error(const char *format, ...)
{
    va_list args;

    va_start(args, format);

    console_setcolor(CONSOLE_RED);
    vprintf(format, args);
    console_reset();

    va_end(args);
}