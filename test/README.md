# mameTask-pico Tests

This directory contains tests for the mameTask-pico library. The tests are designed to run on both host (development computer) and device (Raspberry Pi Pico) platforms using a unified test structure.

## Directory Structure

```
test/
├── utest.h                 # Single-header unit testing framework
├── platform.h              # Platform abstraction layer
├── README.md               # This file
├── CMakeLists.txt          # CMake file for tests
├── pico_sdk_import.cmake   # CMake file for importing Pico SDK
├── test_main.cpp           # Main entry point for tests
├── test_task.cpp           # Tests for TaskCallable concept and ScheduledTask class
├── test_runner.cpp         # Tests for TaskRunner class
├── test_device.cpp         # Device-specific tests (only run on Pico)
└── mock/                   # Mock implementations for host testing
    └── pico/               # Mock Pico SDK directory structure
        └── async_context_poll.h  # Mock implementation of async_context_poll.h
```

## Unified Test Structure

The unified test structure allows the same test code to run on both host and device platforms. This is achieved through a platform abstraction layer (`platform.h`) that provides a consistent interface for platform-specific functionality.

### Platform Abstraction Layer

The platform abstraction layer provides:

- Platform initialization and cleanup
- Sleep functions
- Time functions
- GPIO operations
- Async context operations

This allows test code to be written once and run on both platforms with minimal platform-specific code.

## Building and Running Tests

### Using CMake

#### Host Tests

```bash
# Create a build directory
mkdir -p build
cd build

# Configure and build for host
cmake ..
make

# Run the tests
./test/mameTask_tests
```

#### Device Tests

```bash
# Set the PICO_SDK_PATH environment variable to point to your Pico SDK installation
export PICO_SDK_PATH=/path/to/pico-sdk

# Create a build directory
mkdir -p build
cd build

# Configure and build for Pico
cmake -DBUILD_FOR_PICO=ON ..
make

# This will generate mameTask_tests.uf2 in the build/test directory
```

### Running Device Tests

1. Connect your Raspberry Pi Pico to your computer via USB
2. Put the Pico into bootloader mode by holding the BOOTSEL button while connecting it
3. Copy the test binary (e.g., `mameTask_tests.uf2`) to the Pico
4. The tests will run automatically, and the results will be displayed via USB serial

## Adding New Tests

### Adding Common Tests

1. Create a new `.cpp` file in the `test` directory
2. Include the necessary headers:
   ```cpp
   #include "utest.h"
   #include "platform.h"
   #include "../src/mameTaskPico.hpp"
   ```
3. Use the platform abstraction layer for platform-specific functionality
4. Define your test cases using the `UTEST` macro
5. Add your test file to the `TEST_SOURCES` variable in the `test/CMakeLists.txt` file

### Adding Device-Specific Tests

1. Create a new `.cpp` file in the `test` directory
2. Include the necessary headers:
   ```cpp
   #include "utest.h"
   #include "platform.h"
   #include "../src/mameTaskPico.hpp"
   ```
3. Wrap your test cases with `#ifdef PLATFORM_DEVICE` to ensure they only run on the device
4. Define your test cases using the `UTEST` macro
5. Add your test file to the `DEVICE_SOURCES` variable in the `test/CMakeLists.txt` file
