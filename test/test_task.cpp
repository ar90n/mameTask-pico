#include "utest.h"
#include "platform.h"
#include "../src/mameTaskPico.hpp"

// Global counter for tests
static int g_counter = 0;

// Reset counter
static void reset_counter() {
    g_counter = 0;
}

// Simple task function for testing
static void increment_counter() {
    g_counter++;
}

// Test that the TaskCallable concept works with a function pointer
UTEST(TaskCallable, FunctionPointer) {
    // Verify that increment_counter satisfies TaskCallable
    static_assert(TaskCallable<decltype(increment_counter)>, 
                 "Function pointer should satisfy TaskCallable");
    ASSERT_TRUE(true);
}

// Test that the TaskCallable concept works with a lambda
UTEST(TaskCallable, Lambda) {
    // Create a lambda
    auto lambda = []() { g_counter++; };
    
    // Verify that the lambda satisfies TaskCallable
    static_assert(TaskCallable<decltype(lambda)>, 
                 "Lambda should satisfy TaskCallable");
    ASSERT_TRUE(true);
}

// Test that the TaskCallable concept rejects invalid callables
UTEST(TaskCallable, InvalidCallable) {
    // Create an invalid lambda (takes an argument)
    auto invalid_lambda = [](int x) { (void)x; };
    
    // Verify that the invalid lambda does not satisfy TaskCallable
    static_assert(!TaskCallable<decltype(invalid_lambda)>, 
                 "Lambda with arguments should not satisfy TaskCallable");
    ASSERT_TRUE(true);
}

// Define a mock task that implements ScheduledTaskInterface
struct MockScheduledTask {
    async_at_time_worker_t worker;
    unsigned interval;
    
    MockScheduledTask(unsigned initial_interval) : interval(initial_interval) {
        worker.do_work = [](async_context_t* context, async_at_time_worker_t* worker) {
            // Cast back to MockScheduledTask to access the interval
            MockScheduledTask* self = reinterpret_cast<MockScheduledTask*>(worker->user_data);
            
            // Increment the global counter
            g_counter++;
            
            // Reschedule the task with the current interval
            async_context_add_at_time_worker_in_ms(context, worker, self->interval);
        };
        worker.user_data = this;
    }
    
    async_at_time_worker_t& get_native_worker() { return worker; }
    
    void set_interval(unsigned new_interval) {
        interval = new_interval;
    }
    
    unsigned get_interval() const {
        return interval;
    }
};

// Test that our mock task satisfies ScheduledTaskInterface
UTEST(ScheduledTaskInterface, MockImplementation) {
    // Verify that MockScheduledTask satisfies ScheduledTaskInterface
    static_assert(ScheduledTaskInterface<MockScheduledTask>, 
                 "MockScheduledTask should satisfy ScheduledTaskInterface");
    
    // Create a mock task
    MockScheduledTask task(100);
    
    // Verify that we can access the worker
    async_at_time_worker_t& worker = task.get_native_worker();
    (void)worker; // Prevent unused variable warning
    
    ASSERT_TRUE(true);
}

// Test that the mock task's interval methods work correctly
UTEST(ScheduledTask, IntervalMethods) {
    // Create a mock task with an initial interval
    MockScheduledTask task(100);
    
    // Verify initial interval
    ASSERT_EQ(task.get_interval(), 100);
    
    // Change interval
    task.set_interval(200);
    
    // Verify new interval
    ASSERT_EQ(task.get_interval(), 200);
}

// Test that the mock task executes the callback when triggered
UTEST(ScheduledTask, Execution) {
    // Reset counter
    reset_counter();
    
    // Create a mock task
    MockScheduledTask task(100);
    
    // Get the worker
    auto& worker = task.get_native_worker();
    
    // Create a mock context
    async_context_t context;
    context.current_time_us = test_platform::time_us_64();
    
    // Manually trigger the worker
    worker.do_work(&context, &worker);
    
    // Verify that the callback was executed
    ASSERT_EQ(g_counter, 1);
    
    // Trigger again
    worker.do_work(&context, &worker);
    
    // Verify that the callback was executed again
    ASSERT_EQ(g_counter, 2);
    
    // Sleep a bit to demonstrate platform abstraction
    test_platform::sleep_ms(10);
}

