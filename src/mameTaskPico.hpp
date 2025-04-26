#pragma once

#include <concepts>
#include <functional>
#include <memory>
#include <type_traits>
#include <utility>

#include <pico/async_context_poll.h>

/**
 * @brief Concept for callable objects that can be used as tasks
 *
 * Requires that the object can be called with no arguments and returns void
 */
template<typename F>
concept TaskCallable = requires(F f) {
  { f() } -> std::same_as<void>;
};

/**
 * @brief Concept for any type that can be used as a scheduled task
 *
 * Requires that the object has a get_native_worker method that returns
 * a reference to an async_at_time_worker_t
 */
template<typename T>
concept ScheduledTaskInterface = requires(T t) {
  { t.get_native_worker() } -> std::same_as<async_at_time_worker_t&>;
};

// Forward declarations for internal implementation details
template<TaskCallable F>
class ScheduledTask;

/**
 * @brief Wrapper class for the async context to encapsulate PICO SDK dependencies
 *
 * Manages a collection of scheduled tasks and provides methods to poll and run them
 */
template<ScheduledTaskInterface... Tasks>
class TaskRunner
{
private:
  async_context_poll_t context;
  std::tuple<Tasks...> tasks;

public:
  /**
   * @brief Constructs a TaskRunner with the given tasks
   *
   * @param args The scheduled tasks to run
   */
  TaskRunner(Tasks&&... args)
    : tasks(std::make_tuple(std::forward<Tasks>(args)...))
  {
    async_context_poll_init_with_defaults(&context);

    std::apply(
      [this](auto&... task)
      { (async_context_add_at_time_worker_in_ms(&context.core, &task.get_native_worker(), 0), ...); },
      tasks
    );
  }

  ~TaskRunner() = default;

  // Prevent copying to avoid resource management issues
  TaskRunner(const TaskRunner&)            = delete;
  TaskRunner& operator=(const TaskRunner&) = delete;

  /**
   * @brief Polls the async context once to execute any ready tasks
   */
  void poll() { async_context_poll(&context.core); }

  /**
   * @brief Runs the task loop indefinitely, polling at regular intervals
   *
   * @param poll_interval_ms Time between polls in milliseconds (default: 10ms)
   */
  void run_forever(uint32_t poll_interval_ms = 10)
  {
    while (true)
    {
      poll();
      sleep_ms(poll_interval_ms);
    }
  }
};

/**
 * @brief Wrapper class for a scheduled task to encapsulate PICO SDK dependencies
 *
 * @tparam F The type of the callable object
 */
template<TaskCallable F>
class ScheduledTask
{
private:
  async_at_time_worker_t worker;
  F                      callback;
  unsigned const         interval;

public:
  /**
   * @brief Constructs a ScheduledTask with the given interval and callback
   *
   * @param interval The interval in milliseconds at which to run the task
   * @param callback The function to call when the task is executed
   */
  ScheduledTask(unsigned interval, F&& callback)
    : callback(std::forward<F>(callback))
    , interval(interval)
  {
    // Create worker
    worker = { .do_work =
                 [](async_context_t* context, async_at_time_worker_t* worker)
               {
                 auto* self = reinterpret_cast<ScheduledTask*>(worker->user_data);
                 self->callback();
                 async_context_add_at_time_worker_in_ms(context, worker, self->interval);
               },
               .user_data = reinterpret_cast<void*>(this) };
  }

  /**
   * @brief Gets the current interval for the task
   *
   * @return The current interval in milliseconds
   */
  unsigned get_interval() const
  {
    return interval;
  }

  // Allow moving
  ScheduledTask(ScheduledTask&&)            = default;
  ScheduledTask& operator=(ScheduledTask&&) = default;

  // Prevent copying to avoid resource management issues
  ScheduledTask(const ScheduledTask&)            = delete;
  ScheduledTask& operator=(const ScheduledTask&) = delete;

  /**
   * @brief Gets the native worker for this task
   *
   * @return Reference to the async_at_time_worker_t
   */
  auto& get_native_worker() { return worker; }
};

/**
 * @brief Creates a scheduled task with the given interval and callback
 *
 * @param interval The interval in milliseconds at which to run the task
 * @tparam F The type of the callable object
 * @param callback The function to call when the task is executed
 * @return A ScheduledTask object
 */
template<TaskCallable F>
auto create_scheduled_task(unsigned interval, F&& callback)
{
  return ScheduledTask<F>(interval, std::forward<F>(callback));
}
