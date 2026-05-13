# NGPC Asteroids

Classic space game for the Neo Geo Pocket Color:

- **Asteroids** — rocks, rotation, shoot

## Requirements

- Linux (tested on Linux Mint)
- Wine 5.0+
- Toshiba TLCS-900H toolchain (cc900, tulink, tuconv, s242ngp)
- Mednafen 1.22.2+ for testing
- GNU Make

## Quick Start

```bash
git clone <this repo>
cd ngp-asteroids

# Point at your toolchain and verify everything
TOOLCHAIN_SRC=/path/to/ngpcbins ./bootstrap.sh

# Build
make

# Run in emulator (mednafen in this case)
make run
```

## Project Structure

```
ngpc-space/
├── Makefile            # Build rules (reads toolchain.mk)
├── bootstrap.sh        # Environment setup / verification
├── toolchain.mk        # Auto-generated toolchain paths
├── common/             # Shared code across all games
│   ├── ngpc.h          #   Hardware registers
│   ├── carthdr.h       #   ROM header
│   └── library.c / .h  #   Support files
├── src/                # Game code
│   └── main.c          #   Entry point + game
├── lcf/
│   └── asteroids.lcf       # Linker command file
├── lib/                # system.rel if needed
├── build/              # Compiled objects (generated)
├── bin/                # ROM output (generated)
│   └── asteroids.ngp   # ROM image
├── assets/             # Tile data, sprites
└── tools/              # Asset conversion scripts
```

## Toolchain Notes

- cc900 is C89 only — no stdint, no mixed declarations, no typedef enums
- cc900 must run from its own BIN directory under Wine
- All paths through the compiler use `winepath -w` conversion
- See `common/ngpc.h` for hardware register reference

## Controls (Mednafen)

| Key | NGPC |
|-----|------|
| Left Joystick | Rotate Left |
| Right Joystick | Rotate Rght |
| Up Joystick | Thrust |
| A Button | Fire |
| B Button | Warp |
| Option | Menu |

## License

Homebrew — do what you want with it.
