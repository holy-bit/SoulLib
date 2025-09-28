#ifndef ITIMETRACKABLE_H
#define ITIMETRACKABLE_H

#include <cstdint>

class ITimeTrackable {
public:
    virtual ~ITimeTrackable() = default;

    virtual uint64_t getStartTime() const = 0;
    virtual uint64_t getEndTime() const = 0;
    virtual bool isComplete(uint64_t currentTime) const = 0;
    virtual uint64_t getRemainingTime(uint64_t currentTime) const = 0;
};

#endif // ITIMETRACKABLE_H