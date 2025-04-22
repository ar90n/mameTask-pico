# Blink and Print Example

This example demonstrates the use of the mameTaskPico library by:
1. Blinking an LED at a regular interval (250ms)
2. Printing status messages at a regular interval (1000ms)

## Overview

The example creates two tasks:
- A task that toggles the onboard LED every 250ms
- A task that prints a status message every 1000ms

These tasks are managed by a TaskRunner that schedules and executes them according to their intervals.

## Building the Example

To build this example, you need to have the Raspberry Pi Pico SDK installed and set install path to `PICO_SDK_PATH`. Then follow these steps:

1. Create a build directory:
   ```
   mkdir build
   cd build
   ```

2. Configure the build:
   ```
   cmake ..
   ```

3. Build the project:
   ```
   make
   ```

This will generate a `blink_and_print.uf2` file that can be flashed to the Raspberry Pi Pico.

## Running the Example

1. Connect your Raspberry Pi Pico to your computer while holding the BOOTSEL button.
2. Release the button after connecting. The Pico should appear as a USB mass storage device.
3. Copy the `blink_and_print.uf2` file to the Pico.
4. The Pico will automatically reboot and run the example.

## Expected Behavior

- The onboard LED will blink at a 250ms interval (on for 250ms, off for 250ms).
- Every second, a status message will be printed to the USB serial output.
- You can view these messages by connecting to the Pico's USB serial port using a terminal program.

## Code Explanation

The example demonstrates:
- How to create tasks with different intervals
- How to combine multiple tasks in a TaskRunner
- How to run tasks concurrently using the mameTaskPico library

The key components are:
- `create_scheduled_task()` - Creates a task with a specified interval
- `TaskRunner` - Manages and executes the tasks
- `run_forever()` - Runs the tasks indefinitely
