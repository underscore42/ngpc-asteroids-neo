# Asteroids Neo — NGPC Homebrew

Classic Asteroids reimagined for the Neo Geo Pocket Color.

Built as a homebrew learning project using the ameliandev NGPC framework,
exploring the TLCS-900H toolchain, K2GE graphics engine, and T6W28 sound.

## Features
- 16×16 ship sprite with 8-direction rotation and thrust animation
- 32×16 alien ship with aimed shots (appears every wave)
- Multi-size asteroids: large (16×16), medium (8×8), small (8×8)
- Greyscale palette system with distinct brightness per rock size
- Hyperspace warp (B button, 10% death chance, cooldown timer)
- Bitmap title screen marquee with colour accents
- 6 sound effects (fire, thrust, warp, explosion, UFO hum, UFO fire)
- High score table with flash save attempt
- Subtle starfield background
- Wave progression with increasing difficulty

## Screenshots
See `screenshots/` directory.

## Controls
| Mednafen Key | NGPC Button | Action |
|---|---|---|
| A / D | Left / Right | Rotate ship |
| W | Up | Thrust |
| Z | A | Fire |
| X | B | Warp (Hyperspace) |
| Enter | Option | Menu / Back |

## Building

### Requirements
- Linux (tested on Linux Mint, ThinkPad T440p)
- Wine 5.0+ (for TLCS-900H toolchain)
- Toshiba TLCS-900H toolchain: cc900, tulink, tuconv, s242ngp
- Mednafen 1.22+ or RetroArch with NeoPop core
- GNU Make
- Python 3 + Pillow (for `tools/png2sprite.py`)

### Quick Start
```bash
git clone <this repo>
cd asteroids-neo
make clean && make
make run
```

### Asset Pipeline
Convert PNG sprites to NGPC packed 2bpp tile data:
```bash
# 1bpp mode (binary on/off)
python3 tools/png2sprite.py assets/ship.png --name ship_n --mode 1bpp --preview

# 2bpp mode (4 colour shading)
python3 tools/png2sprite.py assets/asteroid.png --name rock_large --mode 2bpp --offset 216

# Round-trip: edit PNG, re-convert, paste into src/tiles.c
```

## Project Structure
```
asteroids-neo/
├── src/                # Game source (multi-file)
│   ├── main.c          #   Entry point, game loop, state machine
│   ├── game.c/h        #   Shared state, entity pool, constants
│   ├── entities.c/h    #   Entity rendering, movement, collision
│   ├── screen.c/h      #   Title screen, HUD, palettes, starfield
│   ├── tiles.c/h       #   All tile data, install functions
│   ├── sound.c/h       #   Sound effect definitions
│   ├── save.c/h        #   Flash high score persistence
│   └── carthdr.h       #   ROM header
├── common/             # ameliandev framework
│   ├── ngpc.h          #   Hardware registers & types
│   ├── library.c/h     #   Framework functions
│   └── library.inc     #   Z80 sound driver
├── tools/
│   └── png2sprite.py   #   PNG → packed 2bpp tile converter
├── assets/             # Source artwork (PNG)
├── screenshots/        # Game screenshots
├── releases/           # Release ROM builds
├── lcf/asteroids.lcf   # Linker command file
├── lib/system.lib      # NGPC system library
├── Makefile
├── LICENSE             # MIT
└── README.md
```

## Technical Notes

### NGPC Tile Format
Tiles are 8×8 pixels, packed 2bpp: each row is one `u16` word where
bits 15-14 = pixel 0 (leftmost), bits 13-12 = pixel 1, etc.
Colour index 0 = transparent on sprite plane, opaque black on scroll plane.

### Key Learnings
- `CartID` must be `0x0000` for NGPC colour mode to work in Mednafen
- System font occupies tiles 0-255; custom tiles safe from 144+, marquee at 300+
- `InstallTileSetAt()` Len parameter is in **words** (u16), not tiles
- `volatile u16*` generates bad code in cc900 — use non-volatile pointers
- JOYPAD register at 0x6F82 is active-HIGH in Mednafen
- Link order matters: game objects before common, system.lib before c900ml.lib

### Sprite Sizes
| Entity | Size | Tiles | Palette |
|---|---|---|---|
| Ship | 16×16 | 2×2 = 4 | PAL_SHIP (blue + orange thrust) |
| Alien | 32×16 | 4×2 = 8 | PAL_UFO (green) |
| Large rock | 16×16 | 2×2 = 4 | PAL_ROCK1 (bright grey) |
| Medium rock | 8×8 | 1 | PAL_ROCK2 (mid grey) |
| Small rock | 8×8 | 1 | PAL_ROCK3 (dim grey) |
| Bullet | 8×8 | 1 | PAL_SHIP |
| UFO shot | 8×8 | 1 | PAL_UFO |

## Roadmap
- [ ] Move entities to sprite plane (true transparency)
- [ ] Parallax scrolling starfield on scroll plane 2
- [ ] More asteroid variants, rotations
- [ ] Flash save verification on real hardware

## Acknowledgements
- **ameliandev** — [ngpc-project-template](https://github.com/ameliandev/ngpc-project-template):
  C framework that makes NGPC homebrew practical
- **Dark Fader / BlackThunder** — s242ngp ROM packer
- **Toshiba** — TLCS-900H toolchain (cc900, tulink, tuconv)
- **SNK** — Neo Geo Pocket Color hardware
- **Mednafen** — accurate NGP/NGPC emulation
- **Ed Logg & Lyle Rains** — original 1979 Atari Asteroids

## License
MIT — see [LICENSE](LICENSE)
