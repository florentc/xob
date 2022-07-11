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

#include "display.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xrandr.h>
#include <stdlib.h>
#include <string.h>

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

/* Draw an empty bar with the given colors */
static void draw_empty(X_context x, Geometry_context g, Colors colors)
{
    /* Outline */
    fill_rectangle(x, colors.bg, 0, 0,
                   2 * (g.outline + g.border + g.padding) + size_x(g),
                   2 * (g.outline + g.border + g.padding) + size_y(g));
    /* Border */
    fill_rectangle(x, colors.border, g.outline, g.outline,
                   2 * (g.border + g.padding) + size_x(g),
                   2 * (g.border + g.padding) + size_y(g));
    /* Padding */
    fill_rectangle(x, colors.bg, g.outline + g.border, g.outline + g.border,
                   2 * g.padding + size_x(g), 2 * g.padding + size_y(g));
}

/* Draw a given length of filled bar with the given color */
static void draw_content(X_context x, Geometry_context g, int filled_length,
                         Color color)
{
    if (g.orientation == HORIZONTAL)
    {
        fill_rectangle(x, color, g.outline + g.border + g.padding,
                       g.outline + g.border + g.padding, filled_length,
                       g.thickness);
    }
    else
    {
        fill_rectangle(x, color, g.outline + g.border + g.padding,
                       g.outline + g.border + g.padding + g.length -
                           filled_length,
                       g.thickness, filled_length);
    }
}

/* Draw a separator (padding-sized gap) at the given position */
static void draw_separator(X_context x, Geometry_context g, int position,
                           Color color)
{
    if (g.orientation == HORIZONTAL)
    {
        fill_rectangle(
            x, color, g.outline + g.border + (g.padding / 2) + position,
            g.outline + g.border + g.padding, g.padding, g.thickness);
    }
    else
    {
        fill_rectangle(x, color, g.outline + g.border + g.padding,
                       g.outline + g.border + (g.padding / 2) + g.length -
                           position,
                       g.thickness, g.padding);
    }
}

void compute_geometry(Style conf, Display_context *dc, int *topleft_x,
                      int *topleft_y, int *fat_layer, int *available_length)
{
    dc->geometry.outline = conf.outline;
    dc->geometry.border = conf.border;
    dc->geometry.padding = conf.padding;
    dc->geometry.thickness = conf.thickness;
    dc->geometry.orientation = conf.orientation;
    dc->geometry.length_dynamic.rel = conf.length.rel;
    dc->geometry.length_dynamic.abs = conf.length.abs;

    *fat_layer =
        dc->geometry.padding + dc->geometry.border + dc->geometry.outline;

    /* Orientation-related dimensions */
    *available_length = dc->geometry.orientation == HORIZONTAL
                            ? dc->x.monitor_info.width
                            : dc->x.monitor_info.height;

    dc->geometry.length =
        fit_in(*available_length * conf.length.rel + conf.length.abs, 0,
               *available_length - 2 * *fat_layer);

    /* Compute position of the top-left corner */
    *topleft_x = fit_in(dc->x.monitor_info.width * conf.x.rel -
                            (size_x(dc->geometry) + 2 * *fat_layer) / 2,
                        0,
                        dc->x.monitor_info.width -
                            (size_x(dc->geometry) + 2 * *fat_layer)) +
                 conf.x.abs + dc->x.monitor_info.x;
    *topleft_y = fit_in(dc->x.monitor_info.height * conf.y.rel -
                            (size_y(dc->geometry) + 2 * *fat_layer) / 2,
                        0,
                        dc->x.monitor_info.height -
                            (size_y(dc->geometry) + 2 * *fat_layer)) +
                 conf.y.abs + dc->x.monitor_info.y;
}

