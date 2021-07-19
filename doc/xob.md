% XOB(1) xob 0.2 | xob Manual
% Florent Ch
% July 2021

# NAME

xob - A lightweight overlay volume/backlight/progress/anything bar for the X Window System.

# SYNOPSIS

**xob** [**-m** *maximum*] [**-t** *timeout*] [**-c** *configfile*] [**-s** *style*] [**-q**]

# DESCRIPTION

**xob** (the X Overlay Bar) displays numerical values fed through the standard input on a bar that looks like the volume bar on a television screen. When a new integer value is read on the standard input, the bar is displayed over other windows for a configurable amount of time and then disappears until it is fed a new value. When a value is followed by a bang '!', an alternative color is used. This feature makes it possible to provide visual feedback for alternative states (e.g. unmuted/muted, auto/manual). The appearance is configurable through options described in this manual. The way overflows (when the value exceeds the maximum) are displayed is also configurable. The program ends when it reads "end" or "quit" (or actually anything else than a number).

# OPTIONS

**-m** *maximum*
:   Number of steps in the bar (and maximum value since 0 is always the minimum value). By default: 100.

**-t** *timeout*
:   Duration in milliseconds between an update and the vanishing of the bar. If set to 0, the bar is never hidden. By default: 1000 (1 second).

**-s** *style*
:   Style (appearance) to choose in the configuration file. By default: default.

**-c** *configfile*
:   Specifies a configuration file path. By default: see below.

**-q**
:   Specifies whether to suppress all normal output. By default: not suppressed

# USAGE

## GENERAL CASE

Use a program that listens to events (such as a change in audio volume levels) and issues new values on the standard output automatically. Launch `the_listener_program | xob`. Ready to use input programs for audio volume and screen backlight are available on the xob project homepage: https://github.com/florentc/xob

## FALLBACK METHOD

In case no input program fits your needs, you may trigger changes manually. Append new values in a named pipe (a pipe that persists as a special file on the filesystem) and have xob consume them as they arrive.

Create a named pipe, e.g. */tmp/xobpipe*, on your filesystem.

    mkfifo /tmp/xobpipe

Have xob consume new values as they arrive on the pipe.

    tail -f /tmp/xobpipe | xob

Write values to the pipe when you deem it relevant. In the classic audio volume bar example, that would be after the user has pressed a button and you changed the volume (usually set up as a keybinding in your window manager or desktop environment).

    command_that_outputs_a_value >> /tmp/xobpipe

To try it manually, issue a test value such as `echo 43 >> /tmp/xobpipe`.

# CONFIGURATION FILE

The configuration file only specifies styles (appearances) for the bar. The maximum value and timeout are set by **-m** and **-t**. When starting, xob looks for the configuration file in the following order:

1. The path specified as the **-c** argument.
2. \$XDG\_CONFIG\_HOME\$/xob/styles.cfg (if \$XDG\_CONFIG\_HOME\$ is set)
3. ~/.config/xob/styles.cfg
4. Under the system configuration directory (determined during build process): e.g. /etc/xob/styles.cfg or /usr/local/etc/xob/styles.cfg

## SYNTAX

The configuration file adheres to the libconfig syntax. An option is set up using "=" or ":". The options are separated using ";".

    option1 = value1;
    option2 = value2;

Commented lines start with "#".

    # This is a comment

Options can be grouped together inside curly brackets. Some options expect a group as a value.

    option3 = {
        suboptionA = valueA;
        suboptionB = {
            subsuboptionB1 = valueB1;
            subsuboptionB2 = valueB2;
            };
        suboptionC = valueC;
    };

## OPTIONS

                    length
          ───────────────────────────
    ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░] outline
    ░░███████████████████████████████████░░] border
    ░░██░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░██░░] padding
    ░░██░░████████████░░░░░░░░░░░░░░░░░██░░│
    ░░██░░█foreground█░░░░background░░░██░░│ thickness
    ░░██░░████████████░░░░░░░░░░░░░░░░░██░░│
    ░░██░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░██░░] padding
    ░░██████████████border███████████████░░] border
    ░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░░] outline
          |                          |
          0                       maximum

In the following, a dot "." means "suboption". For instance "color.normal.fg" means "The suboption fg of the suboption normal of option color".

**orientation** *"horizontal" | "vertical"* (default: vertical)
:   Orientation of the bar which either fills up from left to right ("horizontal") or bottom to top ("vertical").

**x.relative** *ratio* (default: 1.0)
:   Relative horizontal position on screen between 0.0 (left edge) and 1.0 (right edge). Centered by default (0.5).

**x.offset** *pixels* (default: -48)
:   Horizontal offset in pixels from the position specified with **x.relative**. This can be used for absolute positioning from an edge of the screen, creating gaps, stacking bars, etc.

**y.relative** *ratio* (default: 0.5)
:   Relative vertical position on screen between 0.0 (top edge) and 1.0 (bottom edge).

**y.offset** *pixels* (default: 0)
:   Vertical offset in pixels from the position specified with **y.relative**. This can be used for absolute positioning from an edge of the screen, creating gaps, stacking bars, etc.

