#ifndef TEST_H
#define TEST_H

/* printf wrapper that prints text green */
void printf_success(const char *format, ...);
/* printf wrapper that prints text red */
void printf_error(const char *format, ...);

#endif