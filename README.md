# mameTask-pico

A lightweight, efficient, and easy-to-use task scheduling library for Raspberry Pi Pico SDK that enables cooperative multitasking with minimal overhead.

## Overview

mameTask-pico addresses the gap between bare-metal programming, full RTOS solutions, and ad-hoc state machines by providing a lightweight cooperative multitasking system that is simple to use yet powerful enough for many embedded applications.

### Key Features

- **Lightweight Implementation**
  - Minimal memory footprint
  - No dynamic memory allocation
  - Efficient execution with low overhead

- **Cooperative Multitasking**
  - Support for multiple concurrent tasks
  - Task scheduling with configurable intervals
  - Runtime configurable task timing

- **C++20 Compatibility**
  - Leverages modern C++ features
  - Uses concepts for type constraints
  - Employs immutable data structures for reliability

- **Pico SDK Integration**
  - Seamless integration with Raspberry Pi Pico SDK
  - Utilizes Pico SDK timers and async functionality
  - Compatible with existing Pico SDK projects

## Requirements

- Raspberry Pi Pico SDK (compatible with the latest version)
- C++20 Compatible Compiler (GCC 10+ or Clang 10+)
- CMake (version 3.13 or higher)
- Raspberry Pi Pico hardware (or compatible RP2040-based board)

## Installation

### As Part of Your Project

1. Clone this repository or add it as a submodule
   ```bash
   # As a submodule
   git submodule add https://github.com/yourusername/mameTask-pico.git
   
   # Or clone directly
   git clone https://github.com/yourusername/mameTask-pico.git
   ```

2. Include the library in your CMakeLists.txt
   ```cmake
   add_subdirectory(mameTask-pico)
   target_link_libraries(your_target mameTask-pico)
   ```

3. Include the header in your source files
   ```cpp
   #include "mameTaskPico.hpp"
   ```

## Quick Start

Here's a simple example that blinks an LED and prints a status message

```cpp
#include "pico/stdlib.h"
#include "mameTaskPico.hpp"

// LED blink task
void blink_led() {
    gpio_put(PICO_DEFAULT_LED_PIN, !gpio_get(PICO_DEFAULT_LED_PIN));
}

// Status print task
void print_status() {
    static uint32_t counter = 0;
    printf("System running: %lu seconds, counter: %lu\n", 
           time_us_64() / 1000000, counter++);
}

int main() {
    // Initialize hardware
    stdio_init_all();
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    
    // Create tasks with different intervals
    auto led_task = create_scheduled_task(500, blink_led);
    auto print_task = create_scheduled_task(1000, print_status);
    
    // Create task runner with both tasks
    TaskRunner runner(std::move(led_task), std::move(print_task));
    
    // Run tasks indefinitely
    runner.run_forever();
    
    return 0;
}
```

## API Reference

### Core Concepts

- **TaskCallable**: A concept that defines any callable entity that takes no arguments and returns void
- **ScheduledTaskInterface**: A concept that defines the interface for scheduled tasks

### Main Classes

#### ScheduledTask

Wraps a callable object and associates it with timing information.

```cpp
// Create a task that runs every 500ms
auto task = create_scheduled_task(500, []() {
    // Task code here
});

// Change the interval at runtime
task.set_interval(250);

// Get the current interval
unsigned current_interval = task.get_interval();
```

#### TaskRunner

Manages a collection of tasks and provides methods to poll and run them.

```cpp
// Create a task runner with multiple tasks
TaskRunner runner(std::move(task1), std::move(task2), std::move(task3));

// Poll once to execute any ready tasks
runner.poll();

// Or run indefinitely with a specific polling interval (default: 10ms)
runner.run_forever(20);
```

### Helper Functions

- **create_scheduled_task**: Creates a scheduled task with the given interval and callback

## Examples

The `example` directory contains sample programs demonstrating the use of mameTask-pico

### Blink and Print

Located in `example/blink_and_print`, this example demonstrates
- Blinking an LED at a regular interval (500ms)
- Printing status messages at a regular interval (1000ms)

### Building Examples

1. Navigate to the example directory
2. Create a build directory: `mkdir build && cd build`
3. Configure the build: `cmake ..`
4. Build the project: `make`
5. Flash the resulting `.uf2` file to your Pico

## Testing

mameTask-pico includes a comprehensive test suite that can run on both host (development computer) and device (Raspberry Pi Pico) platforms.

### Running Tests on Host

```bash
cd test
mkdir build && cd build
cmake ..
make
./mameTask_tests
```

### Running Tests on Device

```bash
cd test
mkdir build && cd build
cmake -DBUILD_FOR_PICO=ON ..
make
```

Then flash the resulting `mameTask_tests.uf2` file to your Pico.

## Features

- Core task scheduling with runtime configurable intervals
- Basic task runner with polling mechanism
- Helper functions for task creation

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add some amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgments

- Raspberry Pi Pico SDK team for the excellent SDK
- Contributors to the project