/* Set combined positon */
static void set_combined_position(Display_context *pdc)
{
    pdc->x.monitor_info.x = 0;
    pdc->x.monitor_info.y = 0;
    pdc->x.monitor_info.width = WidthOfScreen(pdc->x.screen);
    pdc->x.monitor_info.height = HeightOfScreen(pdc->x.screen);
    // strcpy(pdc->x.monitor_info.name, MONITOR_COMBINED);
}

/* Set bar size relative to the first monitor */
static void set_relative_position(Display_context *pdc)
{
    Window root = RootWindow(pdc->x.display, pdc->x.screen_number);

    /* Get monitors info */
    int num_monitors;
    XRRMonitorInfo *monitor_sizes =
        XRRGetMonitors(pdc->x.display, root, 0, &num_monitors);

    /* Set monitor_info for the first monitor */
    if (num_monitors != 0)
    {
        pdc->x.monitor_info.x = monitor_sizes[0].x;
        pdc->x.monitor_info.y = monitor_sizes[0].y;
        pdc->x.monitor_info.width = monitor_sizes[0].width;
        pdc->x.monitor_info.height = monitor_sizes[0].height;
    }
    else // Monitor name is not found
    {
        /* Use combined surface for monitor option if no monitors with
         * provided name found */
        fprintf(stderr, "Error: Can't get monitors info.\n");
        fprintf(stderr, "Info: falling back to combined mode.\n");
        set_combined_position(pdc);
        // strcpy(pdc.x.monitor_info.name, MONITOR_COMBINED);
    }
    XRRFreeMonitors(monitor_sizes);
}

/* Set specified monitor */
static void set_specified_position(Display_context *pdc, const Style *pconf)
{
    Window root = RootWindow(pdc->x.display, pdc->x.screen_number);

    /* Get monitors info */
    int num_monitors;
    char *monitor_name;
    XRRMonitorInfo *monitor_sizes =
        XRRGetMonitors(pdc->x.display, root, 0, &num_monitors);

    /* Compare monitors output names */
    int i;
    for (i = 0; i < num_monitors; i++)
    {
        monitor_name = XGetAtomName(pdc->x.display, monitor_sizes[i].name);
        if (strcmp(pconf->monitor, monitor_name) == 0)
            break;
    }
    if (i != num_monitors)
    {
        pdc->x.monitor_info.x = monitor_sizes[i].x;
        pdc->x.monitor_info.y = monitor_sizes[i].y;
        pdc->x.monitor_info.width = monitor_sizes[i].width;
        pdc->x.monitor_info.height = monitor_sizes[i].height;
        strcpy(pdc->x.monitor_info.name, monitor_name);
    }
    else // Monitor name is not found
    {
        /* Use combined surface for monitor option if no monitors with
         * provided name found */
        fprintf(stderr, "Error: monitor %s is not found.\n", pconf->monitor);
        fprintf(stderr, "Info: falling back to combined mode.\n");
        set_combined_position(pdc);
        // strcpy(pdc.x.monitor_info.name, MONITOR_COMBINED);
    }
    XRRFreeMonitors(monitor_sizes);
}

