#ifndef EVENTSCHEDULER_H
#define EVENTSCHEDULER_H

#include "TimedEvent.h"
#include <queue>
#include <vector>
#include <functional>

class EventScheduler {
private:
    struct Compare {
        bool operator()(const TimedEvent& a, const TimedEvent& b) {
            if (a.getPriority() == b.getPriority()) {
                return a.shouldTrigger(0) > b.shouldTrigger(0); // Compare by trigger time
            }
            return a.getPriority() < b.getPriority(); // Compare by priority
        }
    };

    std::priority_queue<TimedEvent, std::vector<TimedEvent>, Compare> eventQueue;

public:
    void scheduleEvent(const TimedEvent& event);
    void update(uint64_t currentTime);
    std::vector<TimedEvent> getPendingEvents() const;
    void cancelEvent(const std::string& id);
    void rescheduleEvent(const std::string& id, uint64_t newTime);
};

#endif // EVENTSCHEDULER_H