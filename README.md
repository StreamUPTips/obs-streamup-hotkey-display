# StreamUP Hotkey Display for OBS Studio

Shows what keyboard shortcuts you're pressing in real time. There's a dock in OBS that displays the current key combination, and you can push it to a text source so your viewers can see it on screen too. Handy for tutorials, educational streams, or any time you want people to follow along with what you're doing.

## How It Works

The plugin hooks into your keyboard and mouse inputs and displays whatever combination you're pressing. Mouse clicks and scrolling only get picked up when you're holding a modifier key, so it's not logging every random click.

Works on Windows, macOS and Linux (X11 required, Wayland will warn you it's not supported).

## Features

### Dock
- Live display of the current key combination
- Scrollable history of recent key presses (configurable size)
- Start and stop toggle right in the dock
- Auto-clears the display after a set duration

### Text Source Output
- Send the current hotkey to any text source in OBS (GDI+, FreeType 2, or Pango)
- Configurable display duration so it disappears after a set time
- Add a prefix and suffix around the key text
- Auto-shows the text source when a key is pressed

### Key Capture Options
- Modifier combos captured by default (Ctrl, Alt, Shift, Super/Cmd + key)
- Optional single key capture for numpad, number row, letters, punctuation
- Manual whitelist for specific keys you want captured individually
- Customisable key separator (change " + " to " - " or whatever)

### OBS Hotkey
Bind a hotkey to toggle the display on and off without opening the dock.

### WebSocket API
Registered under the `streamup-hotkey-display` vendor:

| Request | What it does |
|---------|-------------|
| `get_status` | Returns whether capture is active and the last combination |
| `enable` | Turns on keyboard capture |
| `disable` | Turns off keyboard capture |
| `get_last_combination` | Returns the most recent key combination |

Also emits `key_pressed` events so external tools like Stream Deck or Streamer.Bot can react to input in real time.

## Build

**In-tree build:**
1. Build OBS Studio: https://obsproject.com/wiki/Install-Instructions
2. Check out this repository to `frontend/plugins/obs-streamup-hotkey-display`
3. Add `add_subdirectory(obs-streamup-hotkey-display)` to `frontend/plugins/CMakeLists.txt`
4. Rebuild OBS Studio

**Stand-alone build (Linux only):**
1. Make sure you have the OBS development packages installed
2. Check out this repository and run `cmake -S . -B build -DBUILD_OUT_OF_TREE=On && cmake --build build`

## Guide

For the full walkthrough, check out the [guide on Notion](https://streamup.notion.site/StreamUP-Hotkey-Display-53b513b427e8425eb584bdc408117daa).

Got feedback or requests? Drop them in the [StreamUP Discord](https://discord.com/invite/RnDKRaVCEu).

## Support

Built and maintained by Andi. If this has been useful, consider supporting to keep it going.

- [**Memberships**](https://andilippi.co.uk/pages/memberships) - Access all products and exclusive perks
- [**PayPal**](https://www.paypal.me/andilippi) - Buy me a beer
- [**Twitch**](https://www.twitch.tv/andilippi) - Come hang out and ask questions
- [**YouTube**](https://www.youtube.com/andilippi) - Tutorials on OBS and streaming