/* Move and resize the bar relative to a monitor with provided coords */
static void move_resize_to_coords_monitor(Display_context *pdc, int x, int y)
{
    int fat_layer, available_length, bar_size_x, bar_size_y, i;
    int topleft_x, topleft_y;
    int num_monitors;
    XRRMonitorInfo *monitor_sizes;
    fat_layer =
        pdc->geometry.padding + pdc->geometry.border + pdc->geometry.outline;

    monitor_sizes = XRRGetMonitors(
        pdc->x.display, RootWindow(pdc->x.display, pdc->x.screen_number), 0,
        &num_monitors);
    for (i = 0; i < num_monitors; i++)
    {
        /* Find monitor by coordinates of focused window */
        if (x > monitor_sizes[i].x &&
            x < monitor_sizes[i].x + monitor_sizes[i].width &&
            y > monitor_sizes[i].y &&
            y < monitor_sizes[i].y + monitor_sizes[i].height)
        {
            /* Recalculate bar sizes */
            available_length = pdc->geometry.orientation == HORIZONTAL
                                   ? monitor_sizes[i].width
                                   : monitor_sizes[i].height;

            pdc->geometry.length =
                fit_in(available_length * pdc->geometry.length_dynamic.rel +
                           pdc->geometry.length_dynamic.abs,
                       0, available_length - 2 * fat_layer);

            bar_size_x = size_x(pdc->geometry) + 2 * fat_layer;
            bar_size_y = size_y(pdc->geometry) + 2 * fat_layer;

            /* Recalculate bar position */
            topleft_x = fit_in(monitor_sizes[i].width * pdc->geometry.x.rel -
                                   bar_size_x / 2,
                               0, monitor_sizes[i].width - bar_size_x) +
                        pdc->geometry.x.abs + monitor_sizes[i].x;

            topleft_y = fit_in(monitor_sizes[i].height * pdc->geometry.y.rel -
                                   bar_size_y / 2,
                               0, monitor_sizes[i].height - bar_size_y) +
                        pdc->geometry.y.abs + monitor_sizes[i].y;

            /* Move and resize bar */
            XMoveResizeWindow(pdc->x.display, pdc->x.window, topleft_x,
                              topleft_y, bar_size_x, bar_size_y);
            break;
        }
    }

    XRRFreeMonitors(monitor_sizes);
}

/* Mobe the bar to monitor with focused window */
static void move_resize_to_focused_monitor(Display_context *pdc)
{
    int revert_to_window;
    int focused_x, focused_y;

    Window focused_window, fchild_window;

    XGetInputFocus(pdc->x.display, &focused_window, &revert_to_window);

    /* Get coords of focused window */
    XTranslateCoordinates(pdc->x.display, focused_window,
                          RootWindow(pdc->x.display, pdc->x.screen_number), 0,
                          0, &focused_x, &focused_y, &fchild_window);

    move_resize_to_coords_monitor(pdc, focused_x, focused_y);
}

/* Move the bar to monitor with pointer */
static void move_resize_to_pointer_monitor(Display_context *pdc)
{
    int pointer_x, pointer_y, win_x, win_y;
    unsigned int p_mask;
    Window p_root, p_child;

    XQueryPointer(pdc->x.display,
                  RootWindow(pdc->x.display, pdc->x.screen_number), &p_root,
                  &p_child, &pointer_x, &pointer_y, &win_x, &win_y, &p_mask);

    move_resize_to_coords_monitor(pdc, pointer_x, pointer_y);
}

/* PUBLIC Returns a new display context from a given configuration. If the
 * .x.display field of the returned display context is NULL, display could not
 * have been opened.*/
