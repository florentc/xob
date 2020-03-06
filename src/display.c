/* xob - A lightweight overlay volume/anything bar for the X Window System.
 * Copyright (C) 2020 Florent Ch.
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

#include "display.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>

/* Keep value in range */
static int fit_in(int value, int min, int max)
{
    value = (value < min) ? min : value;
    value = (value > max) ? max : value;
    return value;
}

/* Horizontal and vertical size depending on orientation */
static int size_x(Geometry_context g)
{
    return g.orientation == HORIZONTAL ? g.length : g.thickness;
}
static int size_y(Geometry_context g)
{
    return g.orientation == HORIZONTAL ? g.thickness : g.length;
}

/* Get graphic context from a hex color specification string */
/* NULL is returned in case of a parse error. */
static GC gc_from_string(X_context x, const char *hexcolorstring)
{
    XColor color;
    GC gc = XCreateGC(x.display, x.window, 0, NULL);
    Colormap colormap = DefaultColormap(x.display, x.screen_number);
    if (XParseColor(x.display, colormap, hexcolorstring, &color) == 0)
    {
        gc = NULL;
    }
    else
    {
        XAllocColor(x.display, colormap, &color);
        XSetForeground(x.display, gc, color.pixel);
    }
    return gc;
}

/* Draw an empty bar with the given border color */
static void draw_empty(X_context x, Geometry_context g, Gc_colorset color)
{
    /* Outline */
    XFillRectangle(x.display, x.window, color.bg, 0, 0,
                   2 * (g.outline + g.border + g.padding) + size_x(g),
                   2 * (g.outline + g.border + g.padding) + size_y(g));
    /* Border */
    XFillRectangle(x.display, x.window, color.border, g.outline, g.outline,
                   2 * (g.border + g.padding) + size_x(g),
                   2 * (g.border + g.padding) + size_y(g));
    /* Padding */
    XFillRectangle(x.display, x.window, color.bg, g.outline + g.border,
                   g.outline + g.border, 2 * g.padding + size_x(g),
                   2 * g.padding + size_y(g));
}

/* Draw a given length of filled bar with the given color */
static void draw_content(X_context x, Geometry_context g, int filled_length,
                         GC color)
{
    if (g.orientation == HORIZONTAL)
    {
        XFillRectangle(
            x.display, x.window, color, g.outline + g.border + g.padding,
            g.outline + g.border + g.padding, filled_length, g.thickness);
    }
    else
    {
        XFillRectangle(
            x.display, x.window, color, g.outline + g.border + g.padding,
            g.outline + g.border + g.padding + g.length - filled_length,
            g.thickness, filled_length);
    }
}

/* Draw a separator (padding-sized gap) at the given position */
static void draw_separator(X_context x, Geometry_context g, int position,
                           GC color)
{
    if (g.orientation == HORIZONTAL)
    {
        XFillRectangle(x.display, x.window, color,
                       g.outline + g.border + (g.padding / 2) + position,
                       g.outline + g.border + g.padding, g.padding,
                       g.thickness);
    }
    else
    {
        XFillRectangle(
            x.display, x.window, color, g.outline + g.border + g.padding,
            g.outline + g.border + (g.padding / 2) + g.length - position,
            g.thickness, g.padding);
    }
}

/* PUBLIC Returns a new display context from a given configuration. If the
 * .x.display field of the returned display context is NULL, display could not
 * have been opened.*/
