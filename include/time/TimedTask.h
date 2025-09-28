#ifndef TIMEDTASK_H
#define TIMEDTASK_H

#include "ITimeTrackable.h"
#include <cstdint>
#include <string>
#include <ostream>

class TimedTask : public ITimeTrackable {
public:
enum class Priority {
    LOW,
    MEDIUM,
    HIGH
};

friend std::ostream& operator<<(std::ostream& os, const Priority& priority) {
    switch (priority) {
        case Priority::LOW: os << "LOW"; break;
        case Priority::MEDIUM: os << "MEDIUM"; break;
        case Priority::HIGH: os << "HIGH"; break;
    }
    return os;
}

private:
    uint64_t startTime;
    uint64_t endTime;
    Priority priority;
    std::string id;

public:
    TimedTask(uint64_t start, uint64_t end, Priority prio, const std::string& taskId);

    uint64_t getStartTime() const override;
    uint64_t getEndTime() const override;
    bool isComplete(uint64_t currentTime) const override;
    uint64_t getRemainingTime(uint64_t currentTime) const override;

    void onComplete();
    void tick(uint64_t currentTime);
    void restart(uint64_t newDuration);
    void extend(uint64_t duration);
    void forceComplete();
    Priority getPriority() const;
    std::string getId() const;
};

#endif // TIMEDTASK_H