#include "utest.h"
#include "platform.h"
#include "../src/mameTaskPico.hpp"
#include <vector>
#include <chrono>

// Global counters for tests
static int g_counter1 = 0;
static int g_counter2 = 0;
static int g_counter3 = 0;

// Reset all global counters
static void reset_counters() {
    g_counter1 = 0;
    g_counter2 = 0;
    g_counter3 = 0;
}

// Define a mock task that implements ScheduledTaskInterface
struct MockTask {
    async_at_time_worker_t worker;
    int* counter_ptr;
    unsigned interval_ms;
    
    MockTask(int* counter, unsigned interval = 0) 
        : counter_ptr(counter), interval_ms(interval) {
        worker.do_work = [](async_context_t* context, async_at_time_worker_t* worker) {
            // Cast back to MockTask to access the counter
            MockTask* self = reinterpret_cast<MockTask*>(worker->user_data);
            
            // Increment the counter
            (*self->counter_ptr)++;
            
            // Reschedule the task
            async_context_add_at_time_worker_in_ms(context, worker, self->interval_ms);
        };
        worker.user_data = this;
    }
    
    async_at_time_worker_t& get_native_worker() { return worker; }
    
    void set_interval(unsigned new_interval) {
        interval_ms = new_interval;
    }
    
    unsigned get_interval() const {
        return interval_ms;
    }
};

// Define a mock task that records execution times
class TimingMockTask {
public:
    async_at_time_worker_t worker;
    std::vector<uint64_t>* times_ptr;
    unsigned interval_ms;
    
    TimingMockTask(std::vector<uint64_t>* times, unsigned interval = 0) 
        : times_ptr(times), interval_ms(interval) {
        worker.do_work = [](async_context_t* context, async_at_time_worker_t* worker) {
            // Cast back to TimingMockTask to access the data
            TimingMockTask* self = reinterpret_cast<TimingMockTask*>(worker->user_data);
            
            // Record the execution time
            self->times_ptr->push_back(test_platform::time_us_64());
            
            // Reschedule the task
            async_context_add_at_time_worker_in_ms(context, worker, self->interval_ms);
        };
        worker.user_data = this;
    }
    
    async_at_time_worker_t& get_native_worker() { return worker; }
    
    void set_interval(unsigned new_interval) {
        interval_ms = new_interval;
    }
    
    unsigned get_interval() const {
        return interval_ms;
    }
};

// Test that the ScheduledTaskInterface concept works with our mock implementation
UTEST(ScheduledTaskInterface, MockTaskImplementation) {
    // Verify that MockTask satisfies ScheduledTaskInterface
    static_assert(ScheduledTaskInterface<MockTask>, "MockTask should satisfy ScheduledTaskInterface");
    
    // Create a counter
    int counter = 0;
    
    // Create a mock task
    MockTask task(&counter);
    
    // Verify that we can access the worker
    async_at_time_worker_t& worker = task.get_native_worker();
    (void)worker; // Prevent unused variable warning
    
    ASSERT_TRUE(true);
}

// Test that tasks are executed correctly
UTEST(TaskRunner, TaskExecution) {
    // Reset counters
    reset_counters();
    
    // Create a mock task
    MockTask task(&g_counter1);
    
    // Create a TaskRunner
    TaskRunner runner(std::move(task));
    
    // Poll multiple times
    for (int i = 0; i < 5; i++) {
        runner.poll();
    }
    
    // Verify that the task was executed multiple times
    ASSERT_GT(g_counter1, 0);
}

// Test that tasks are executed at the correct intervals
UTEST(TaskRunner, ExecutionTiming) {
    // Create a vector to store execution times
    std::vector<uint64_t> execution_times;
    
    // Create a timing mock task with a 50ms interval
    TimingMockTask task(&execution_times, 50);
    
    // Create a TaskRunner
    TaskRunner runner(std::move(task));
    
    // Poll and sleep to allow the task to execute multiple times
    for (int i = 0; i < 5; i++) {
        runner.poll();
        test_platform::sleep_ms(20); // Sleep less than the interval to ensure we don't miss executions
    }
    
    // Verify that we have at least 2 execution times
    ASSERT_GE(execution_times.size(), 2);
    
    // Verify that the intervals between executions are approximately correct
    for (size_t i = 1; i < execution_times.size(); i++) {
        uint64_t interval_us = execution_times[i] - execution_times[i-1];
        uint64_t interval_ms = interval_us / 1000;
        
        // Allow for some timing variation (±20ms)
        ASSERT_GE(interval_ms, 30);
        ASSERT_LE(interval_ms, 70);
    }
}

