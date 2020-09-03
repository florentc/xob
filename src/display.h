/* xob - A lightweight overlay volume/anything bar for the X Window System.
 * Copyright (C) 2021 Florent Ch.
 *
 * xob is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * xob is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with xob.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef DISPLAY_H
#define DISPLAY_H

#include "conf.h"
#include <X11/Xlib.h>

typedef enum
{
    NORMAL,
    ALTERNATIVE
} Show_mode;

typedef struct
{
    Display *display;
    int screen_number;
    Screen *screen;
    Window window;
    Bool mapped;
} X_context;

typedef struct
{
    Color fg;
    Color bg;
    Color border;
} Gc_colorset;

typedef struct
{
    Gc_colorset normal;
    Gc_colorset overflow;
    Gc_colorset alt;
    Gc_colorset altoverflow;
} Color_context;

typedef struct
{
    int outline;
    int border;
    int padding;
    int length;
    int thickness;
    Orientation orientation;
} Geometry_context;

typedef struct
{
    X_context x;
    Color_context color;
    Geometry_context geometry;
} Display_context;

Display_context init(Style conf);
Display_context show(Display_context dc, int value, int cap,
                     Overflow_mode overflow_mode, Show_mode show_mode);
Display_context hide(Display_context dc);
void display_context_destroy(Display_context dc);

/* Draw a rectangle with the given size, position and color */
void fill_rectangle(X_context xc, Color c, int x, int y, unsigned int w,
                    unsigned int h);

Depth get_display_context_depth(Display_context dc);

#endif /* __DISPLAY_H__ */
