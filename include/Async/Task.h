#pragma once

#include <atomic>
#include <condition_variable>
#include <coroutine>
#include <deque>
#include <exception>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <span>
#include <type_traits>
#include <utility>
#include <vector>

namespace soul::async {

class TaskScheduler;

namespace detail {

struct TaskStateBase;
using TaskStateBasePtr = std::shared_ptr<TaskStateBase>;

struct FinalAwaiter;

template <typename T>
struct TaskState;

template <typename T>
struct TaskPromise;

struct TaskPromiseVoid;

} // namespace detail

/**
 * @brief Opaque handle representing a scheduled coroutine and its completion state.
 * @details Tokens allow callers to express explicit dependencies between tasks without exposing
 *          implementation details of the underlying coroutine handle. They can be stored, copied,
 *          and later passed back into the scheduler or frame orchestration utilities.
 */
class TaskToken {
public:
    TaskToken() = default;

    explicit TaskToken(const std::shared_ptr<detail::TaskStateBase>& state) noexcept;

    /**
     * @brief Checks if the token references an active task state.
     */
    [[nodiscard]] bool valid() const noexcept;

    /**
     * @brief Exposes the underlying state for advanced instrumentation hooks.
     */
    [[nodiscard]] const std::shared_ptr<detail::TaskStateBase>& state() const noexcept;

private:
    std::shared_ptr<detail::TaskStateBase> m_state;
};

template <typename T = void>
class Task;

/**
 * @brief Multi-threaded coroutine scheduler that owns a worker pool and dependency graph.
 * @details Tasks scheduled here can express explicit dependency chains via `TaskToken`s. The
 *          scheduler ensures continuations resume on the pool threads and exposes blocking APIs for
 *          shutdown or synchronous waiting in tests and tooling.
 */
class TaskScheduler {
public:
    explicit TaskScheduler(std::size_t workerCount = 0);
    ~TaskScheduler();

    TaskScheduler(const TaskScheduler&) = delete;
    TaskScheduler& operator=(const TaskScheduler&) = delete;

    TaskScheduler(TaskScheduler&&) = delete;
    TaskScheduler& operator=(TaskScheduler&&) = delete;

    /**
     * @brief Blocks the calling thread until the given token resolves or throws.
     * @param token Handle previously returned by `schedule` or `run_async`.
     */
    void wait(const TaskToken& token);

    /**
     * @brief Starts the worker threads and processes queued jobs until `stop` is invoked.
     */
    void run();

    /**
     * @brief Requests cooperative shutdown of all workers and wakes any waiting threads.
     */
    void stop();

    /**
     * @brief Registers a coroutine with optional dependencies and returns a tracked task.
     * @tparam T Result type produced by the coroutine.
     * @param task Coroutine ready to execute (typically created via `co_return`).
     * @param dependencies Optional list of tokens that must complete before this task begins.
     * @return Task wrapper re-bound to this scheduler instance.
     */
    template <typename T>
    Task<T> schedule(Task<T>&& task, std::span<const TaskToken> dependencies = {});

    /**
     * @brief Executes a callable on the worker pool and returns an awaitable task.
     * @tparam Func Callable type (lambda, function, functor) executed on a worker thread.
     * @tparam Result Inferred return type of the callable.
     * @param func Callable to run asynchronously.
     * @return Task that resolves when the callable finishes or throws.
     */
    template <typename Func,
              typename Result = std::invoke_result_t<std::decay_t<Func>>>
    Task<Result> run_async(Func&& func);

    /**
     * @brief Resumes a coroutine on the scheduler, typically used by continuations.
     */
    void resume_coroutine(std::coroutine_handle<> handle);

private:
    friend struct detail::TaskStateBase;
    friend struct detail::TaskPromiseVoid;

    class Worker;

    void enqueue(std::function<void()> job);
    void schedule_state(const std::shared_ptr<detail::TaskStateBase>& state);
    void on_task_finished(const std::shared_ptr<detail::TaskStateBase>& state);

