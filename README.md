# Flipper Zero USB UPS Emulator for MikroTik RouterOS

Flipper Zero application that emulates an APC-compatible USB HID UPS. It was built to test MikroTik RouterOS UPS monitoring without a physical UPS connected.

Tested with:

- Flipper Zero FZ.1
- Unleashed 093, API 88.9
- MikroTik RouterOS with the `ups` package
- USB HID connection (`usbhidX`)

This is a test tool, not a UPS or power-protection device.

## What works

RouterOS status reporting was verified for:

- On Line
- On Battery
- Low Battery
- Replace Battery
- Overload
- Battery Charge
- Run Time Left

The HID profile also exposes battery voltage, input/output voltage, frequency, temperature and load.

Known limitations:

- RouterOS Model / Version fields stay empty in the tested setup.
- MikroTik `Beep` returns `Cannot beep USB ups (6)` and does not reach the Flipper.

## Modes

| Mode | Behaviour |
| --- | --- |
| `ONLINE` | Simulated UPS on utility power. |
| `BATTERY` | Simulated UPS running from battery. |
| `ON FLIPPER` | Battery percentage and battery voltage are read from the Flipper itself. |

Controls:

| Key | Function |
| --- | --- |
| Left / Right | Change mode |
| Up / Down | Change simulated battery charge by 5% |
| OK | `OK -> LOW -> REPL -> OVRL -> OK` |
| Back | Exit |

The normal simulated load is 5%. `OVRL` reports 100% load in addition to the overload status.

## LED indication

- No USB host: LED off
- USB configuration error: red
- ONLINE: solid green
- BATTERY: green 1 s duty-cycle indicator based on battery charge; bright-green time is proportional to charge and the rest is dim green
- ON FLIPPER: magenta

## Install on Flipper Zero

The prebuilt file is in:

```text
release/ups_emulator_v9_unleashed093.fap
```

Copy it to the microSD card, for example:

```text
/ext/apps/Tools/ups_emulator.fap
```

Start the app before connecting the Flipper to the MikroTik. Disconnect it from qFlipper/file-transfer mode first, then connect the Flipper USB-C port directly to the MikroTik USB host port with a data cable.

The app should change from `waiting` to `connected` after USB enumeration.

## MikroTik RouterOS setup

RouterOS needs the optional `ups` package. Install the package matching the exact RouterOS version and CPU architecture of the MikroTik. For example, the original test device used `mmips`.

MikroTik documentation:

- https://help.mikrotik.com/docs/spaces/ROS/pages/120324130/UPS
- https://help.mikrotik.com/docs/spaces/ROS/pages/40992872/Packages

After installing the package and rebooting:

1. Open `System -> UPS` in WinBox.
2. Add a UPS entry.
3. Select the detected USB HID port, for example `usbhid1`. The number may differ.
4. For test-bench use, set:

```text
Off Line Time = 00:00:00
Min Run Time   = 00:00:00
```

This prevents the emulated UPS state from intentionally hibernating the router based on runtime thresholds.

Example configuration:

```text
name="ups-flipper"
port=usbhid1
offline-time=0s
min-runtime=0s
```

Useful RouterOS commands:

```routeros
/system ups print detail
/system ups monitor 0
```

A working ONLINE state should include values similar to:

```text
on-line: yes
on-battery: no
battery-charge: 80%
```

Switching the Flipper to BATTERY should change the state to `on-battery: yes`.

## USB/HID implementation

The app implements a USB HID Power Device / Battery System profile.

For RouterOS compatibility testing it currently identifies as:

```text
VID:PID 051D:0002
```

This is an APC USB identity used only to trigger the RouterOS UPS driver. The project is not affiliated with APC or Schneider Electric and does not represent a real APC product.

The RouterOS status flags use the APC-compatible `PowerSummary.PresentStatus` report path. The status bit layout was verified against RouterOS using the debug build that preceded v9.

## Build

The current source targets Unleashed 093 / API 88.9.

Clone the matching firmware release:

```bash
git clone --recursive https://github.com/DarkFlippers/unleashed-firmware.git
cd unleashed-firmware
git checkout unlshd-093
git submodule update --init --recursive
```

Copy these files into:

```text
applications_user/ups_emulator/
```

Required files:

```text
application.fam
ups_emulator.c
ups_icon.png
```

Build the external app:

```bash
./fbt fap_ups_emulator
```

On Windows use `fbt.cmd` instead of `./fbt`.

Unleashed build documentation:

- https://github.com/DarkFlippers/unleashed-firmware/blob/dev/documentation/HowToBuild.md
- https://github.com/DarkFlippers/unleashed-firmware/blob/dev/documentation/fbt.md

## Version

Current public build: v9 stable.

Prebuilt target:

```text
Unleashed 093
API 88.9
HW target 7
Flipper Zero FZ.1
```
