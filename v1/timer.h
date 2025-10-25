#ifndef TIMER_H
#define TIMER_H

#include <GLFW/glfw3.h>
#include "engine.h"

class Timer final : public EngineObject
{
public:
    explicit Timer(EngineObject* parent);
    // no need for free, EngineObject::free() is enough
    ~Timer() = default;

    // initialize start values
    void init();

    // update time stamp
    void update();

    // resets start time
    void reset();
    // recalulates last delta and returns time since last start time
    float getTime();

    // raw values
    // returns when last time delta was calculated
    [[nodiscard]] float getRawStartTime() const {return m_startTime;}
    // returns last time calculated
    [[nodiscard]] float getRawLastDelta() const {return m_lastDelta;}
    // returns time since timer was created
    [[nodiscard]] float getRawTime() const {return m_time;}

private:
    float m_startTime{0.0f};
    float m_lastDelta{0.0f};

    // keeps track of time since timer was created
    float m_time{0.0f};
};

#endif // TIMER_H
