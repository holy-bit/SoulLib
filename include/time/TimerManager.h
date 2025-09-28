#ifndef TIMERMANAGER_H
#define TIMERMANAGER_H

#include "TimedTask.h"
#include <vector>
#include <memory>
#include <algorithm>

class TimerManager {
private:
    std::vector<std::shared_ptr<TimedTask>> activeTasks;

public:
    void addTask(const std::shared_ptr<TimedTask>& task);
    void updateAll(uint64_t currentTime);
    std::vector<std::shared_ptr<TimedTask>> getActiveTasks() const;
    std::vector<std::shared_ptr<TimedTask>> getCompletedTasks(uint64_t currentTime) const;
    void removeCompletedTasks(uint64_t currentTime);
    std::shared_ptr<TimedTask> findTaskById(const std::string& id) const;
    std::shared_ptr<TimedTask> getNextToComplete(uint64_t currentTime) const;
};

#endif // TIMERMANAGER_H