// Test that the mock task works with lambdas
UTEST(ScheduledTask, Lambda) {
    // Reset counter
    reset_counter();
    
    // Create a lambda that increments the counter
    auto lambda = []() {
        g_counter++;
    };
    
    // Verify that the lambda satisfies TaskCallable
    static_assert(TaskCallable<decltype(lambda)>, 
                 "Lambda should satisfy TaskCallable");
    
    // Create a mock task that uses a lambda
    struct LambdaMockTask {
        async_at_time_worker_t worker;
        unsigned interval;
        std::function<void()> callback;
        
        LambdaMockTask(unsigned initial_interval, std::function<void()> cb) 
            : interval(initial_interval), callback(cb) {
            worker.do_work = [](async_context_t* context, async_at_time_worker_t* worker) {
                // Cast back to LambdaMockTask to access the callback
                LambdaMockTask* self = reinterpret_cast<LambdaMockTask*>(worker->user_data);
                
                // Call the callback
                self->callback();
                
                // Reschedule the task with the current interval
                async_context_add_at_time_worker_in_ms(context, worker, self->interval);
            };
            worker.user_data = this;
        }
        
        async_at_time_worker_t& get_native_worker() { return worker; }
    };
    
    // Create a mock task with the lambda
    LambdaMockTask task(100, lambda);
    
    // Get the worker
    auto& worker = task.get_native_worker();
    
    // Create a mock context
    async_context_t context;
    context.current_time_us = test_platform::time_us_64();
    
    // Manually trigger the worker
    worker.do_work(&context, &worker);
    
    // Verify that the callback was executed
    ASSERT_EQ(g_counter, 1);
}

// Test that changing the interval of a task works correctly
UTEST(ScheduledTask, DynamicIntervalChange) {
    // Reset counter
    reset_counter();
    
    // Create a mock task with an initial interval
    MockScheduledTask task(100);
    
    // Verify initial interval
    ASSERT_EQ(task.get_interval(), 100);
    
    // Change interval
    task.set_interval(200);
    
    // Verify new interval
    ASSERT_EQ(task.get_interval(), 200);
    
    // Get the worker
    auto& worker = task.get_native_worker();
    
    // Create a mock context
    async_context_t context;
    context.current_time_us = test_platform::time_us_64();
    
    // Manually trigger the worker
    worker.do_work(&context, &worker);
    
    // Verify that the callback was executed
    ASSERT_EQ(g_counter, 1);
    
    // The worker should have been rescheduled with the new interval
    // We can't directly test this with the current mock implementation,
    // but we can verify that the interval is still set correctly
    ASSERT_EQ(task.get_interval(), 200);
}

// Platform-specific tests
#ifdef PLATFORM_DEVICE
// Test using actual GPIO on the device
UTEST(DeviceSpecific, GPIOTest) {
    // Initialize the LED pin
    test_platform::gpio::init(PICO_DEFAULT_LED_PIN);
    test_platform::gpio::set_dir(PICO_DEFAULT_LED_PIN, true); // true = output
    
    // Turn LED on
    test_platform::gpio::put(PICO_DEFAULT_LED_PIN, true);
    
    // Verify LED state
    ASSERT_TRUE(test_platform::gpio::get(PICO_DEFAULT_LED_PIN));
    
    // Turn LED off
    test_platform::gpio::put(PICO_DEFAULT_LED_PIN, false);
    
    // Verify LED state
    ASSERT_FALSE(test_platform::gpio::get(PICO_DEFAULT_LED_PIN));
}
#endif
