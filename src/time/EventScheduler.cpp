#include "time/EventScheduler.h"
#include <algorithm>

void EventScheduler::scheduleEvent(const TimedEvent& event) {
    eventQueue.push(event);
}

void EventScheduler::update(uint64_t currentTime) {
    while (!eventQueue.empty() && eventQueue.top().shouldTrigger(currentTime)) {
        eventQueue.top().execute();
        eventQueue.pop();
    }
}

std::vector<TimedEvent> EventScheduler::getPendingEvents() const {
    std::vector<TimedEvent> pendingEvents;
    auto tempQueue = eventQueue;
    while (!tempQueue.empty()) {
        pendingEvents.push_back(tempQueue.top());
        tempQueue.pop();
    }
    return pendingEvents;
}

void EventScheduler::cancelEvent(const std::string& id) {
    std::priority_queue<TimedEvent, std::vector<TimedEvent>, Compare> newQueue;
    while (!eventQueue.empty()) {
        if (eventQueue.top().getId() != id) {
            newQueue.push(eventQueue.top());
        }
        eventQueue.pop();
    }
    eventQueue = std::move(newQueue);
}

void EventScheduler::rescheduleEvent(const std::string& id, uint64_t newTime) {
    std::priority_queue<TimedEvent, std::vector<TimedEvent>, Compare> newQueue;
    while (!eventQueue.empty()) {
        if (eventQueue.top().getId() == id) {
            TimedEvent updatedEvent = eventQueue.top();
            updatedEvent.reschedule(newTime);
            newQueue.push(updatedEvent);
        } else {
            newQueue.push(eventQueue.top());
        }
        eventQueue.pop();
    }
    eventQueue = std::move(newQueue);
}