#pragma once

#include <chrono>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "Async/Task.h"

namespace soul::time {

/**
 * @brief Frame-oriented orchestrator that sequences coroutine tasks with optional timing offsets.
 */
class FrameScheduler {
public:
    struct TaskHandle {
        soul::async::Task<void> task;
        soul::async::TaskToken token;
    };

    explicit FrameScheduler(std::shared_ptr<soul::async::TaskScheduler> scheduler);

    /**
     * @brief Schedules a coroutine for immediate execution, wired to the shared task scheduler.
     * @param name Unique identifier used to track dependencies across frames.
     * @param task Coroutine body to execute.
     * @param dependencies Optional list of prerequisite tasks that must complete first.
     */
    [[nodiscard]] TaskHandle schedule(std::string name,
                                      soul::async::Task<void> task,
                                      std::span<const soul::async::TaskToken> dependencies = {});

    /**
     * @brief Queues a coroutine to run after the provided delay has elapsed.
     */
    [[nodiscard]] TaskHandle schedule_after(std::chrono::nanoseconds delay,
                                            std::string name,
                                            soul::async::Task<void> task,
                                            std::span<const soul::async::TaskToken> dependencies = {});

    /**
     * @brief Blocks until all in-flight frame tasks reach completion.
     */
    void wait_for_all();

private:
    std::shared_ptr<soul::async::TaskScheduler> m_scheduler;
    std::unordered_map<std::string, soul::async::TaskToken> m_tokens;
};

} // namespace soul::time
