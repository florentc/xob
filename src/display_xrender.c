/* xob - A lightweight overlay volume/anything bar for the X Window System.
 * Copyright (C) 2021 Florent Ch.
 * Copyright (C) 2021 Oliver Hattshire
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
#include <X11/extensions/Xrender.h>
#include <stdlib.h>

static Bool is_alpha_visual(Display_context dc, Visual *visual)
{
    XRenderPictFormat *fmt = XRenderFindVisualFormat(dc.x.display, visual);
    return (fmt->type == PictTypeDirect && fmt->direct.alphaMask);
}

static Depth get_alpha_depth_if_available(Display_context dc)
{
    Depth depth;
    depth.nvisuals = 0;

    int depths_list_num;
    int *depths_list =
        XListDepths(dc.x.display, dc.x.screen_number, &depths_list_num);

    for (int i = depths_list_num - 1; i != 0; i--)
    {
        if (depths_list[i] == 32)
        {
            int visuals_count;
            XVisualInfo *visuals_available;

            static long visual_template_mask =
                VisualScreenMask | VisualDepthMask | VisualClassMask;
            XVisualInfo visual_template = {
                .screen = dc.x.screen_number, .depth = 32, .class = TrueColor};

            visuals_available =
                XGetVisualInfo(dc.x.display, visual_template_mask,
                               &visual_template, &visuals_count);

            for (int i = 0; i < visuals_count; i++)
            {
                if (is_alpha_visual(dc, visuals_available[i].visual))
                {
                    depth.visuals = visuals_available[i].visual;
                    depth.nvisuals = 1;
                    depth.depth = 32;
                    break;
                }
            }
            XFree(visuals_available);
            break;
        }
    }
    XFree(depths_list);
    return depth;
}

static XRenderColor xrendercolor_from_color(Color color)
{
    unsigned short alpha = color.alpha * 257;
    return (XRenderColor){.alpha = alpha,
                          .red = (color.red * 257 * alpha) / 0xffffU,
                          .green = (color.green * 257 * alpha) / 0xffffU,
                          .blue = (color.blue * 257 * alpha) / 0xffffU};
}

void fill_rectangle(X_context xc, Color c, int x, int y, unsigned int w,
                    unsigned int h)
{
    XRenderColor xrc = xrendercolor_from_color(c);
    XWindowAttributes attrib;
    XGetWindowAttributes(xc.display, xc.window, &attrib);
    XRenderPictFormat *pfmt =
        XRenderFindVisualFormat(xc.display, attrib.visual);

    Picture pict = XRenderCreatePicture(xc.display, xc.window, pfmt, 0, 0);
    XRenderFillRectangle(xc.display, PictOpSrc, pict, &xrc, x, y, w, h);
    XRenderFreePicture(xc.display, pict);
}

Depth get_display_context_depth(Display_context dc)
{
    Depth depth = {.depth = DefaultDepth(dc.x.display, dc.x.screen_number),
                   .visuals = DefaultVisual(dc.x.display, dc.x.screen_number),
                   .nvisuals = 1};
    Depth adepth = get_alpha_depth_if_available(dc);
    return adepth.nvisuals == 1 ? adepth : depth;
}
