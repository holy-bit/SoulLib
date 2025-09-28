#include "time/TimerManager.h"

void TimerManager::addTask(const std::shared_ptr<TimedTask>& task) {
    activeTasks.push_back(task);
}

void TimerManager::updateAll(uint64_t currentTime) {
    for (auto& task : activeTasks) {
        task->tick(currentTime);
    }
}

std::vector<std::shared_ptr<TimedTask>> TimerManager::getActiveTasks() const {
    return activeTasks;
}

std::vector<std::shared_ptr<TimedTask>> TimerManager::getCompletedTasks(uint64_t currentTime) const {
    std::vector<std::shared_ptr<TimedTask>> completedTasks;
    for (const auto& task : activeTasks) {
        if (task->isComplete(currentTime)) {
            completedTasks.push_back(task);
        }
    }
    return completedTasks;
}

void TimerManager::removeCompletedTasks(uint64_t currentTime) {
    activeTasks.erase(
        std::remove_if(activeTasks.begin(), activeTasks.end(), [currentTime](const std::shared_ptr<TimedTask>& task) {
            return task->isComplete(currentTime);
        }),
        activeTasks.end());
}

std::shared_ptr<TimedTask> TimerManager::findTaskById(const std::string& id) const {
    auto it = std::find_if(activeTasks.begin(), activeTasks.end(), [&id](const std::shared_ptr<TimedTask>& task) {
        return task->getId() == id;
    });
    return (it != activeTasks.end()) ? *it : nullptr;
}

std::shared_ptr<TimedTask> TimerManager::getNextToComplete(uint64_t currentTime) const {
    std::shared_ptr<TimedTask> nextTask = nullptr;
    uint64_t minTime = UINT64_MAX;
    for (const auto& task : activeTasks) {
        uint64_t remainingTime = task->getRemainingTime(currentTime);
        if (remainingTime < minTime) {
            minTime = remainingTime;
            nextTask = task;
        }
    }
    return nextTask;
}