# Changelog

This file recaps changes between releases.

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
