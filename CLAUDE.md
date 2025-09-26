# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is a ZMK (Zephyr Mechanical Keyboard) firmware configuration repository for a **Sofle split keyboard** with Nice!Nano v2 controllers.

## Common Development Tasks

### Build Firmware Locally
ZMK firmware builds are typically done via GitHub Actions, but for local development:
```bash
# Install west (if not installed)
pip3 install west

# Initialize workspace (first time only)
west init -l config
west update

# Build for left half
west build -b nice_nano_v2 -- -DSHIELD=sofle_left

# Build for right half
west build -b nice_nano_v2 -- -DSHIELD=sofle_right
```

### GitHub Actions Build
Firmware automatically builds on push/PR via `.github/workflows/build.yml`. Download artifacts from the Actions tab.

### Flash Firmware
1. Put the keyboard half into bootloader mode (double-tap reset button)
2. Copy the appropriate `.uf2` file to the mounted drive (NICENANO)
3. The device will auto-reboot after flashing

## Architecture & Key Files

### Configuration Structure
- **`config/sofle.keymap`**: Main keymap definition using device tree syntax. Contains 4 layers (BASE, LOWER, RAISE, ADJUST) with conditional layer activation.
- **`config/sofle.conf`**: Hardware feature toggles (OLED display, encoders, RGB underglow)
- **`build.yaml`**: Build matrix defining left/right shield targets for GitHub Actions

### Key Bindings Architecture
- Uses ZMK behavior system with device tree bindings
- Layers activated via momentary layer keys (`mo LOWER`, `mo RAISE`)
- ADJUST layer auto-activates when both LOWER and RAISE are held (conditional layers)
- Encoders mapped to volume/page scrolling via `sensor-bindings`

### Hardware Features
- **OLED Display**: Enabled (`CONFIG_ZMK_DISPLAY=y`)
- **Rotary Encoders**: Enabled (`CONFIG_EC11=y`)
- **RGB Underglow**: Available but disabled (uncomment in `sofle.conf` to enable)
- **Bluetooth**: Multi-profile support with device switching in ADJUST layer

## Development Notes

### Modifying Keymaps
When editing `sofle.keymap`:
- Use ZMK keycodes from `dt-bindings/zmk/keys.h`
- Maintain the visual grid alignment in binding arrays for readability
- Test changes locally before pushing (GitHub Actions will validate)

### Adding Features
- **RGB**: Uncomment `CONFIG_ZMK_RGB_UNDERGLOW=y` in `sofle.conf`
- **Macros**: Add macro definitions in the root device tree node
- **Combos**: Define combo behaviors for multi-key shortcuts
- **Custom Behaviors**: Use ZMK's behavior system for advanced key actions

### Troubleshooting Builds
- Check GitHub Actions logs for detailed error messages
- Ensure shield names match exactly: `sofle_left` and `sofle_right`
- Verify keycode names against ZMK documentation