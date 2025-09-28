#include "Async/Task.h"

#include <algorithm>
#include <thread>

namespace soul::async {

class TaskScheduler::Worker {
public:
    explicit Worker(TaskScheduler& owner)
        : m_owner(owner),
          m_thread([this]() { run(); }) {}

    Worker(const Worker&) = delete;
    Worker& operator=(const Worker&) = delete;

    ~Worker() {
        if (m_thread.joinable()) {
            m_thread.join();
        }
    }

private:
    void run() {
        while (m_owner.m_running.load(std::memory_order_acquire)) {
            std::function<void()> job;
            {
                std::unique_lock lock(m_owner.m_queueMutex);
                m_owner.m_queueCv.wait(lock, [&]() {
                    return !m_owner.m_running.load(std::memory_order_acquire) || !m_owner.m_jobs.empty();
                });

                if (!m_owner.m_running.load(std::memory_order_acquire) && m_owner.m_jobs.empty()) {
                    return;
                }

                job = std::move(m_owner.m_jobs.front());
                m_owner.m_jobs.pop_front();
            }

            if (job) {
                job();
            }
        }
    }

    TaskScheduler& m_owner;
    std::thread m_thread;
};

TaskScheduler::TaskScheduler(std::size_t workerCount) {
    if (workerCount == 0) {
        workerCount = std::max<std::size_t>(1, std::thread::hardware_concurrency());
    }
    m_workers.reserve(workerCount);
    for (std::size_t i = 0; i < workerCount; ++i) {
        m_workers.emplace_back(std::make_unique<Worker>(*this));
    }
}

TaskScheduler::~TaskScheduler() {
    stop();
}

void TaskScheduler::wait(const TaskToken& token) {
    if (!token.valid()) {
        return;
    }
    token.state()->wait();
}

void TaskScheduler::run() {
    m_running.store(true, std::memory_order_release);
}

void TaskScheduler::stop() {
    bool expected = true;
    if (m_running.compare_exchange_strong(expected, false, std::memory_order_acq_rel)) {
        {
            std::lock_guard lock(m_queueMutex);
        }
        m_queueCv.notify_all();
    } else {
        m_running.store(false, std::memory_order_release);
        m_queueCv.notify_all();
    }
    for (auto& worker : m_workers) {
        worker.reset();
    }
}

void TaskScheduler::enqueue(std::function<void()> job) {
    {
        std::lock_guard lock(m_queueMutex);
        m_jobs.emplace_back(std::move(job));
    }
    m_queueCv.notify_one();
}

void TaskScheduler::schedule_state(const std::shared_ptr<detail::TaskStateBase>& state) {
    enqueue([state]() {
        if (state->coroutine && !state->coroutine.done()) {
            state->coroutine.resume();
        }
    });
}

void TaskScheduler::on_task_finished(const std::shared_ptr<detail::TaskStateBase>& state) {
    std::vector<std::shared_ptr<detail::TaskStateBase>> ready;
    {
        std::lock_guard lock(state->continuationMutex);
        for (auto it = state->dependents.begin(); it != state->dependents.end(); ++it) {
            if (auto dependent = it->lock()) {
                if (dependent->pendingDependencies.fetch_sub(1, std::memory_order_acq_rel) == 1) {
                    ready.emplace_back(std::move(dependent));
                }
            }
        }
        state->dependents.clear();
    }

    for (auto& dep : ready) {
        schedule_state(dep);
    }
}

void TaskScheduler::resume_coroutine(std::coroutine_handle<> handle) {
    enqueue([handle]() mutable {
        if (handle && !handle.done()) {
            handle.resume();
        }
    });
}

} // namespace soul::async
