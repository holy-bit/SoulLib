#include <gtest/gtest.h>

#include <array>
#include <atomic>
#include <chrono>
#include <future>
#include <iostream>
#include <mutex>
#include <vector>

#include "Async/Task.h"
#include "time/FrameScheduler.h"

namespace {

soul::async::Task<void> MakePushTask(std::vector<int>& buffer, std::mutex& guard, int value) {
    {
        std::lock_guard lock(guard);
        buffer.push_back(value);
    }
    co_return;
}

}

TEST(FrameScheduler, ExecutesDependenciesInTopologicalOrder) {
    auto scheduler = std::make_shared<soul::async::TaskScheduler>(2);
    soul::time::FrameScheduler frameScheduler(scheduler);

    std::vector<int> order;
    std::mutex orderGuard;

    std::promise<void> completion;
    auto completionFuture = completion.get_future();

    auto root = frameScheduler.schedule("root", MakePushTask(order, orderGuard, 1));
    const std::array<soul::async::TaskToken, 1> childDeps{root.token};
    auto childA = frameScheduler.schedule("childA", MakePushTask(order, orderGuard, 2), childDeps);
    const std::array<soul::async::TaskToken, 1> grandChildDeps{childA.token};
    auto childB = frameScheduler.schedule("childB",
                                          [&]() -> soul::async::Task<void> {
                                              {
                                                  std::lock_guard lock(orderGuard);
                                                  order.push_back(3);
                                              }
                                              completion.set_value();
                                              co_return;
                                          }(),
                                          grandChildDeps);

    const auto status = completionFuture.wait_for(std::chrono::milliseconds(100));
    ASSERT_EQ(status, std::future_status::ready)
        << "FrameScheduler did not finish the dependency chain within 100ms";
    if (status == std::future_status::ready) {
        frameScheduler.wait_for_all();
    }

    ASSERT_EQ(order.size(), 3u);
    EXPECT_EQ(order[0], 1);
    EXPECT_EQ(order[1], 2);
    EXPECT_EQ(order[2], 3);
}

TEST(FrameScheduler, DelayedTasksRespectDelayOrdering) {
    auto scheduler = std::make_shared<soul::async::TaskScheduler>(1);
    soul::time::FrameScheduler frameScheduler(scheduler);

    std::atomic<bool> firstRan{false};
    std::atomic<bool> observedOrder{false};
    std::promise<void> completion;
    auto completionFuture = completion.get_future();

    auto first = frameScheduler.schedule("immediate", [&]() -> soul::async::Task<void> {
        firstRan.store(true, std::memory_order_release);
        co_return;
    }());

    const std::array<soul::async::TaskToken, 1> dependencyTokens{first.token};
    auto delayed = frameScheduler.schedule_after(std::chrono::milliseconds(10),
                                                 "delayed",
                                                 [&]() -> soul::async::Task<void> {
                                                     observedOrder.store(firstRan.load(std::memory_order_acquire),
                                                                         std::memory_order_release);
                                                     completion.set_value();
                                                     co_return;
                                                 }(),
                                                 dependencyTokens);

    const auto delayedStatus = completionFuture.wait_for(std::chrono::milliseconds(250));
    ASSERT_EQ(delayedStatus, std::future_status::ready)
        << "FrameScheduler delayed task did not complete within 250ms";
    if (delayedStatus == std::future_status::ready) {
        frameScheduler.wait_for_all();
    }

    EXPECT_TRUE(firstRan.load(std::memory_order_acquire));
    EXPECT_TRUE(observedOrder.load(std::memory_order_acquire));
}
