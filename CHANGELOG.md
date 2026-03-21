# OBS Hotkey Display - Changelog

---

## v1.1.0 (21 Mar '26)
**Patch Focus:** New features, bug fixes & performance

### New Features
- Added configurable key separator so you can change the default " + " between keys to whatever you want (e.g. "-", " - ", "+")
- Added key combination history list below the main display with configurable history size
- Added OBS hotkey for toggling capture on/off. Bind any key in OBS Settings > Hotkeys to start/stop without clicking the dock
- Added WebSocket request handlers: `get_status`, `enable`, `disable`, and `get_last_combination` for external tools like Stream Deck and Streamer.Bot
- Added F1-F12 support in the manual key whitelist
- Display now clears automatically when you stop monitoring

### Bug Fixes
- Fixed key combinations not re-displaying when pressing the same key again while holding a modifier (e.g. hold Shift, press A, release A, press A again would not show the second press)
- Fixed single key presses in quick succession incorrectly showing as combined (e.g. pressing A then quickly pressing B would show "A + B")
- Fixed Shift + single key combinations being ignored. Shift + F5, Shift + Insert, etc. now display correctly
- Fixed duplicate modifier names when pressing both left and right variants (e.g. "Ctrl + Ctrl" now correctly shows "Ctrl")
- Fixed double hook installation on startup that could cause duplicate key events
- Fixed memory leak from unused layout member in the settings dialog
- Fixed unnecessary hook re-installation after every display clear timer
- Reduced redundant source lookups per key event from 3 OBS API calls to 1

### Platform Improvements
- macOS: Event tap now automatically re-enables if the system disables it due to timeout
- Linux: Wayland sessions are now detected with a warning that X11 is required for global keyboard capture

### Under the Hood
- Reduced mutex lock/unlock operations per key event from 5 to 1
- Consolidated duplicate show/hide source code into a single method
- Proper encapsulation for dock widget members
- Added settings defaults for forward-compatibility with future settings
- General code cleanup

## v1.0.0 (25 Oct '25)
**Patch Focus:** Update to OBS 32
- Added logging functionality
- Fixed Shift + F key combinations not being detected correctly
- Added mouse click detection
- Added required modifier key for mouse actions
- Update to OBS 32
- Refined the dock layout spacing so the hotkey display matches OBS theming margins
- Added localisation

## v0.0.1 (10 Sept '24)
**Initial Release**
Core functionality for displaying hotkeys in OBS:
- Live hotkey display dock with real-time keyboard hook functionality
- OBS dock integration with start/stop controls
- Text source integration with prefix and suffix support
- Scene name display settings
- Timer settings for hotkey display duration
- Websocket support for broadcasting key press events to external applications
- Localization framework for translations
- Cross-platform support (Windows, Mac, Linux)
- Hook state persistence across OBS sessions
- Settings menu with customizable display options
- Fixed high CPU usage issues
- Fixed startup UI errors