    std::vector<std::unique_ptr<Worker>> m_workers;
    std::mutex m_queueMutex;
    std::condition_variable m_queueCv;
    std::deque<std::function<void()>> m_jobs;
    std::atomic_bool m_running{true};
};

namespace detail {

struct TaskStateBase : std::enable_shared_from_this<TaskStateBase> {
    TaskStateBase() = default;
    virtual ~TaskStateBase() = default;

    std::mutex continuationMutex;
    std::vector<std::coroutine_handle<>> continuations;
    std::condition_variable completionCv;
    bool completed{false};
    TaskScheduler* scheduler{nullptr};
    std::atomic_uint32_t pendingDependencies{0};
    std::vector<std::weak_ptr<TaskStateBase>> dependents;
    std::coroutine_handle<> coroutine{};
    std::exception_ptr exception;

    void add_continuation(std::coroutine_handle<> handle);
    void resume_continuations();
    void wait();
    void on_completed();
};

template <typename T>
struct TaskState : TaskStateBase {
    std::optional<T> result;

    T extract();
};

template <>
struct TaskState<void> : TaskStateBase {
    void extract();
};

struct FinalAwaiter {
    bool await_ready() const noexcept { return false; }

    template <typename Promise>
    void await_suspend(std::coroutine_handle<Promise> handle) noexcept {
        auto state = handle.promise().state;
        state->on_completed();
    }

    void await_resume() const noexcept {}
};

template <typename T>
struct TaskPromise {
    using State = TaskState<T>;
    std::shared_ptr<State> state;

    TaskPromise() : state(std::make_shared<State>()) {}

    Task<T> get_return_object() noexcept;

    std::suspend_always initial_suspend() const noexcept { return {}; }
    FinalAwaiter final_suspend() const noexcept { return {}; }

    template <typename Value>
    void return_value(Value&& value) {
        state->result = std::forward<Value>(value);
    }

    void unhandled_exception() {
        state->exception = std::current_exception();
    }
};

struct TaskPromiseVoid {
    std::shared_ptr<TaskState<void>> state;

    TaskPromiseVoid() : state(std::make_shared<TaskState<void>>()) {}

    Task<void> get_return_object() noexcept;

    std::suspend_always initial_suspend() const noexcept { return {}; }
    FinalAwaiter final_suspend() const noexcept { return {}; }

    void return_void() noexcept {}

    void unhandled_exception() {
        state->exception = std::current_exception();
    }
};

} // namespace detail

/**
 * @brief Awaitable wrapper produced by SoulLib coroutines.
 * @tparam T Result type returned when the coroutine completes (void allowed).
 * @details Tasks can be awaited (`co_await`), waited via `get()`, or converted to `TaskToken`
 *          handles for dependency tracking. Movement transfers ownership of the state; copying is
 *          deliberately disallowed to avoid double resumption.
 */
template <typename T>
class Task {
public:
    using promise_type = detail::TaskPromise<T>;

    Task() = default;

    explicit Task(std::shared_ptr<detail::TaskState<T>> state) noexcept
        : m_state(std::move(state)) {}

    Task(const Task&) = delete;
    Task& operator=(const Task&) = delete;

    Task(Task&& other) noexcept : m_state(std::move(other.m_state)) {}
    Task& operator=(Task&& other) noexcept {
        if (this != &other) {
            m_state = std::move(other.m_state);
        }
        return *this;
    }

    ~Task() = default;

    /**
     * @brief Indicates whether the coroutine has already finished without suspending.
     */
    bool await_ready() const noexcept {
        return m_state && m_state->completed;
    }

    /**
     * @brief Registers a continuation and resumes the producer coroutine if needed.
     * @param awaiting Awaiter coroutine to resume once the task completes.
     */
    void await_suspend(std::coroutine_handle<> awaiting) const {
        m_state->add_continuation(awaiting);
        auto* baseState = static_cast<detail::TaskStateBase*>(m_state.get());
        if (baseState->coroutine && !baseState->coroutine.done()) {
            if (baseState->scheduler) {
                baseState->scheduler->resume_coroutine(baseState->coroutine);
            } else {
                baseState->coroutine.resume();
            }
        }
    }

