xob - X Overlay Bar
===================

![Illustration screencast](/doc/img/screencast.gif)

A lightweight configurable overlay volume/backlight/progress/anything bar for the X Window System. Each time a new value is read on the standard input, it is displayed as a tv-like bar over other windows. It then vanishes after a configurable amount of time. A value followed by a bang '!' is displayed using an alternate color to account for special states (e.g. muted audio). There is also support for overflows (when the value exceeds the maximum).

## Installation

xob depends on libx11 and libconfig.

    make
    make install

As of May 2019, there are packages for the following Linux distributions:

* [Arch Linux package on the AUR](https://aur.archlinux.org/packages/xob/)
* [Source Mage GNU/Linux](http://codex.sourcemage.org/test/wm-addons/xob)
* [Void Linux package](https://github.com/void-linux/void-packages/tree/master/srcpkgs/xob)

## Usage

    xob [-m maximum] [-t timeout] [-c configfile] [-s style]

* **maximum** Maximum value/number of steps in the bar (default: 100). 0 is always the minimum.
* **timeout** Time in seconds the bar stays on-screen after an update (default: 1). 0 means the bar is never hidden.
* **configfile** Path to a file that specifies styles (appearances).
* **style** Chosen style from the configuration (default: the style named "default").

### Try it out

Launch xob in a terminal, enter a value (positive integer), press return. Suffix a value with '!' for alternate color. Use a value above the maximum (default: 100) to see how overflows are displayed.

### General case

You may manage a bar for audio volume, backlight intensity, or whatever, using a named pipe. Create a named pipe, e.g. */tmp/xobpipe*, on your filesystem using

    mkfifo /tmp/xobpipe

Connect the named pipe to the standard input of an xob instance.

    tail -f /tmp/xobpipe | xob

Set up your environment so that after updating audio volume, backlight intensity, or whatever, to a new value like 43, it writes that value into the pipe:

    echo 43 > /tmp/xobpipe

Adapt this use-case to your workflow (scripts, callbacks, or keybindings handled by the window manager).

### Ready to use volume bar

If you are using pulseaudio, use the `pulse_listener.py` Python script (depends on the pulsectl python library) and simply pipe it in xob!

```sh
./pulse_listener.py | xob
```

This script listens to volume and mute events. No matter how the volume changes (keybindings, pulse control panel, headphones plugged-in), it will instantly show up the volume bar. You might need to adapt the `SINK_ID` constant in the script to fit your needs.

## Appearance

When starting, xob looks for the configuration file in the following order:

1. The path specified as the **-c** argument.
2. `$XDG_CONFIG_HOME$/xob/styles.cfg` (if `$XDG_CONFIG_HOME$` is set)
3. `~/.config/xob/styles.cfg`
4. Under the system configuration directory (determined during build process): e.g. /etc/xob/styles.cfg or /usr/local/etc/xob/styles.cfg

Consult the man page for detailed information about the configuration file and the available options. The following `styles.cfg` defines a single style called "default" that showcases all the possible options set to the default values. The configuration file may contain additional styles to choose among using the **-s** argument.

    default = {
        x         = {relative = 0.5; offset = 0;};
        y         = {relative = 0.9; offset = 0;};
        length    = {relative = 0.2; offset = 0;};
        thickness = 24;
        outline   = 3;
        border    = 4;
        padding   = 3;

        overflow = "proportional";

        color = {
            normal = {
                fg     = "#ffffff";
                bg     = "#000000";
                border = "#ffffff";
            };
            alt = {
                fg     = "#555555";
                bg     = "#000000";
                border = "#555555";
            };
            overflow = {
                fg     = "#ff0000";
                bg     = "#000000";
                border = "#ff0000";
            };
            altoverflow = {
                fg     = "#550000";
                bg     = "#000000";
                border = "#550000";
            };
        };
    };

### Positionning

The horizontal `x` and vertical `y` position of the bar is determined by a relative position on the screen (0.0 and 1.0 on the edges, 0.5 in the middle) and an offset in pixels from that relative position. Likewise, the length of the bar has a relative component (1.0 full-width, 0.0 collapsed) and an offset in pixels. Here are some examples.

Illustration | Relative `x` | Offset `x` | Relative `y` | Offset `y` | Relative `length` | Offset `length` 
------------ | ------------ | ---------- | ------------ | ---------- | ----------------- | --------------- 
![pos00](/doc/img/positionning/pos00.svg) | 0.5 | 0 | 0.75 | 0 | 0.3 | 0
![pos01](/doc/img/positionning/pos01.svg) | 0.0 | 0 | 0.0 | 0 | 0.3 | 0
![pos02](/doc/img/positionning/pos02.svg) | 1.0 | 0 | 0.0 | 0 | 0.3 | 0
![pos03](/doc/img/positionning/pos03.svg) | 0.5 | 0 | 0.0 | 0 | 0.3 | 0
![pos04](/doc/img/positionning/pos04.svg) | 0.5 | 0 | 1.0 | 0 | 0.3 | 0
![pos05](/doc/img/positionning/pos05.svg) | 1.0 | -10 | 1.0 | -10 | 0.3 | 0
![pos06](/doc/img/positionning/pos06.svg) | 0.5 | 0 | 0.75 | 0 | 0.5 | 0
![pos07](/doc/img/positionning/pos07.svg) | 0.5 | 0 | 0.75 | 0 | 1 | 0
![pos08](/doc/img/positionning/pos08.svg) | 0.5 | 0 | 0.75 | 0 | 1 | -20
![pos09](/doc/img/positionning/pos09.svg) | 0.0 | +110 | 0.0 | +135 | 0.0 | +126

### Size and borders

![Dimensions guide](/doc/img/dimensions.svg)

The values for thickness, padding, border, and outline are a number of pixels.

### Colors and overflows

![Colors guide](/doc/img/colors.svg)

There are three colors: foreground, background, and border. They change depending on the displayed value. Also, there are two ways to display overflows: use the overflow color with no feedback on how much it overflows ("hidden" mode), or use the overflow color on part on the bar only proportionally to how much the value overflows ("proportional" mode).

Illustration | Display mode       | Overflow type
------------ | ------------------ | -------------
![empty](/doc/img/states/empty.svg) | Normal | No overflow (empty)
![quarter](/doc/img/states/quarter.svg) | Normal | No overflow (quarter-full)
![quarter-alt](/doc/img/states/quarter-alt.svg) | Alternate (e.g. mute) | No overflow (quarter-full)
![full](/doc/img/states/full.svg) | Normal | No overflow (full)
![overflow-hidden](/doc/img/states/overflow-hidden.svg) | Normal | Overflow in "hidden" mode
![overflow-proportional](/doc/img/states/overflow-proportional.svg) | Normal | Overflow in "proportional" mode
![overflow-hidden-alt](/doc/img/states/overflow-hidden-alt.svg) | Alternate | Overflow in "hidden" mode
![overflow-proportional-alt](/doc/img/states/overflow-proportional-alt.svg) | Alternate | Overflow in "proportional" mode

### i3wm

![i3 style screenshot](/doc/img/i3-style.png)

Here is a style that integrates well into default i3wm. Add it to your `styles.cfg`

    i3 = {
        padding   = 0;
        border    = 1;
        outline   = 0;
        thickness = 24;
        color = {
            normal = {
                fg = "#285577";
                bg = "#222222";
                border = "#4c7899";
            };
            alt = {
                fg = "#333333";
                bg = "#222222";
                border = "#444444";
            };
            overflow = {
                fg = "#900000";
                bg = "#222222";
                border = "#933333";
            };
            altoverflow = {
                fg = "#900000";
                bg = "#222222";
                border = "#444444";
            };
        };
    };

## FAQ

> "How should I display different sources of information (e.g. volume and brightness)?"

> "What happens if several bars are displayed at the same time?"

You can run and distinguish two or more instances of xob with different styles (including color, position, extreme values, etc.). To do so, specify and use different styles from your configuration file (or use different configuration files). To prevent the bars from overlapping, make use of the offset options. For instance you can offset a bar to the top or bottom (see the following example configuration file).
 
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

> "Can I integrate xob in a panel of my desktop environment or window manager?"

There is no support for panel integration. You can however use absolute positioning and no timeout (`-t 0`) to mimic this behaviour in simple situations.

> "How to set up xob with multiple monitors?"

xob works well under multihead setups but there is no easy way to configure the position of the bar for now. In a dual monitor setup with the default configuration, the horizontal centering is not local to one of the two monitors. It is global. The bar might be split in two: one part on each screen.  You will probably find it easier to fall back to absolute positioning much like the last example in the table in section "Positioning". If you want an xob instance to be centered (horizontally) on the far-right monitor, set *x.relative* to 1.0 (anchored on the far right) and the *x.offset* to minus half the width of that screen.

## Planned features

* Multihead-friendly configuration.
* Vertical mode.
* New overflow styles.
* Smooth transitions.
* Configuration through X resources.

## License

GPL3, see [LICENSE](/LICENSE).
