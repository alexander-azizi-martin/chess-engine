/* console.h -- console C89 header file
 *
 * Copyright (C) 2016 Christian Winkler
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef CONSOLE_H
#define CONSOLE_H

#ifdef __cplusplus
extern "C" {
#endif

enum Color {
    CONSOLE_BLACK,
    CONSOLE_NAVY,
    CONSOLE_GREEN,
    CONSOLE_TEAL,
    CONSOLE_MAROON,
    CONSOLE_PURPLE,
    CONSOLE_OLIVE,
    CONSOLE_SILVER,
    CONSOLE_GREY,
    CONSOLE_BLUE,
    CONSOLE_LIME,
    CONSOLE_AQUA,
    CONSOLE_RED,
    CONSOLE_PINK,
    CONSOLE_YELLOW,
    CONSOLE_WHITE
};
/* clear console content */
void console_clear();
/* reset foreground and background color */
void console_reset();
/* set foreground color */
void console_setcolor(enum Color color);
/* set background color */
void console_setbgcolor(enum Color color);
/* set cursor position */
void console_setcurpos(int x, int y);

#ifdef __cplusplus
}
#endif

#endif /* CONSOLE_H */