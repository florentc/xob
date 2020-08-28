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
#define CONFIG_INTERNALS 1

#include "conf.h"
#include <errno.h>
#include <libconfig.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int config_setting_lookup_float_or_int(const config_setting_t *setting,
                                              const char *name, double *value)
{
    int fallback_value;
    int success_status;

    success_status = CONFIG_FALSE;

    if (config_setting_lookup_float(setting, name, value))
    {
        success_status = CONFIG_TRUE;
    }
    else if (config_setting_lookup_int(setting, name, &fallback_value))
    {
        *value = (double)fallback_value;
        success_status = CONFIG_TRUE;
    }

    return success_status;
}

static int config_setting_lookup_dim(const config_setting_t *setting,
                                     const char *name, Dim *value)
{
    double rel;
    int abs;
    int success_status;
    config_setting_t *dim_setting;

    success_status = CONFIG_FALSE;
    dim_setting = config_setting_get_member(setting, name);

    if (dim_setting != NULL)
    {
        if (config_setting_lookup_float_or_int(dim_setting, "relative", &rel))
        {
            if (rel >= 0 && rel <= 1)
            {
                value->rel = rel;
                success_status = CONFIG_TRUE;

                if (config_setting_lookup_int(dim_setting, "offset", &abs))
                {
                    value->abs = abs;
                }
            }
            else
            {
                fprintf(stderr,
                        "Error: in configuration, line %d - "
                        "Out of range relative value.\n",
                        config_setting_source_line(dim_setting));
            }
        }
    }

    return success_status;
}

static _Bool color_spec_is_valid(const char *spec)
{
    if (spec[0] == '#')
        return 1;
    return 0;
}

/*
unsigned char *parse_hex(const char *hex)
{
    char *cur = (char *)hex;

    const unsigned char n = strlen(cur);
    static unsigned char rgba[4];
    if (*cur == '#' && (n == 6 + 1 || n == 8 + 1))
    {
        cur++;
        for (int i = 0; i < 4; i++)
        {
            if (i == 3 && *cur == 0)
            {
                rgba[3] = 255;
                break;
            }
            for (int j = 0; j < 2; j++)
            {
                rgba[i] <<= 4;
                if (*cur >= '0' && *cur <= '9')
                    rgba[i] |= *cur - '0';
                else if (*cur >= 'A' && *cur <= 'F')
                    rgba[i] |= *cur - ('A' - 10);
                else if (*cur >= 'a' && *cur <= 'f')
                    rgba[i] |= *cur - ('a' - 10);
                else
                {
                    cur = NULL;
                    break;
                }
                cur++;
            }
            if (cur == NULL)
            {
                return NULL;
            }
        }
    }
    return rgba;
}
*/

static Color parse_color(const char *spec)
{
    char color_hex_chars[8];
    char *end;

    strncpy((char *)color_hex_chars, spec + 1, 8);

    unsigned int color =
        (unsigned int)strtoul((char *)color_hex_chars, &end, 16);
    if (end - (char *)color_hex_chars <= 2 + 2 + 2)
        color = (color << 8) | 0xff; // 00.XX.XX.XX -> XX.XX.XX.FF
    Color result = {.red = ((color >> 24) & 0xff),
                    .green = ((color >> 16) & 0xff),
                    .blue = ((color >> 8) & 0xff),
                    .alpha = ((color >> 0) & 0xff)};
    return result;
}

static int config_setting_lookup_color(const config_setting_t *setting,
                                       const char *name, Color *value)
{
    const char *colorstring;
    int success_status = CONFIG_FALSE;

    if (config_setting_lookup_string(setting, name, &colorstring))
    {
        errno = 0;
        if (color_spec_is_valid(colorstring))
        {
            *value = parse_color(colorstring);
            success_status = CONFIG_TRUE;
        }
        else
        {
            fprintf(stderr,
                    "Error: in configuration, line %d - "
                    "Invalid color specification.\n",
                    config_setting_source_line(setting));
        }
    }

    return success_status;
}

