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

static int config_setting_lookup_color(const config_setting_t *setting,
                                       const char *name, const char **value)
{
    const char *colorstring;
    char *endptr;
    unsigned int color;
    int success_status = CONFIG_FALSE;

    if (config_setting_lookup_string(setting, name, &colorstring))
    {
        errno = 0;
        color = (unsigned int) strtol(colorstring + 1, &endptr, 16);
        char cslen = strlen(colorstring);
        if (cslen >= 7 && cslen <= 9 && colorstring[0] == '#' && errno == 0 &&
            strlen(endptr) == 0 && color <= 0xFFFFFFFF)
        {
            *value = colorstring;
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

static int config_setting_lookup_colorspec(const config_setting_t *setting,
                                           const char *name, Colorspec *value)
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

Style parse_style_config(FILE *file, const char *stylename, Style default_style, Style_config config)
{
    config_setting_t *style_config;
    config_setting_t *color_config;
    Style style = default_style;

    if (config_read(config, file))
    {
        style_config = config_lookup(config, stylename);
        if (style_config != NULL)
        {
            config_setting_lookup_int(style_config, "thickness",
                                      &style.thickness);
            config_setting_lookup_int(style_config, "border", &style.border);
            config_setting_lookup_int(style_config, "padding", &style.padding);
            config_setting_lookup_int(style_config, "outline", &style.outline);
            config_setting_lookup_dim(style_config, "x", &style.x);
            config_setting_lookup_dim(style_config, "y", &style.y);
            config_setting_lookup_dim(style_config, "length", &style.length);
            config_setting_lookup_orientation(style_config, "orientation",
                                              &style.orientation);
            config_setting_lookup_overflowmode(style_config, "overflow",
                                               &style.overflow);
            color_config = config_setting_get_member(style_config, "color");
            if (color_config != NULL)
            {
                config_setting_lookup_colorspec(color_config, "normal",
                                                &style.color.normal);
                config_setting_lookup_colorspec(color_config, "overflow",
                                                &style.color.overflow);
                config_setting_lookup_colorspec(color_config, "alt",
                                                &style.color.alt);
                config_setting_lookup_colorspec(color_config, "altoverflow",
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
                config_error_line(config), config_error_text(config));
    }

    return style;
}

Style_config style_config_init()
{
    Style_config c = malloc(sizeof(config_t));
    config_init(c);
    return c;
}

void style_config_destroy(Style_config c)
{
    if(c)
    {
        config_destroy(c);
        free(c);
        c = NULL;
    }
}