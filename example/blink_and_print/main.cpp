/**
 * @file main.cpp
 * @brief Example program for mameTaskPico library
 * 
 * This example demonstrates the use of the mameTaskPico library by:
 * 1. Blinking an LED at a regular interval (500ms)
 * 2. Printing status messages at a regular interval (1000ms)
 */

#include "pico/stdlib.h"
#include "../../src/mameTaskPico.hpp"

// LED blink task - toggles the LED state
void blink_led() {
    // Toggle the LED state
    gpio_put(PICO_DEFAULT_LED_PIN, !gpio_get(PICO_DEFAULT_LED_PIN));
}

// Text output task - prints status information
void print_status() {
    static uint32_t counter = 0;
    printf("System running: %f seconds, counter: %lu\n", 
           time_us_64() / 1000000.0, counter++);
}

int main() {
    // Initialize hardware
    stdio_init_all();
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    
    // Create tasks with different intervals
    auto led_task = create_scheduled_task(250, blink_led);
    auto print_task = create_scheduled_task(1000, print_status);
    
    // Create task runner with both tasks
    TaskRunner runner(std::move(led_task), std::move(print_task));
    
    // Run tasks indefinitely
    runner.run_forever();
    
    return 0;
}
