# Changelog

## v9

First stable public build.

- Verified RouterOS `On Line` and `On Battery` states.
- Verified `Low Battery`, `Replace Battery` and `Overload` flags.
- Uses APC-compatible `PowerSummary.PresentStatus` HID status layout.
- Simulated load reduced to 5%.
- Added `ON FLIPPER` mode using the real Flipper battery charge and voltage.
- Added battery-dependent green LED duty-cycle indication in BATTERY mode.
- Model/Version fields and MikroTik Beep remain unsupported in the tested RouterOS setup.