// Test that tasks with different priorities are executed in the correct order
UTEST(TaskRunner, MultipleTaskPriority) {
    // Reset counters
    reset_counters();
    
    // Create a vector to track execution order
    std::vector<int> execution_order;
    
    // Create mock tasks with different intervals (priority)
    struct PriorityMockTask {
        async_at_time_worker_t worker;
        unsigned interval_ms;
        int task_id;
        std::vector<int>* order_ptr;
        
        PriorityMockTask(int id, std::vector<int>* order, unsigned interval) 
            : interval_ms(interval), task_id(id), order_ptr(order) {
            worker.do_work = [](async_context_t* context, async_at_time_worker_t* worker) {
                // Cast back to PriorityMockTask to access the data
                PriorityMockTask* self = reinterpret_cast<PriorityMockTask*>(worker->user_data);
                
                // Record the task ID in the execution order
                self->order_ptr->push_back(self->task_id);
                
                // Reschedule the task with the current interval
                async_context_add_at_time_worker_in_ms(context, worker, self->interval_ms);
            };
            worker.user_data = this;
        }
        
        async_at_time_worker_t& get_native_worker() { return worker; }
    };
    
    // Create tasks with different priorities (intervals)
    // Task 1: High priority (10ms)
    // Task 2: Medium priority (20ms)
    // Task 3: Low priority (30ms)
    PriorityMockTask task1(1, &execution_order, 10);
    PriorityMockTask task2(2, &execution_order, 20);
    PriorityMockTask task3(3, &execution_order, 30);
    
    // Create a context for testing
    async_context_poll_t context;
    async_context_poll_init_with_defaults(&context);
    
    // Add tasks to the context
    async_context_add_at_time_worker_in_ms(&context.core, &task1.get_native_worker(), task1.interval_ms);
    async_context_add_at_time_worker_in_ms(&context.core, &task2.get_native_worker(), task2.interval_ms);
    async_context_add_at_time_worker_in_ms(&context.core, &task3.get_native_worker(), task3.interval_ms);
    
    // Poll multiple times with sleep to allow tasks to be scheduled based on their intervals
    for (int i = 0; i < 10; i++) {
        async_context_poll(&context.core);
        test_platform::sleep_ms(5); // Sleep a short time to allow for scheduling
    }
    
    // Verify that we have at least a few executions
    ASSERT_GE(execution_order.size(), 3);
    
    // Count the number of executions for each task
    int count1 = 0, count2 = 0, count3 = 0;
    for (int id : execution_order) {
        if (id == 1) count1++;
        else if (id == 2) count2++;
        else if (id == 3) count3++;
    }
    
    // Verify that higher priority tasks (shorter intervals) executed more frequently
    ASSERT_GE(count1, count2); // Task 1 should execute at least as often as Task 2
    ASSERT_GE(count2, count3); // Task 2 should execute at least as often as Task 3
}

// Platform-specific tests
#ifdef PLATFORM_DEVICE
// Test running tasks on the device with LED blinking
UTEST(DeviceSpecific, LEDBlinkTask) {
    // Initialize the LED pin
    test_platform::gpio::init(PICO_DEFAULT_LED_PIN);
    test_platform::gpio::set_dir(PICO_DEFAULT_LED_PIN, true); // true = output
    
    // LED state
    static int led_state = 0;
    
    // Create a task to toggle the LED
    auto toggle_led = []() {
        led_state = !led_state;
        test_platform::gpio::put(PICO_DEFAULT_LED_PIN, led_state);
    };
    
    // Create a scheduled task
    auto task = create_scheduled_task(100, toggle_led);
    
    // Create a TaskRunner
    TaskRunner runner(std::move(task));
    
    // Initial LED state should be off
    ASSERT_EQ(led_state, 0);
    
    // Poll to toggle the LED
    runner.poll();
    
    // LED state should be toggled
    ASSERT_EQ(led_state, 1);
    
    // Poll again to toggle the LED back
    runner.poll();
    
    // LED state should be toggled again
    ASSERT_EQ(led_state, 0);
}
#endif