**length.relative** *ratio* (default: 0.3)
:   Relative length (horizontal bar) or height (vertical bar) of the bar between 0.0 (collapsed) and 1.0 (full available length on screen).

**length.offset** *pixels* (default: 0)
:   Length offset in pixels from the relative length specified with **length.relative**. This can be used for absolute sizes (using positive values and 0 as **length.relative**) or creating gaps (using negative values and 1 as **length.relative**).

**thickness.absolute** *positive integer* (default: 24)
:   Height (horizontal bar) or width (vertical bar) in pixels without taking borders into account.

**border** *pixels* (default: 4)
:   Size of the border in pixels.

**outline** *pixels* (default: 3)
:   Size of the outline in pixels.

**padding** *pixels* (default: 3)
:   Size of the padding in pixels.

**overflow** *"hidden" | "proportional"* (default: proportional)
:   When "hidden" is set, a full bar is displayed using the overflow color when the value exceeds the maximum, whatever that value is. When "proportional" is set, only a portion of the bar whose size depends on the overflow is displayed using the overflow color. If the value is twice the maximum, half the bar is displayed using the overflow color. The two zones are separated by a padding-sized gap.

Each of the following have three suboptions ".fg", ".bg", and ".border" corresponding to hexadecimal RGB or RGBA specifications (e.g. #ff0000 or #ff0000ff) of the colors of the foreground, the backgroud, and the border. If xob is built without transparency support, the alpha channel is ignored.

**color.normal** *colors* (default: {fg = "#ffffff"; bg = "#00000090"; border = "#ffffff";})
:   Colors in the general case.

**color.alt** *colors* (default: {fg = "#555555"; bg = "#00000090"; border = "#555555";})
:   Colors for alternate display (e.g. muted).

**color.overflow** *colors* (default: {fg = "#ff0000"; bg = "#00000090"; border = "#ff0000";})
:   Colors in case of overflow (the displayed valued exceeds the maximum).

**color.altoverflow** *colors* (default: {fg = "#ff0000"; bg = "#00000090"; border = "#555555";})
:   Colors for alternate display in case of overflow.


## STYLES

All the options described above must be encompassed inside a style specification. A style consists of a group of all or some of the options described above. The name of the style is the name of an option at the root level of the configuration file. When an option is missing from a style, the default values are used instead. A configuration file may specify several styles (at least 1) to choose using the **-s** argument.

This example configuration file provides two styles "volume" and "backlight". Instances of xog launched with **-s volume** and **-s backlight** will look according to the corresponding style.

    volume = {
        thickness = 24;
        outline   = 1;
        border    = 2;
        padding   = 0;
        y = {
            relative = 0.9;
            offset = 0;
        };
    };
    backlight = {
        thickness = 24;
        outline   = 1;
        border    = 2;
        padding   = 0;
        y = {
            relative = 0.9;
            # To prevent overlap with the volume bar if displayed at the same time
            offset = -30;
        };

        color = {
            normal = {
                fg     = "#0000ff";
                bg     = "#000000";
                border = "#0000ff";
            };
        };
    };

## DEFAULT CONFIGURATION FILE

    default = {
        x         = {relative = 1; offset = -48;};
        y         = {relative = 0.5; offset = 0;};
        length    = {relative = 0.3; offset = 0;};
        thickness = 24;
        outline   = 3;
        border    = 4;
        padding   = 3;
        orientation = "vertical";

        overflow = "proportional";

        color = {
            normal = {
                fg     = "#ffffff";
                bg     = "#00000090";
                border = "#ffffff";
            };
            alt = {
                fg     = "#555555";
                bg     = "#00000090";
                border = "#555555";
            };
            overflow = {
                fg     = "#ff0000";
                bg     = "#00000090";
                border = "#ff0000";
            };
            altoverflow = {
                fg     = "#550000";
                bg     = "#00000090";
                border = "#550000";
            };
        };
    };

# FAQ

> "How should I display different sources of information (e.g. volume and brightness)?"
> "What happens if several bars are displayed at the same time?"

You can run and distinguish two or more instances of **xob** with different styles (including color, position, extreme values, etc.). To do so, specify and use different styles from your configuration file (or use different configuration files). To prevent the bars from overlapping, make use of the offset options. For instance you can offset a bar to the top or bottom (see the example provided in the **STYLES** section).

> "Can I integrate xob in a panel of my desktop environment or window manager?"

There is no support for panel integration. You can however use absolute positioning and no timeout (*timeout* set at 0) to mimic this behaviour in simple situations.

> "How to set up xob with multiple monitors?"

xob works well under multihead setups but there is no easy way to configure the position of the bar for now. For example, in a dual monitor setup with the default configuration, the horizontal centering is not local to one of the two monitors. It is global. The bar might be split in two: one part on each screen. Stick to a corner or use absolute positioning. If you want an xob instance to be centered (horizontally) on the far-right monitor, set *x.relative* to 1.0 (anchored on the far right) and the *x.offset* to minus half the width of that screen.

# CONTRIBUTIONS

Feedback and contributions are welcome.
