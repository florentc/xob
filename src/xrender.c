#include "display.h"
#include <X11/extensions/Xrender.h>
#include <stdlib.h>

_Bool is_alpha_visual(Display_context dc, Visual *visual)
{
    XRenderPictFormat *fmt = XRenderFindVisualFormat(dc.x.display, visual);
    if (fmt->type == PictTypeDirect && fmt->direct.alphaMask)
    {
        return True;
    }
    return False;
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

static XRenderColor from_color(Color color)
{
    XRenderColor result;
    result.alpha = color.alpha * 257;
    result.red = (color.alpha * 257 * result.alpha) / 0xffffU;
    result.green = (color.alpha * 257 * result.alpha) / 0xffffU;
    result.blue = (color.alpha * 257 * result.alpha) / 0xffffU;
    return result;
}

/* Draw a rectangle with the given size, position and color */
void fill_rectangle(X_context xc, Color c, int x, int y, unsigned int w,
                    unsigned int h)
{
    XRenderColor xrc = from_color(c);
    XWindowAttributes attrib;
    XGetWindowAttributes(xc.display, xc.window, &attrib);
    XRenderPictFormat *pfmt =
        XRenderFindVisualFormat(xc.display, attrib.visual);

    Picture pict = XRenderCreatePicture(xc.display, xc.window, pfmt, 0, 0);
    XRenderFillRectangle(xc.display, PictOpSrc, pict, &xrc, x, y, w, h);
    XSync(xc.display, 0);
}

Depth get_display_context_depth(Display_context dc)
{
    Depth depth = {.depth = DefaultDepth(dc.x.display, dc.x.screen_number),
                   .visuals = DefaultVisual(dc.x.display, dc.x.screen_number),
                   .nvisuals = 1};

    Depth adepth = get_alpha_depth_if_available(dc);
    if (adepth.nvisuals == 1)
    {
        return adepth;
    }
    return depth;
}
