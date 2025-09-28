#include "time/TimedEvent.h"

TimedEvent::TimedEvent(uint64_t trigger, std::function<void()> cb, int prio, const std::string& eventId, uint64_t repeat)
    : triggerTime(trigger), callback(std::move(cb)), priority(prio), id(eventId), repeatInterval(repeat) {}

bool TimedEvent::shouldTrigger(uint64_t currentTime) const {
    return currentTime >= triggerTime;
}

void TimedEvent::execute() const {
    if (callback) {
        callback();
    }
    if (repeatInterval > 0) {
        triggerTime += repeatInterval;
    }
}

void TimedEvent::reschedule(uint64_t newTime) {
    triggerTime = newTime;
}

int TimedEvent::getPriority() const {
    return priority;
}

const std::string& TimedEvent::getId() const {
    return id;
}