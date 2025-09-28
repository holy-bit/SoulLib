#include "utility.h"
#include "time/TimerManager.h"
#include "time/TimedTask.h"
#include "time/TimeService.h"
#include <iostream>
#include <memory>
#include <thread>
#include <chrono>

int main() {
    TimeService timeService;
    TimerManager timerManager;

    // Use execution time instead of server time
    auto start = std::chrono::steady_clock::now();
    uint64_t serverTime = std::chrono::duration_cast<std::chrono::milliseconds>(start.time_since_epoch()).count();

    // Set the current time using execution time
    timeService.syncWithServerTime(serverTime);

    // Create a timed task
    auto task = std::make_shared<TimedTask>(
        timeService.getCurrentTime(),
        timeService.getCurrentTime() + 5000,
        TimedTask::Priority::HIGH,
        "task1"
    );

    // Add the task to the timer manager
    timerManager.addTask(task);

    // Simulate real-time updates
    uint64_t taskCompletionTime = task->getEndTime();
    while (timeService.getCurrentTime() <= taskCompletionTime+200) {
        // Update real-time in TimeService
        timeService.updateRealTime();

        // Update the timer manager
        timerManager.updateAll(timeService.getCurrentTime());

        // Check completed tasks
        auto completedTasks = timerManager.getCompletedTasks(timeService.getCurrentTime());
        for (const auto& completedTask : completedTasks) {
            auto startTime = completedTask->getStartTime();
            auto endTime = completedTask->getEndTime();
            auto duration = endTime - startTime;

            std::cout << "Task " << completedTask->getPriority() 
                      << " started at " << startTime 
                      << ", completed at " << endTime 
                      << ", duration: " << duration << " ms\n";
        }

        // Remove completed tasks
        timerManager.removeCompletedTasks(timeService.getCurrentTime());

        // Sleep for a short duration to simulate real-time
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return 0;
}