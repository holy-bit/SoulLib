#include "time/TimedTask.h"

TimedTask::TimedTask(uint64_t start, uint64_t end, Priority prio, const std::string& taskId)
    : startTime(start), endTime(end), priority(prio), id(taskId) {}

uint64_t TimedTask::getStartTime() const {
    return startTime;
}

uint64_t TimedTask::getEndTime() const {
    return endTime;
}

bool TimedTask::isComplete(uint64_t currentTime) const {
    return currentTime >= endTime;
}

uint64_t TimedTask::getRemainingTime(uint64_t currentTime) const {
    return (currentTime >= endTime) ? 0 : (endTime - currentTime);
}

void TimedTask::onComplete() {
    // Custom completion logic
}

void TimedTask::tick(uint64_t currentTime) {
    // Custom tick logic
}

void TimedTask::restart(uint64_t newDuration) {
    endTime = startTime + newDuration;
}

void TimedTask::extend(uint64_t duration) {
    endTime += duration;
}

void TimedTask::forceComplete() {
    endTime = 0;
}

TimedTask::Priority TimedTask::getPriority() const {
    return priority;
}

std::string TimedTask::getId() const {
    return id;
}