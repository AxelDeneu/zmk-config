# Repository guidance

ZMK v0.3 configuration for a Sofle split keyboard with nice!nano v2 controllers.
Read `README.md` for the layer map, Studio setup, builds, and hardware checks.

## Configuration

- `config/sofle.keymap`: five layers, 60 bindings per layer. Layer order is
  BASE (0), LOWER (1), RAISE (2), ADJUST (3), DEV (4).
- LOWER + RAISE activates DEV through `conditional_layers`. ADJUST is reached
  by holding LOWER and the top-right key; it is not a conditional layer.
- `config/sofle.conf`: shared OLED, encoder, RGB and idle settings.
- `config/sofle_left.conf`: central-only modifier indicators and Raw HID.
- `config/sofle_right.conf`: peripheral-only Smart Battery animation settings.
- `config/west.yml`: ZMK v0.3 and external modules pinned to commits.
- `build.yaml`: left build with Studio USB, right build without Studio.
- `.github/workflows/build.yml`: builds both halves using the ZMK v0.3 workflow.

## Changes and verification

Use the existing `FR_*` codes from `<locale/keys_fr.h>` for French characters.
Do not redefine locale codes or add Shift to already shifted `FR_N*` codes.
Keep layer indices and the left-to-right binding order consistent with Sofle's
physical layout. Update the visual legends when changing bindings.

Compile both halves after keymap or configuration changes. Keep Studio and
Raw HID on the central half only. Keep the explicit Smart Battery animation
timing: the pinned OLED module lacks a default for that animation.

The build catches configuration errors, but USB enumeration, AZERTY output,
encoders, OLED, RGB and split operation require tests on the keyboard.
Studio stores edits on the device; it does not update the repository. Its
saved mapping takes precedence over a newly flashed `.keymap` until
"Restore Stock Settings" is used.