    /**
     * @brief Retrieves the final value, rethrowing any stored exception.
     */
    T await_resume() {
        if (m_state->exception) {
            std::rethrow_exception(m_state->exception);
        }
        return m_state->extract();
    }

    /**
     * @brief Synchronously waits for completion and returns the produced value.
     * @note Useful for bridging into legacy synchronous code or unit tests.
     */
    T get() {
        if (m_state && !m_state->completed) {
            auto* baseState = static_cast<detail::TaskStateBase*>(m_state.get());
            if (baseState->coroutine && !baseState->coroutine.done()) {
                if (baseState->scheduler) {
                    baseState->scheduler->resume_coroutine(baseState->coroutine);
                } else {
                    baseState->coroutine.resume();
                }
            }
            if (!baseState->completed) {
                baseState->wait();
            }
        }
        if (m_state->exception) {
            std::rethrow_exception(m_state->exception);
        }
        return m_state->extract();
    }

    /**
     * @brief Creates a dependency token to wire this task into other schedules.
     */
    [[nodiscard]] TaskToken token() const noexcept {
        return TaskToken{m_state};
    }

    /**
     * @brief Exposes the shared state for advanced diagnostics.
     */
    [[nodiscard]] const std::shared_ptr<detail::TaskState<T>>& state() const noexcept {
        return m_state;
    }

private:
    std::shared_ptr<detail::TaskState<T>> m_state;
};

/**
 * @brief `void` specialisation that mirrors the general task semantics without a return value.
 */
template <>
class Task<void> {
public:
    using promise_type = detail::TaskPromiseVoid;

    Task() = default;

    explicit Task(std::shared_ptr<detail::TaskState<void>> state) noexcept
        : m_state(std::move(state)) {}

    Task(const Task&) = delete;
    Task& operator=(const Task&) = delete;

    Task(Task&& other) noexcept : m_state(std::move(other.m_state)) {}
    Task& operator=(Task&& other) noexcept {
        if (this != &other) {
            m_state = std::move(other.m_state);
        }
        return *this;
    }

    ~Task() = default;

    /**
     * @brief Indicates whether the coroutine has already finished without suspending.
     */
    bool await_ready() const noexcept {
        return m_state && m_state->completed;
    }

    /**
     * @brief Registers a continuation and resumes the producer coroutine if needed.
     * @param awaiting Awaiter coroutine to resume once the task completes.
     */
    void await_suspend(std::coroutine_handle<> awaiting) const {
        m_state->add_continuation(awaiting);
        auto* baseState = static_cast<detail::TaskStateBase*>(m_state.get());
        if (baseState->coroutine && !baseState->coroutine.done()) {
            if (baseState->scheduler) {
                baseState->scheduler->resume_coroutine(baseState->coroutine);
            } else {
                baseState->coroutine.resume();
            }
        }
    }

    /**
     * @brief Propagates any stored exception once the coroutine completes.
     */
    void await_resume() {
        if (m_state->exception) {
            std::rethrow_exception(m_state->exception);
        }
        m_state->extract();
    }

    /**
     * @brief Synchronously waits for completion to aid interop with blocking code.
     */
    void get() {
        if (m_state && !m_state->completed) {
            auto* baseState = static_cast<detail::TaskStateBase*>(m_state.get());
            if (baseState->coroutine && !baseState->coroutine.done()) {
                if (baseState->scheduler) {
                    baseState->scheduler->resume_coroutine(baseState->coroutine);
                } else {
                    baseState->coroutine.resume();
                }
            }
            if (!baseState->completed) {
                baseState->wait();
            }
        }
        if (m_state->exception) {
            std::rethrow_exception(m_state->exception);
        }
        m_state->extract();
    }

    /**
     * @brief Creates a dependency token to wire this task into other schedules.
     */
    [[nodiscard]] TaskToken token() const noexcept {
        return TaskToken{m_state};
    }

