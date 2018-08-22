/* xob - A lightweight overlay volume/anything bar for the X Window System.
 * Copyright (C) 2018 Florent Ch.
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
} show_mode_t;

typedef struct
{
    Display *display;
    int screen_number;
    Screen *screen;
    Window window;
    Bool mapped;
} x_context_t;

typedef struct
{
    GC fg;
    GC bg;
    GC border;
} gc_colorset_t;

typedef struct
{
    gc_colorset_t normal;
    gc_colorset_t overflow;
    gc_colorset_t alt;
    gc_colorset_t altoverflow;
} color_context_t;

typedef struct
{
    int outline;
    int border;
    int padding;
    int width;
    int height;
} geometry_context_t;

typedef struct
{
    x_context_t x;
    color_context_t color;
    geometry_context_t geometry;
} display_context_t;

display_context_t init(style_t conf);
display_context_t show(display_context_t dc, int value, int cap,
                       overflow_mode_t overflow_mode, show_mode_t show_mode);
display_context_t hide(display_context_t dc);

#endif /* __DISPLAY_H__ */
