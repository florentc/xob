# Changelog

This file recaps changes between releases.

## [0.2] - 2020-03-09

This release introduces more precise timeout specification (mind the new unit for `-t`) and an orientation option. This option makes it possible to chose between a new vertical style or the classic horizontal style.

### Added

- New vertical mode to display a bar that fills up from bottom to top.
- New `orientation` option to choose between classic horizontal mode and new vertical mode.
- More precise timeout specification (millisecond). Values below one second are now allowed.

### Changed

- Default style: the bar is now vertical near the right edge of the screen. This prevents the bar from overlapping two screens in typical horizontal dual-monitor setups.
- Unit for timeout `-t` is now the millisecond. xob will issue a warning when the specified value seems too low (e.g. after a migration from v0.1.x).

### Fixed

- Integers (`0` and `1`) not accepted as values for `rel` options in configuration file .
- Typos in documentation.


## [0.1.1] - 2018-08-25

This release does not add any major new end-user feature. It corrects issues and improves the build process thanks to a revamped `Makefile` for easier packaging of the program in linux distributions. It also provides a default system configuration file.

### Added

- `WM_CLASS` property for the xob windows
- Default system configuration file
- Configurable (during build) system configuration path: xob does not always fallback to `/etc/xob/styles.cfg` anymore, it adapts to the system configuration directory specified during the build process.
- Changelog
- New planned feature: configuration through X resources

### Changed

- Naming conventions in types and include guards to avoid clash with possible reserved C and POSIX identifiers
- Naming conventions of the variables in the `Makefile` to fit the GNU Coding Standards

### Fixed

- Build errors on certain configurations due to blunders in `Makefile`

## [0.1] - 2018-08-21

Initial release of xob.
