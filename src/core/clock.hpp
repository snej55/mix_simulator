#ifndef CLOCK_H
#define CLOCK_H

#include "engine_types.hpp"

class Clock final : public EngineObject
{
public:
    explicit Clock(EngineObject* engine);

    // update delta time and last time
    void update();
    [[nodiscard]] float getDeltaTime() const;
    [[nodiscard]] float getTime();

private:
    float m_deltaTime{1.0f};
    float m_lastTime{0.0f};

    // gets updated in Clock::update() and Clock::getTime()
    float m_time{0.0f};
};

#endif