    /**
     * @brief Exposes the shared state for advanced diagnostics.
     */
    [[nodiscard]] const std::shared_ptr<detail::TaskState<void>>& state() const noexcept {
        return m_state;
    }

private:
    std::shared_ptr<detail::TaskState<void>> m_state;
};

inline TaskToken::TaskToken(const std::shared_ptr<detail::TaskStateBase>& state) noexcept
    : m_state(state) {}

inline bool TaskToken::valid() const noexcept {
    return static_cast<bool>(m_state);
}

inline const std::shared_ptr<detail::TaskStateBase>& TaskToken::state() const noexcept {
    return m_state;
}

template <typename T>
Task<T> detail::TaskPromise<T>::get_return_object() noexcept {
    state->coroutine = std::coroutine_handle<TaskPromise>::from_promise(*this);
    return Task<T>{state};
}

inline Task<void> detail::TaskPromiseVoid::get_return_object() noexcept {
    state->coroutine = std::coroutine_handle<TaskPromiseVoid>::from_promise(*this);
    return Task<void>{state};
}

inline void detail::TaskStateBase::add_continuation(std::coroutine_handle<> handle) {
    bool resumeImmediately = false;
    {
        std::lock_guard lock(continuationMutex);
        if (completed) {
            resumeImmediately = true;
        } else {
            continuations.emplace_back(handle);
        }
    }

    if (resumeImmediately) {
        if (scheduler) {
            scheduler->resume_coroutine(handle);
        } else if (handle) {
            handle.resume();
        }
    }
}

inline void detail::TaskStateBase::resume_continuations() {
    std::vector<std::coroutine_handle<>> pending;
    {
        std::lock_guard lock(continuationMutex);
        pending.swap(continuations);
    }

    for (auto handle : pending) {
        if (scheduler) {
            scheduler->resume_coroutine(handle);
        } else if (handle) {
            handle.resume();
        }
    }
}

inline void detail::TaskStateBase::wait() {
    std::unique_lock lock(continuationMutex);
    completionCv.wait(lock, [this]() { return completed; });
}

inline void detail::TaskStateBase::on_completed() {
    {
        std::lock_guard lock(continuationMutex);
        completed = true;
    }

    completionCv.notify_all();
    resume_continuations();

    if (scheduler) {
        scheduler->on_task_finished(shared_from_this());
    }
}

template <typename T>
T detail::TaskState<T>::extract() {
    if (!result.has_value()) {
        return T{};
    }

    T value = std::move(*result);
    result.reset();
    return value;
}

inline void detail::TaskState<void>::extract() {}

template <typename T>
Task<T> TaskScheduler::schedule(Task<T>&& task, std::span<const TaskToken> dependencies) {
    auto state = task.state();
    if (!state) {
        return Task<T>{};
    }

    state->scheduler = this;

    uint32_t pending = 0;
    for (const auto& token : dependencies) {
        auto dependencyState = token.state();
        if (!dependencyState) {
            continue;
        }

        bool registered = false;
        {
            std::lock_guard lock(dependencyState->continuationMutex);
            if (!dependencyState->completed) {
                dependencyState->dependents.emplace_back(state);
                registered = true;
            }
        }

        if (registered) {
            ++pending;
            state->pendingDependencies.fetch_add(1, std::memory_order_relaxed);
        }
    }

    if (pending == 0) {
        schedule_state(state);
    }

    return Task<T>{std::move(state)};
}

template <typename Func, typename Result>
Task<Result> TaskScheduler::run_async(Func&& func) {
    using FunctionType = std::decay_t<Func>;
    auto state = std::make_shared<detail::TaskState<Result>>();
    state->scheduler = this;

    enqueue([state, job = FunctionType(std::forward<Func>(func))]() mutable {
        try {
            if constexpr (std::is_void_v<Result>) {
                job();
            } else {
                state->result = job();
            }
        } catch (...) {
            state->exception = std::current_exception();
        }

        state->on_completed();
    });

    return Task<Result>{std::move(state)};
}

} // namespace soul::async
