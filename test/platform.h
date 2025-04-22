#pragma once

// Platform detection
#ifdef PICO_TARGET_NAME
    // We're building for the Pico
    #include "pico/stdlib.h"
    #define PLATFORM_DEVICE
#else
    // We're building for the host
    #define PLATFORM_HOST
    #include <chrono>
    #include <thread>
    #include <unordered_map>
    #include <functional>
    #include <vector>
    #include <algorithm>
    #include "mock/pico/async_context_poll.h"
#endif

// Common platform-independent interface
namespace test_platform {
    // Initialize platform-specific resources
    void init();
    
    // Clean up platform-specific resources
    void cleanup();
    
    // Sleep for the specified number of milliseconds
    void sleep_ms(uint32_t ms);
    
    // Get current time in microseconds
    uint64_t time_us_64();
    
    // Platform-specific GPIO operations
    namespace gpio {
        void init(unsigned int pin);
        void set_dir(unsigned int pin, bool out);
        void put(unsigned int pin, bool value);
        bool get(unsigned int pin);
    }
    
    // Async context operations
    namespace async {
        // Initialize an async context
        void init_context(async_context_poll_t* context);
        
        // Poll an async context
        void poll_context(async_context_t* context);
        
        // Add a worker to an async context with a delay in milliseconds
        void add_worker_in_ms(async_context_t* context, 
                             async_at_time_worker_t* worker, 
                             uint32_t ms);
    }
}

// Platform-specific implementations
#ifdef PLATFORM_DEVICE
// Device (Pico) implementation
namespace test_platform {
    inline void init() {
        stdio_init_all();
    }
    
    inline void cleanup() {
        // No special cleanup needed for device
    }
    
    inline void sleep_ms(uint32_t ms) {
        ::sleep_ms(ms);  // Use Pico SDK sleep_ms
    }
    
    inline uint64_t time_us_64() {
        return ::time_us_64();  // Use Pico SDK time_us_64
    }
    
    namespace gpio {
        inline void init(unsigned int pin) {
            gpio_init(pin);
        }
        
        inline void set_dir(unsigned int pin, bool out) {
            gpio_set_dir(pin, out);
        }
        
        inline void put(unsigned int pin, bool value) {
            gpio_put(pin, value);
        }
        
        inline bool get(unsigned int pin) {
            return gpio_get(pin);
        }
    }
    
    namespace async {
        inline void init_context(async_context_poll_t* context) {
            async_context_poll_init_with_defaults(context);
        }
        
        inline void poll_context(async_context_t* context) {
            async_context_poll(context);
        }
        
        inline void add_worker_in_ms(async_context_t* context, 
                                    async_at_time_worker_t* worker, 
                                    uint32_t ms) {
            async_context_add_at_time_worker_in_ms(context, worker, ms);
        }
    }
}
#else
// Host implementation
namespace test_platform {
    inline void init() {
        // No special initialization needed for host
    }
    
    inline void cleanup() {
        // No special cleanup needed for host
    }
    
    inline void sleep_ms(uint32_t ms) {
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    }
    
    inline uint64_t time_us_64() {
        auto now = std::chrono::high_resolution_clock::now();
        auto duration = now.time_since_epoch();
        return std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
    }
    
    namespace gpio {
        // Mock GPIO implementation for host
        static std::unordered_map<unsigned int, bool> gpio_values;
        
        inline void init(unsigned int pin) {
            gpio_values[pin] = false;
        }
        
        inline void set_dir(unsigned int pin, bool out) {
            // No-op for host
            (void)pin;
            (void)out;
        }
        
        inline void put(unsigned int pin, bool value) {
            gpio_values[pin] = value;
        }
        
        inline bool get(unsigned int pin) {
            return gpio_values[pin];
        }
    }
    
    namespace async {
        inline void init_context(async_context_poll_t* context) {
            if (context) {
                context->core.scheduled_workers.clear();
                context->core.current_time_us = time_us_64();
            }
        }
        
        inline void poll_context(async_context_t* context) {
            if (!context || context->scheduled_workers.empty()) {
                return;
            }
            
            // Update current time
            context->current_time_us = time_us_64();
            
            // Find workers that are ready to run
            std::vector<async_at_time_worker_t*> ready_workers;
            
            // Collect workers that are ready to run
            auto it = context->scheduled_workers.begin();
            while (it != context->scheduled_workers.end()) {
                if (it->first <= context->current_time_us) {
                    ready_workers.push_back(it->second);
                    it = context->scheduled_workers.erase(it);
                } else {
                    ++it;
                }
            }
            
            // Run all ready workers
            for (auto* worker : ready_workers) {
                if (worker && worker->do_work) {
                    worker->do_work(context, worker);
                }
            }
        }
        
        inline void add_worker_in_ms(async_context_t* context, 
                                    async_at_time_worker_t* worker, 
                                    uint32_t ms) {
            if (!context || !worker) {
                return;
            }
            
            // Calculate the time when the worker should run
            uint64_t run_time_us = context->current_time_us + (ms * 1000);
            
            // Schedule the worker
            context->scheduled_workers.push_back(std::make_pair(run_time_us, worker));
            
            // Sort workers by run time
            std::sort(context->scheduled_workers.begin(), context->scheduled_workers.end(),
                    [](const auto& a, const auto& b) { return a.first < b.first; });
        }
    }
}
#endif

// Define PICO_DEFAULT_LED_PIN for host if not already defined
#ifndef PICO_DEFAULT_LED_PIN
#define PICO_DEFAULT_LED_PIN 25
#endif
