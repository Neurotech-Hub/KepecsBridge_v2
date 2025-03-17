# Kepecs Bridge v2

Arduino-based dual stepper motor controller with magnetic sensor calibration.

## Features

- Dual A4988 stepper motor control
- Magnetic sensor-based calibration
- Position tracking and limits
- Percentage-based positioning
- Serial command interface

## Commands

- `cal` - Run calibration routine
- `mag` - Monitor magnetic sensor state
- `p0-100` - Move to percentage of range (e.g., `p50`)
- `0:steps` - Move motor 0 by steps (e.g., `0:1000`)
- `1:steps` - Move motor 1 by steps (e.g., `1:1000`)
- `help` - Display command menu

## Requirements

- Arduino ESP32
- 2x A4988 stepper drivers
- Magnetic sensor
- 2x Stepper motors

## Setup

1. Upload sketch to ESP32
2. Open Serial Monitor at 115200 baud
3. Run `cal` command to calibrate
4. Use other commands to control motors

## Safety

- Calibration must be completed before motor movement is allowed
- Motor 0 has position limits (0 to 18000 steps)
- Motors are automatically disabled after movement 