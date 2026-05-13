# NGPC Asteroids

Classic Asteroids for the Neo Geo Pocket Color — standalone breakout from the
[ngpc-space](https://github.com/) Space Collection homebrew compilation cart.

## Features
- 8-direction ship with thrust & momentum
- Greyscale asteroids (large/medium/small with different brightness)
- UFO alien ship (appears wave 3+, fires aimed shots)
- Hyperspace warp (B button, 10% death chance, cooldown)
- Title screen, high scores, wave counter
- Subtle starfield background
- Sound effects (thrust, fire, warp, explosions)

## Screenshots
*(coming soon)*

## Controls (Mednafen)
| Key | NGPC | Action |
|-----|------|--------|
| A/D | L/R Joystick | Rotate |
| W | Up Joystick | Thrust |
| Z | A Button | Fire |
| X | B Button | Warp (Hyperspace) |
| Enter | Option | Menu / Back |

## Building

### Requirements
- Linux (tested on Linux Mint, ThinkPad T440p)
- Wine 5.0+
- Toshiba TLCS-900H toolchain (cc900, tulink, tuconv, s242ngp)
- Mednafen 1.22.2+ for emulator testing
- GNU Make
- Python 3 + Pillow (for asset pipeline)
- Physical NGP/NGPC + flash cart for hardware testing

### Quick Start
```bash
git clone <this repo>
cd ngp-asteroids
make clean && make
make run
```

### Asset Pipeline
Convert PNG sprites to C tile data:
```bash
# 1bpp (on/off) — good for ship, bullets
python3 tools/png2sprite.py assets/ship_up.png --name ship_up --mode 1bpp --offset 144

# 2bpp (4 colours) — good for shaded asteroids
python3 tools/png2sprite.py assets/asteroid_large.png --name ast_large --mode 2bpp --offset 160

# Preview in terminal
python3 tools/png2sprite.py assets/ship_up.png --preview
```

Supported sprite sizes: 8×8, 16×16, 16×32, 24×24, or any multiple of 8.
Multi-tile sprites are output in left-to-right, top-to-bottom tile order.

## Project Structure
```
ngp-asteroids/
├── Makefile            # Build rules (reads toolchain.mk)
├── bootstrap.sh        # Environment setup / verification
├── toolchain.mk        # Auto-generated toolchain paths
├── common/             # Shared framework (ameliandev template)
│   ├── ngpc.h          #   Hardware registers & types
│   ├── library.c/.h    #   Framework: tiles, palettes, sound, etc.
│   └── library.inc     #   Z80 sound driver binary
├── src/                # Game source
│   ├── main.c          #   Complete game (~640 lines)
│   └── carthdr.h       #   ROM header (NGPC colour mode)
├── lcf/
│   └── asteroids.lcf   # Linker command file
├── lib/
│   └── system.lib      # NGPC system library
├── assets/             # Source artwork (PNG)
├── tools/              # Asset conversion scripts
│   └── png2sprite.py   #   PNG → C tile data converter
├── build/              # Compiled objects (generated)
└── bin/
    └── asteroids.ngp   # ROM image (generated)
```

## Toolchain Notes
- cc900 is C89 only — no stdint, no mixed declarations
- cc900 must run from its own BIN directory under Wine
- All paths use `winepath -w` conversion
- `volatile u16*` generates bad code in cc900 — use non-volatile pointers
- Signed math and `PrintDecimal` crash the runtime — use unsigned throughout
- JOYPAD register at 0x6F82 is active-HIGH in Mednafen
- Link order: game objects before common objects, system.lib before c900ml.lib
- CartID must be 0x0000 for correct NGPC colour mode detection

## Acknowledgements
- **ameliandev** — [ngpc-project-template](https://github.com/ameliandev/ngpc-project-template)
  providing the C framework (library.c/h, system.lib) and build system foundation
  that makes NGPC homebrew development in C practical
- **lordmizel** — Super Hang-On reference implementation (C++/SDL2) that informed
  the broader NGPC homebrew project this game is part of
- **Dark Fader / BlackThunder** — s242ngp ROM packer tool
- **Toshiba** — TLCS-900H toolchain (cc900, tulink, tuconv)
- **SNK** — Neo Geo Pocket Color hardware
- **Mednafen** — accurate NGP/NGPC emulation for development testing
- The original 1979 Atari **Asteroids** by Ed Logg and Lyle Rains

## Licence
Homebrew — do what you want with it.
