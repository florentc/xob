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

#ifndef CONF_H
#define CONF_H

#include <stdio.h>

typedef struct
{
    unsigned char red;
    unsigned char green;
    unsigned char blue;
    unsigned char alpha;
} RGBA_color;

/**
 * The coloring of a bar.
 *
 * Colors storage order is 0xRRGGBBAA.
 */
typedef struct
{
    RGBA_color fg;
    RGBA_color bg;
    RGBA_color border;
} Colors;

typedef struct
{
    double rel;
    int abs;
} Dim;

typedef enum
{
    HIDDEN,
    PROPORTIONAL
} Overflow_mode;

typedef enum
{
    HORIZONTAL,
    VERTICAL
} Orientation;

typedef struct
{
    Dim x;
    Dim y;
    Dim length;
    int thickness;
    int border;
    int padding;
    int outline;
    Orientation orientation;
    Overflow_mode overflow;
    struct
    {
        Colors normal;
        Colors overflow;
        Colors alt;
        Colors altoverflow;
    } color;
} Style;

/* clang-format off */
#define DEFAULT_CONFIGURATION (Style) {\
        .x =\
        {\
            .rel = 1.0,\
            .abs = -48\
        },\
        .y =\
        {\
            .rel = 0.5,\
            .abs = 0\
        },\
        .length =\
        {\
            .rel = 0.3,\
            .abs = 0\
        },\
        .thickness = 24,\
        .border = 4,\
        .padding = 3,\
        .outline = 3,\
        .orientation = VERTICAL,\
        .overflow = PROPORTIONAL,\
        .color =\
        {\
            .normal =\
            {\
                .fg = \
                { \
                    .red   = 0xff,\
                    .green = 0xff,\
                    .blue  = 0xff,\
                    .alpha = 0xff\
                },\
                .bg = \
                { \
                    .red   = 0x00,\
                    .green = 0x00,\
                    .blue  = 0x00,\
                    .alpha = 0xff\
                },\
                .border = \
                { \
                    .red   = 0xff,\
                    .green = 0xff,\
                    .blue  = 0xff,\
                    .alpha = 0xff\
                }\
            },\
            .overflow =\
            {\
                .fg = \
                { \
                    .red   = 0xff,\
                    .green = 0x00,\
                    .blue  = 0x00,\
                    .alpha = 0xff\
                },\
                .bg = \
                { \
                    .red   = 0x00,\
                    .green = 0x00,\
                    .blue  = 0x00,\
                    .alpha = 0xff\
                },\
                .border = \
                { \
                    .red   = 0xff,\
                    .green = 0x00,\
                    .blue  = 0x00,\
                    .alpha = 0xff\
                }\
            },\
            .alt =\
            {\
                .fg = \
                { \
                    .red   = 0x55,\
                    .green = 0x55,\
                    .blue  = 0x55,\
                    .alpha = 0xff\
                },\
                .bg = \
                { \
                    .red   = 0x00,\
                    .green = 0x00,\
                    .blue  = 0x00,\
                    .alpha = 0xff\
                },\
                .border = \
                { \
                    .red   = 0x55,\
                    .green = 0x55,\
                    .blue  = 0x55,\
                    .alpha = 0xff\
                }\
            },\
            .altoverflow =\
            {\
                .fg = \
                { \
                    .red   = 0x55,\
                    .green = 0x00,\
                    .blue  = 0x00,\
                    .alpha = 0xff\
                },\
                .bg = \
                { \
                    .red   = 0x00,\
                    .green = 0x00,\
                    .blue  = 0x00,\
                    .alpha = 0xff\
                },\
                .border = \
                { \
                    .red   = 0x55,\
                    .green = 0x00,\
                    .blue  = 0x00,\
                    .alpha = 0xff\
                }\
            }\
        }\
    }
/* clang-format on */

#define DEFAULT_STYLE "default"
#define DEFAULT_CONFIG_APPNAME "xob"
#define DEFAULT_CONFIG_FILENAME "styles.cfg"

Style parse_style_config(FILE *filename, const char *stylename,
                         Style default_style);

#endif /* __CONF_H__ */
