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

#ifndef MAIN_H
#define MAIN_H

#include "display.h"
#include <stdbool.h>

#define VERSION_NUMBER "0.1.1"

typedef struct
{
    bool valid;
    int value;
    Show_mode show_mode;
} Input_Value;

Input_Value parse_input(void);

#endif
