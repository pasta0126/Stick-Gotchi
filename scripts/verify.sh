#!/usr/bin/env bash
set -euo pipefail

echo "Building firmware (pio run)"
pio run

echo "Validation completed successfully"
echo "Note: this only verifies the firmware compiles. Button input, display rendering,"
echo "IMU/mic behavior, and power handling still require manual verification on an"
echo "M5Stick C Plus2 (pio run --target upload && pio device monitor)."