static int config_setting_lookup_colors(const config_setting_t *setting,
                                        const char *name, Colors *value)
{
    config_setting_t *colorspec_setting;
    int success_status = CONFIG_FALSE;

    colorspec_setting = config_setting_get_member(setting, name);

    if (colorspec_setting != NULL)
    {
        success_status = config_setting_lookup_color(colorspec_setting, "fg",
                                                     &(value->fg)) &&
                         config_setting_lookup_color(colorspec_setting, "bg",
                                                     &(value->bg)) &&
                         config_setting_lookup_color(
                             colorspec_setting, "border", &(value->border));
    }

    return success_status;
}

static int config_setting_lookup_overflowmode(const config_setting_t *setting,
                                              const char *name,
                                              Overflow_mode *value)
{
    const char *stringvalue;
    int success_status = CONFIG_FALSE;

    if (config_setting_lookup_string(setting, name, &stringvalue))
    {
        if (strcmp(stringvalue, "hidden") == 0)
        {
            *value = HIDDEN;
            success_status = CONFIG_TRUE;
        }
        else if (strcmp(stringvalue, "proportional") == 0)
        {
            *value = PROPORTIONAL;
            success_status = CONFIG_TRUE;
        }
        else
        {
            fprintf(stderr,
                    "Error: in configuration, line %d - "
                    "Invalid overflow mode. Expected \"hidden\" or "
                    "\"proportional\"\n",
                    config_setting_source_line(setting));
        }
    }

    return success_status;
}

static int config_setting_lookup_orientation(const config_setting_t *setting,
                                             const char *name,
                                             Orientation *value)
{
    const char *stringvalue;
    int success_status = CONFIG_FALSE;

    if (config_setting_lookup_string(setting, name, &stringvalue))
    {
        if (strcmp(stringvalue, "horizontal") == 0)
        {
            *value = HORIZONTAL;
            success_status = CONFIG_TRUE;
        }
        else if (strcmp(stringvalue, "vertical") == 0)
        {
            *value = VERTICAL;
            success_status = CONFIG_TRUE;
        }
        else
        {
            fprintf(stderr,
                    "Error: in configuration, line %d - "
                    "Invalid orientation. Expected \"horizontal\" or "
                    "\"vertical\".\n",
                    config_setting_source_line(setting));
        }
    }

    return success_status;
}

Style parse_style_config(FILE *file, const char *stylename, Style default_style)
{
    config_t config;
    config_init(&config);

    config_setting_t *xob_config;
    config_setting_t *color_config;
    Style style = default_style;

    if (config_read(&config, file))
    {
        xob_config = config_lookup(&config, stylename);
        if (xob_config != NULL)
        {
            config_setting_lookup_int(xob_config, "thickness",
                                      &style.thickness);
            config_setting_lookup_int(xob_config, "border", &style.border);
            config_setting_lookup_int(xob_config, "padding", &style.padding);
            config_setting_lookup_int(xob_config, "outline", &style.outline);
            config_setting_lookup_dim(xob_config, "x", &style.x);
            config_setting_lookup_dim(xob_config, "y", &style.y);
            config_setting_lookup_dim(xob_config, "length", &style.length);
            config_setting_lookup_orientation(xob_config, "orientation",
                                              &style.orientation);
            config_setting_lookup_overflowmode(xob_config, "overflow",
                                               &style.overflow);
            color_config = config_setting_get_member(xob_config, "color");
            if (color_config != NULL)
            {
                config_setting_lookup_colors(color_config, "normal",
                                             &style.color.normal);
                config_setting_lookup_colors(color_config, "overflow",
                                             &style.color.overflow);
                config_setting_lookup_colors(color_config, "alt",
                                             &style.color.alt);
                config_setting_lookup_colors(color_config, "altoverflow",
                                             &style.color.altoverflow);
            }
        }
        else
        {
            fprintf(stderr, "Error: No style %s.\n", stylename);
        }
    }
    else
    {
        fprintf(stderr, "Error: in configuration, line %d - %s\n",
                config_error_line(&config), config_error_text(&config));
    }

    config_destroy(&config);
    return style;
}
