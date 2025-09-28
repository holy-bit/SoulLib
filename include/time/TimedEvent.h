#ifndef TIMEDEVENT_H
#define TIMEDEVENT_H

#include <cstdint>
#include <functional>
#include <string>

class TimedEvent {
private:
    mutable uint64_t triggerTime;
    std::function<void()> callback;
    uint64_t repeatInterval;
    int priority;
    std::string id;

public:
    TimedEvent(uint64_t trigger, std::function<void()> cb, int prio, const std::string& eventId, uint64_t repeat = 0);

    bool shouldTrigger(uint64_t currentTime) const;
    void execute() const;
    void reschedule(uint64_t newTime);
    int getPriority() const;
    const std::string& getId() const;
};

#endif // TIMEDEVENT_H