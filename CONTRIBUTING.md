Contributing to xob
===================

xob is a very small project. There is no particular template for issues or pull requests. All reviews, expert wisdom, suggestions, and contributions are welcome.

## Style

Coding style is llvm with Allman indentation style and 4 spaces per indentation (use `clang-format`). See the `.clang-format` file.

## Map

There are three parts in this project:

* `main` parses the arguments, looks for a configuration file, and contains the main loop.
    * `parse_input` reads a value from the standard input and returns an `Input_value`: it contains the value itself, whether it is in normal or alternate mode (e.g. muted), and a `valid` boolean in case the provided input cannot be parsed.
    * **Main loop** After initialising a `Display_context` using the information `style` from a configuration file, the program waits for input on stdin. `select` is used to provide a timeout mechanism during this wait. This is in case the bar has been displayed enough time and needs to be hidden using `hide`. If the bar is not on display, there is no timeout. When an input is available, it is parsed using `parse_input`. If it is not a valid input (not a number followed or not by '!'), xob stops. If it is valid, the bar is displayed using `display`.
* `conf` parses a configuration file and generates a valid configuration.
    * `Style` is the structure for a style (or "configuration").
    * `conf.h` defines `DEFAULT_CONFIGURATION` the default hard-coded configuration.
    * `parse_style_config` builds a style from a configuration file reporting errors if need be.
* `display` handles the X related stuff to construct, update, display, and hide the bar.
    * All information required to show, update, and hide a given bar is sumed-up as a `Display_context` value.
    * `init` builds a display context corresponding to a given style.
    * `show` displays the bar, given a value, maximum value, whether the display mode is normal or alternate (`show_mode`), and the prefered way to represent overflows (`overflow_mode`).
    * `hide` hides the bar

Do not hesitate to issue requests for additional information.
