# mameTask-pico Examples

This directory contains example programs that demonstrate the use of the mameTaskPico library.

## Available Examples

### Blink and Print

Located in the `blink_and_print` directory, this example demonstrates:
- Blinking an LED at a regular interval (500ms)
- Printing status messages at a regular interval (1000ms)

This example shows the basic usage of the mameTaskPico library for creating and running multiple tasks with different intervals.

## Building the Examples

Each example has its own build instructions in its respective directory. Generally, the process is:

1. Navigate to the example directory
2. Create a build directory: `mkdir build && cd build`
3. Configure the build: `cmake ..`
4. Build the project: `make`

## Requirements

To build and run these examples, you need:
- Raspberry Pi Pico SDK installed
- CMake (version 3.13 or higher)
- ARM GCC toolchain
- A Raspberry Pi Pico board

## Adding New Examples

To add a new example:
1. Create a new directory under `example/`
2. Add your source files and a CMakeLists.txt file
3. Update the top-level `example/CMakeLists.txt` to include your new example
