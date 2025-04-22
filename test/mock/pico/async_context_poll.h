#pragma once

#include <cstdint>
#include <functional>
#include <thread>
#include <chrono>
#include <vector>
#include <queue>
#include <utility>
#include <algorithm>

// Forward declarations
inline uint64_t time_us_64();

// Simplified mock structures to replace Pico SDK dependencies
struct async_context_t {
    // Store scheduled workers for timing tests
    std::vector<std::pair<uint64_t, struct async_at_time_worker_t*>> scheduled_workers;
    uint64_t current_time_us;
};

struct async_at_time_worker_t {
    void (*do_work)(async_context_t*, async_at_time_worker_t*);
    void* user_data;
};

struct async_context_poll_t {
    async_context_t core;
};

// Mock implementations of Pico SDK functions
inline void async_context_poll_init_with_defaults(async_context_poll_t* context) {
    // Initialize the context
    if (context) {
        context->core.scheduled_workers.clear();
        context->core.current_time_us = time_us_64();
    }
}

inline void async_context_poll(async_context_t* context) {
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

inline void async_context_add_at_time_worker_in_ms(async_context_t* context, 
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

inline void sleep_ms(uint32_t ms) {
    // Platform-specific sleep implementation
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

// Additional mock functions that might be needed
inline uint64_t time_us_64() {
    // Return current time in microseconds
    auto now = std::chrono::high_resolution_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
}
