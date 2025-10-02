#pragma once

#include <chrono>
#include <memory>
#include <span>
#include <string>
#include <unordered_map>
#include <vector>

#include "Async/Task.h"

namespace soul::time {

/**
 * @brief Frame-oriented orchestrator that sequences coroutine tasks with optional timing offsets.
 * @details The scheduler provides a thin coordination layer on top of `soul::async::TaskScheduler`
 *          so systems can register jobs by name, express explicit dependencies, and defer
 *          execution by a fixed duration. Tokens returned from `schedule` or `schedule_after`
 *          can be passed to other systems or reused in subsequent frames to build DAGs.
 */
class FrameScheduler {
public:
    struct TaskHandle {
        /**
         * @brief Awaitable instance tied to the shared scheduler.
         */
        soul::async::Task<void> task;
        /**
         * @brief Dependency token that unlocks the task once awaited or waited on.
         */
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
     * @param delay Duration to wait before the task becomes eligible for execution.
     * @param name Identifier that allows other tasks to depend on this work item.
     * @param task Coroutine to execute once the delay and dependencies resolve.
     * @param dependencies Optional tokens that gate execution until upstream tasks finish.
     */
    [[nodiscard]] TaskHandle schedule_after(std::chrono::nanoseconds delay,
                                            std::string name,
                                            soul::async::Task<void> task,
                                            std::span<const soul::async::TaskToken> dependencies = {});

    /**
     * @brief Blocks until all in-flight frame tasks reach completion.
     * @note Useful during shutdown or validation scenarios where deterministic completion is
     *       required before proceeding.
     */
    void wait_for_all();

private:
    std::shared_ptr<soul::async::TaskScheduler> m_scheduler;
    std::unordered_map<std::string, soul::async::TaskToken> m_tokens;
};

} // namespace soul::time