Display_context init(Style conf)
{
    Display_context dc;
    Depth dc_depth;
    Window root;
    int topleft_x;
    int topleft_y;
    int fat_layer;
    int available_length;
    XSetWindowAttributes window_attributes;
    static long window_attributes_flags =
        CWColormap | CWBorderPixel | CWOverrideRedirect;

    dc.x.display = XOpenDisplay(NULL);
    if (dc.x.display != NULL)
    {
        dc.x.screen_number = DefaultScreen(dc.x.display);
        dc.x.screen = ScreenOfDisplay(dc.x.display, dc.x.screen_number);
        root = RootWindow(dc.x.display, dc.x.screen_number);

        dc_depth = get_display_context_depth(dc);

        window_attributes.colormap =
            XCreateColormap(dc.x.display, root, dc_depth.visuals, AllocNone);
        window_attributes.border_pixel = 0;
        window_attributes.override_redirect = True;

        /* Write bar position relative data to X_context */
        dc.geometry.x.rel = conf.x.rel;
        dc.geometry.x.abs = conf.x.abs;
        dc.geometry.y.rel = conf.y.rel;
        dc.geometry.y.abs = conf.y.abs;

        /* Get bar position from conf */
        if (strcmp(conf.monitor, MONITOR_RELATIVE_FOCUS) == 0)
            dc.geometry.bar_position = POSITION_RELATIVE_FOCUS;
        else if (strcmp(conf.monitor, MONITOR_RELATIVE_POINTER) == 0)
            dc.geometry.bar_position = POSITION_RELATIVE_POINTER;
        else if (strcmp(conf.monitor, MONITOR_COMBINED) == 0)
            dc.geometry.bar_position = POSITION_COMBINED;
        else
            dc.geometry.bar_position = POSITION_SPECIFIED;

        switch (dc.geometry.bar_position)
        {
        case POSITION_RELATIVE_FOCUS:
        case POSITION_RELATIVE_POINTER:
            set_relative_position(&dc);
            break;
        case POSITION_COMBINED:
            set_combined_position(&dc);
            break;
        case POSITION_SPECIFIED:
            set_specified_position(&dc, &conf);
            break;
        default:
            fprintf(stderr, "Error: in switch position\n");
            break;
        }

        compute_geometry(conf, &dc, &topleft_x, &topleft_y, &fat_layer,
                         &available_length);

        /* Creation of the window */
        dc.x.window = XCreateWindow(
            dc.x.display, root, topleft_x, topleft_y,
            size_x(dc.geometry) + 2 * fat_layer,
            size_y(dc.geometry) + 2 * fat_layer, 0, dc_depth.depth, InputOutput,
            dc_depth.visuals, window_attributes_flags, &window_attributes);

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

        /* Colorscheme */
        dc.colorscheme = conf.colorscheme;
    }

    return dc;
}

/* PUBLIC Cleans the X memory buffers. */
void display_context_destroy(Display_context dc)
{
    XCloseDisplay(dc.x.display);
}

/* PUBLIC Show a bar filled at value/cap in normal or alternative mode */
Display_context show(Display_context dc, int value, int cap,
                     Overflow_mode overflow_mode, Show_mode show_mode)
{
    Display_context newdc = dc;

    Colors colors;
    Colors colors_overflow_proportional;

    /* Move the bar for relative positions */
    // switch (dc.geometry.bar_position)
    switch (dc.geometry.bar_position)
    {
    case POSITION_RELATIVE_FOCUS:
        move_resize_to_focused_monitor(&dc);
        break;
    case POSITION_RELATIVE_POINTER:
        move_resize_to_pointer_monitor(&dc);
        break;
    case POSITION_COMBINED:
    case POSITION_SPECIFIED:
        break;
    }

    if (!dc.x.mapped)
    {
        XMapWindow(dc.x.display, dc.x.window);
        XRaiseWindow(dc.x.display, dc.x.window);
        newdc.x.mapped = True;
    }

    switch (show_mode)
    {
    case NORMAL:
        colors_overflow_proportional = dc.colorscheme.normal;
        if (value <= cap)
            colors = dc.colorscheme.normal;
        else
            colors = dc.colorscheme.overflow;
        break;

    case ALTERNATIVE:
        colors_overflow_proportional = dc.colorscheme.alt;
        if (value <= cap)
            colors = dc.colorscheme.alt;
        else
            colors = dc.colorscheme.altoverflow;
        break;
    }

    /* Empty bar */
    draw_empty(dc.x, dc.geometry, colors);

    /* Content */
    draw_content(dc.x, dc.geometry,
                 fit_in(value, 0, cap) * dc.geometry.length / cap, colors.fg);

    /* Proportional overflow : draw separator */
    if (value > cap && overflow_mode == PROPORTIONAL &&
        cap * dc.geometry.length / value > dc.geometry.padding)
    {
        draw_content(dc.x, dc.geometry, cap * dc.geometry.length / value,
                     colors_overflow_proportional.fg);
        draw_separator(dc.x, dc.geometry, cap * dc.geometry.length / value,
                       colors.bg);
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
