xob - X Overlay Bar
===================

![Illustration screencast](/doc/img/screencast.gif)

A lightweight configurable overlay volume/backlight/progress/anything bar for the X Window System (and Wayland compositors with XWayland). Each time a new value is read on the standard input, it is displayed as a tv-like bar over other windows. It then vanishes after a configurable amount of time. A value followed by a bang '!' is displayed using an alternate color to account for special states (e.g. muted audio). There is also support for overflows (when the value exceeds the maximum).

![Features](/doc/img/features.svg)

## Installation

xob depends on libx11, libxrender (for optional transparency support) and libconfig.

    make
    make install

To build xob without transparency support and rely only on libx11 and libconfig: `make enable_alpha=no`.

Packages are available in the following repositories:

[![Packaging status](https://repology.org/badge/vertical-allrepos/xob.svg)](https://repology.org/project/xob/versions)

## Usage

    xob [-m maximum] [-t timeout] [-c configfile] [-s style]

* **maximum** Maximum value/number of steps in the bar (default: 100). 0 is always the minimum.
* **timeout** Duration in milliseconds the bar remains on-screen after an update (default: 1000). 0 means the bar is never hidden.
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

    echo 43 >> /tmp/xobpipe

Adapt this use-case to your workflow (scripts, callbacks, or keybindings handled by the window manager).

### Ready to use volume bar

If you are using pulseaudio, save the following Python script (depends on the pulsectl python library) and simply pipe it in xob! `./pulse-volume-watcher.py | xob`

```python
#!/usr/bin/env python3
from pulsectl import Pulse, PulseLoopStop
import sys

with Pulse() as pulse:
  def callback(ev):
    if ev.index == sink_index: raise PulseLoopStop
  def current_status(sink):
    return round(sink.volume.value_flat * 100), sink.mute == 1
  try:
    sinks = {s.index:s for s in pulse.sink_list()}
    if len(sys.argv) > 1:
      # Sink index from command line argument if provided
      sink_index = int(sys.argv[1])
      if not sink_index in sinks:
        raise KeyError(f"Sink index {sink_index} not found in list of sinks.")
    else:
      # Automatic determination of default sink otherwise
      default_sink_name = pulse.server_info().default_sink_name
      try:
        sink_index = next(index for index, sink in sinks.items()
                          if sink.name == default_sink_name)
      except StopIteration: raise StopIteration("No default sink was found.")

    pulse.event_mask_set('sink')
    pulse.event_callback_set(callback)
    last_value, last_mute = current_status(sinks[sink_index])

    while True:
      pulse.event_listen()
      sinks = {s.index:s for s in pulse.sink_list()}
      value, mute = current_status(sinks[sink_index])
      if value != last_value or mute != last_mute:
        print(str(value) + ('!' if mute else ''))
        last_value, last_mute = value, mute
      sys.stdout.flush()

  except Exception as e:
    print(f"ERROR: {e}", file=sys.stderr)
```

This script listens to volume and mute events. No matter how the volume changes (keybindings, pulse control panel, headphones plugged-in), it will instantly show up the volume bar. It should work out of the box but you can optionally pass the index of a sink on the command line, as given by `pacmd list-sinks`.

### Ready to use brightness bar

One can access the brightness value from `/sys/class/backlight/video_backlight/brightness` (where `video` is your video device). The following script watches for modifications on that file using the watchdog python library. No matter how the brightness changes, this script will return the new brightness value. You may have to change the path of `brightness_file` if you are not using an Intel device. Simply pipe it in xob and you are ready to go. `./brightness-watcher.py | xob`.

```python
#!/usr/bin/env python3
from watchdog.observers import Observer
from watchdog.events import FileSystemEventHandler, FileModifiedEvent
import sys
import time

brightness_file = '/sys/class/backlight/intel_backlight/brightness'
max_brightness_file ='/sys/class/backlight/intel_backlight/max_brightness'
with open(max_brightness_file, 'r') as f:
    maxvalue = int(f.read())

def notify(file_path):
    with open(file_path, 'r') as f: 
        value = int(int(f.read())/maxvalue*100)
        print(value)

class Handler(FileSystemEventHandler):

    def on_modified(self, event):
        if isinstance(event, FileModifiedEvent):
            notify(event.src_path)

handler = Handler()
observer = Observer()
observer.schedule(handler, path=brightness_file)
observer.start()
try:
    while True:
        sys.stdout.flush()
        time.sleep(1)
except KeyboardInterrupt:
    observer.stop()
observer.join()
```
## Appearance

When starting, xob looks for the configuration file in the following order:

1. The path specified as the **-c** argument.
2. `$XDG_CONFIG_HOME$/xob/styles.cfg` (if `$XDG_CONFIG_HOME$` is set)
3. `~/.config/xob/styles.cfg`
4. Under the system configuration directory (determined during build process): e.g. /etc/xob/styles.cfg or /usr/local/etc/xob/styles.cfg

Consult the man page for detailed information about the configuration file and the available options. The following `styles.cfg` defines a single style called "default" that showcases all the possible options set to the default values. The configuration file may contain additional styles to choose among using the **-s** argument.

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

### Orientation

The bar can be horizontal (it fills up from left to right) or vertical (it fills up from bottom to top) depending on option `orientation`. The default value is `"vertical"`.

### Positionning

The horizontal `x` and vertical `y` position of the bar is determined by a relative position on the screen (0.0 and 1.0 on the edges, 0.5 in the middle) and an offset in pixels from that relative position. Likewise, the length of the bar has a relative component (1.0 all available space, 0.0 collapsed) and an offset in pixels. Here are some examples.

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

There are three colors: foreground, background, and border. They change depending on the displayed value. Also, there are two ways to display overflows: use the overflow color with no feedback on how much it overflows ("hidden" mode), or use the overflow color on part of the bar only proportionally to how much the value overflows ("proportional" mode).

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

You can run and distinguish two or more instances of xob with different styles (including color, position, extreme values, etc.). To do so, specify and use different styles from your configuration file (or use different configuration files). To prevent the bars from overlapping, make use of the offset options. For instance, in horizontal mode, you can offset a bar to the top or bottom (see the following example configuration file).
 
    volume = {
        thickness = 24;
        outline   = 1;
        border    = 2;
        padding   = 0;
        y = {
            relative = 0.9;
            offset = 0;
        };
        orientation = "horizontal";
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
        orientation = "horizontal";

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

> "How about multiple monitors?"

xob works well under multihead setups. The `x`, `y`, and `length` style options refer to the combined screen surface. By default the bar is vertical near the right edge of the rightmost monitor. In vertical layouts, you may prefer to switch the bar to horizontal mode as follows to avoid splits.

    horizontal = {
        x         = {relative = 0.5; offset = 0;};
        y         = {relative = 1; offset = -48;};
        orientation = "horizontal";
    };

## Planned features

* Multihead-friendly configuration.
* New overflow styles.
* Smooth transitions.
* Configuration through X resources.

## License

GPL3, see [LICENSE](/LICENSE).
