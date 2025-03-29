# Kepecs Bridge v2

Arduino-based dual stepper motor controller with magnetic sensor calibration.

## Prerequisites

1. Install the ESP32 board package in Arduino IDE following the [official guide](https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html)
2. Install the [StepperDriver library](https://github.com/laurb9/StepperDriver) in Arduino IDE

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

1. **Board Selection**
   - In Arduino IDE, select Tools -> Board -> esp32 -> Adafruit Feather ESP32-S3 2MB PSRAM
   - If not listed, ensure ESP32 board package is installed

2. **Upload Process**
   - If device is unresponsive, enter boot mode:
     1. Hold the Boot button
     2. Press and release the Reset button
     3. Release the Boot button
   - Upload sketch to ESP32
   - Open Serial Monitor at 115200 baud
   - Run `cal` command to calibrate
   - Use other commands to control motors

## Safety

- Calibration must be completed before motor movement is allowed
- Motor 0 has position limits (0 to 18000 steps)
- Motors are automatically disabled after movement 