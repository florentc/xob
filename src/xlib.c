#include "display.h"
#include <X11/Xlib.h>

GC gc_from_color(X_context x, Color color)
{
    XColor xcolor = {
        .red = color.red * 257,
        .green = color.green * 257,
        .blue = color.blue * 257,
        .flags = DoRed | DoGreen | DoBlue,
    };
    GC result = XCreateGC(x.display, x.window, 0, NULL);
    Colormap colormap = DefaultColormap(x.display, x.screen_number);
    XAllocColor(x.display, colormap, &xcolor);
    XSetForeground(x.display, result, xcolor.pixel);
    return result;
}

void fill_rectangle(X_context xc, Color c, int x, int y, unsigned int w,
                    unsigned int h)
{
    GC xgc = gc_from_color(xc, c);
    XFillRectangle(xc.display, xc.window, xgc, x, y, w, h);
}

Depth get_display_context_depth(Display_context dc)
{
    Depth dc_depth = {.depth = DefaultDepth(dc.x.display, dc.x.screen_number),
                      .visuals =
                          DefaultVisual(dc.x.display, dc.x.screen_number),
                      .nvisuals = 1};
    return dc_depth;
}
