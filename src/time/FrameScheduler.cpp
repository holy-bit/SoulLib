#include "time/FrameScheduler.h"

#include <thread>
#include <vector>

namespace soul::time {

FrameScheduler::FrameScheduler(std::shared_ptr<soul::async::TaskScheduler> scheduler)
    : m_scheduler(std::move(scheduler)) {}

FrameScheduler::TaskHandle FrameScheduler::schedule(
    std::string name,
    soul::async::Task<void> task,
    std::span<const soul::async::TaskToken> dependencies) {
    auto scheduled = m_scheduler->schedule(std::move(task), dependencies);
    auto token = scheduled.token();
    m_tokens.emplace(std::move(name), token);
    return TaskHandle{std::move(scheduled), token};
}

FrameScheduler::TaskHandle FrameScheduler::schedule_after(
    std::chrono::nanoseconds delay,
    std::string name,
    soul::async::Task<void> task,
    std::span<const soul::async::TaskToken> dependencies) {
    auto scheduler = m_scheduler;
    auto delayTask = scheduler->schedule([delay]() mutable -> soul::async::Task<void> {
        std::this_thread::sleep_for(delay);
        co_return;
    }());

    std::vector<soul::async::TaskToken> combinedDeps;
    combinedDeps.reserve(dependencies.size() + 1);
    combinedDeps.insert(combinedDeps.end(), dependencies.begin(), dependencies.end());
    combinedDeps.emplace_back(delayTask.token());

    return schedule(std::move(name), std::move(task), std::span<const soul::async::TaskToken>(combinedDeps.data(), combinedDeps.size()));
}

void FrameScheduler::wait_for_all() {
    for (auto& [name, token] : m_tokens) {
        m_scheduler->wait(token);
    }
}

} // namespace soul::time