Display_context init(Style conf)
{
    Display_context dc;
    Window root;
    XSetWindowAttributes window_attributes;
    int topleft_x;
    int topleft_y;
    int fat_layer;
    int available_length;

    dc.x.display = XOpenDisplay(NULL);
    if (dc.x.display != NULL)
    {
        dc.x.screen_number = DefaultScreen(dc.x.display);
        dc.x.screen = ScreenOfDisplay(dc.x.display, dc.x.screen_number);
        root = RootWindow(dc.x.display, dc.x.screen_number);

        /* Override the window manager */
        window_attributes.override_redirect = True;

        /* Compute geometry */
        dc.geometry.outline = conf.outline;
        dc.geometry.border = conf.border;
        dc.geometry.padding = conf.padding;
        dc.geometry.thickness = conf.thickness;
        dc.geometry.orientation = conf.orientation;
        fat_layer =
            dc.geometry.padding + dc.geometry.border + dc.geometry.outline;

        /* Orientation-related dimensions */
        available_length = dc.geometry.orientation == HORIZONTAL
                               ? WidthOfScreen(dc.x.screen)
                               : HeightOfScreen(dc.x.screen);

        dc.geometry.length =
            fit_in(available_length * conf.length.rel + conf.length.abs, 0,
                   available_length - 2 * fat_layer);

        /* Compute position of the top-left corner */
        topleft_x = fit_in(WidthOfScreen(dc.x.screen) * conf.x.rel -
                               (size_x(dc.geometry) + 2 * fat_layer) / 2,
                           0,
                           WidthOfScreen(dc.x.screen) -
                               (size_x(dc.geometry) + 2 * fat_layer)) +
                    conf.x.abs;
        topleft_y = fit_in(HeightOfScreen(dc.x.screen) * conf.y.rel -
                               (size_y(dc.geometry) + 2 * fat_layer) / 2,
                           0,
                           HeightOfScreen(dc.x.screen) -
                               (size_y(dc.geometry) + 2 * fat_layer)) +
                    conf.y.abs;

        /* Creation of the window */
        dc.x.window = XCreateWindow(
            dc.x.display, root, topleft_x, topleft_y,
            size_x(dc.geometry) + 2 * fat_layer,
            size_y(dc.geometry) + 2 * fat_layer, 0,
            DefaultDepth(dc.x.display, dc.x.screen_number), CopyFromParent,
            DefaultVisual(dc.x.display, dc.x.screen_number), CWOverrideRedirect,
            &window_attributes);

        /* Set a WM_CLASS for the window */
        XClassHint *class_hint = XAllocClassHint();
        if (class_hint != NULL)
        {
            class_hint->res_name = DEFAULT_CONFIG_APPNAME;
            class_hint->res_class = DEFAULT_CONFIG_APPNAME;
            XSetClassHint(dc.x.display, dc.x.window, class_hint);
            XFree(class_hint);
        }

        /* The new window is not mapped yet */
        dc.x.mapped = False;

        /* Color context */
        dc.color.normal.fg = gc_from_string(dc.x, conf.color.normal.fg);
        dc.color.normal.bg = gc_from_string(dc.x, conf.color.normal.bg);
        dc.color.normal.border = gc_from_string(dc.x, conf.color.normal.border);
        dc.color.overflow.fg = gc_from_string(dc.x, conf.color.overflow.fg);
        dc.color.overflow.bg = gc_from_string(dc.x, conf.color.overflow.bg);
        dc.color.overflow.border =
            gc_from_string(dc.x, conf.color.overflow.border);
        dc.color.alt.fg = gc_from_string(dc.x, conf.color.alt.fg);
        dc.color.alt.bg = gc_from_string(dc.x, conf.color.alt.bg);
        dc.color.alt.border = gc_from_string(dc.x, conf.color.alt.border);
        dc.color.altoverflow.fg =
            gc_from_string(dc.x, conf.color.altoverflow.fg);
        dc.color.altoverflow.bg =
            gc_from_string(dc.x, conf.color.altoverflow.bg);
        dc.color.altoverflow.border =
            gc_from_string(dc.x, conf.color.altoverflow.border);
    }

    return dc;
}

/* PUBLIC Show a bar filled at value/cap in normal or alternative mode */
Display_context show(Display_context dc, int value, int cap,
                     Overflow_mode overflow_mode, Show_mode show_mode)
{
    Display_context newdc = dc;

    Gc_colorset colorset;
    Gc_colorset colorset_overflow_proportional;

    if (!dc.x.mapped)
    {
        XMapWindow(dc.x.display, dc.x.window);
        XRaiseWindow(dc.x.display, dc.x.window);
        newdc.x.mapped = True;
    }

    switch (show_mode)
    {
    case NORMAL:
        colorset_overflow_proportional = dc.color.normal;
        if (value <= cap)
            colorset = dc.color.normal;
        else
            colorset = dc.color.overflow;
        break;

    case ALTERNATIVE:
        colorset_overflow_proportional = dc.color.alt;
        if (value <= cap)
            colorset = dc.color.alt;
        else
            colorset = dc.color.altoverflow;
        break;
    }

    /* Empty bar */
    draw_empty(dc.x, dc.geometry, colorset);

    /* Content */
    draw_content(dc.x, dc.geometry,
                 fit_in(value, 0, cap) * dc.geometry.length / cap, colorset.fg);

    /* Proportional overflow : draw separator */
    if (value > cap && overflow_mode == PROPORTIONAL &&
        cap * dc.geometry.length / value > dc.geometry.padding)
    {
        draw_content(dc.x, dc.geometry, cap * dc.geometry.length / value,
                     colorset_overflow_proportional.fg);
        draw_separator(dc.x, dc.geometry, cap * dc.geometry.length / value,
                       colorset.bg);
    }

    XFlush(dc.x.display);

    return newdc;
}

/* PUBLIC Hide the window */
Display_context hide(Display_context dc)
{
    Display_context newdc = dc;

    if (dc.x.mapped)
    {
        XUnmapWindow(dc.x.display, dc.x.window);
        newdc.x.mapped = False;
        XFlush(dc.x.display);
    }

    return newdc;
}
