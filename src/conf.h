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

/**
 * The coloring of a bar.
 *
 * Colors storage order is 0xRRGGBBAA.
 */
typedef struct
{
    unsigned int fg;
    unsigned int bg;
    unsigned int border;
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
                .fg     = 0xffffff,\
                .bg     = 0x000000,\
                .border = 0xffffff\
            },\
            .overflow =\
            {\
                .fg     = 0xff0000,\
                .bg     = 0x000000,\
                .border = 0xff0000\
            },\
            .alt =\
            {\
                .fg     = 0x555555,\
                .bg     = 0x000000,\
                .border = 0x555555\
            },\
            .altoverflow =\
            {\
                .fg     = 0x550000,\
                .bg     = 0x000000,\
                .border = 0x550000\
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
