#include "utest.h"
#include "platform.h"
#include "../src/mameTaskPico.hpp"

// This file contains tests that are specific to the Pico hardware
// and will only be compiled and run when building for the device

#ifdef PLATFORM_DEVICE

// Global variables for test state
static int led_state = 0;
static int counter = 0;

// Simple task to toggle the LED
void toggle_led() {
    led_state = !led_state;
    test_platform::gpio::put(PICO_DEFAULT_LED_PIN, led_state);
}

// Simple task to increment a counter
void increment_counter() {
    counter++;
}

// Test that a task can be created and executed on the device
UTEST(DeviceSpecific, TaskCreation) {
    // Create a task to toggle the LED
    auto task = create_scheduled_task(100, toggle_led);
    
    // Verify the interval
    ASSERT_EQ(task.get_interval(), 100);
    
    // Get the worker and manually trigger it
    async_at_time_worker_t& worker = task.get_native_worker();
    async_context_t context;
    context.current_time_us = test_platform::time_us_64();
    
    // Initial LED state should be off
    ASSERT_EQ(led_state, 0);
    
    // Execute the worker
    worker.do_work(&context, &worker);
    
    // LED state should be toggled
    ASSERT_EQ(led_state, 1);
    
    // Execute again
    worker.do_work(&context, &worker);
    
    // LED state should be toggled again
    ASSERT_EQ(led_state, 0);
}

// Test that multiple tasks can be run with the TaskRunner on the device
UTEST(DeviceSpecific, TaskRunner) {
    // Reset test state
    led_state = 0;
    counter = 0;
    
    // Create tasks
    auto led_task = create_scheduled_task(100, toggle_led);
    auto counter_task = create_scheduled_task(50, increment_counter);
    
    // Create a TaskRunner with both tasks
    TaskRunner runner(std::move(led_task), std::move(counter_task));
    
    // Poll the runner a few times
    for (int i = 0; i < 5; i++) {
        runner.poll();
        test_platform::sleep_ms(10);
    }
    
    // Counter should have been incremented multiple times
    // We can't assert the exact value because it depends on timing
    printf("Counter value after polling: %d\n", counter);
    ASSERT_TRUE(counter > 0);
}

// Test hardware-specific functionality
UTEST(DeviceHardware, GPIO) {
    // Initialize multiple GPIO pins
    for (int pin = 0; pin < 5; pin++) {
        test_platform::gpio::init(pin);
        test_platform::gpio::set_dir(pin, true); // Output
        
        // Set pin high
        test_platform::gpio::put(pin, true);
        ASSERT_TRUE(test_platform::gpio::get(pin));
        
        // Set pin low
        test_platform::gpio::put(pin, false);
        ASSERT_FALSE(test_platform::gpio::get(pin));
    }
}

// Test timing accuracy on the device
UTEST(DeviceHardware, Timing) {
    // Get initial time
    uint64_t start_time = test_platform::time_us_64();
    
    // Sleep for a known duration
    test_platform::sleep_ms(100);
    
    // Get end time
    uint64_t end_time = test_platform::time_us_64();
    
    // Calculate elapsed time in milliseconds
    uint64_t elapsed_ms = (end_time - start_time) / 1000;
    
    // Verify that the elapsed time is approximately correct
    // Allow for some timing variation (±20ms)
    printf("Elapsed time: %" PRIu64 " ms\n", elapsed_ms);
    ASSERT_GE(elapsed_ms, 80);
    ASSERT_LE(elapsed_ms, 120);
}

#endif // PLATFORM_DEVICE
