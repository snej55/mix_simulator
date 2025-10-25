#include "clock.h"

Clock::Clock(EngineObject* parent)
 : EngineObject{parent, "Clock"}
{
    Clock::update();
}

void Clock::update()
{
    // recalculate delta time
    const float time{static_cast<float>(glfwGetTime())};
    m_deltaTime = time - m_lastTime;
    m_lastTime = time;

    // update time
    m_time = static_cast<float>(glfwGetTime());
}

float Clock::getDeltaTime() const
{
    return m_deltaTime;
}

float Clock::getTime()
{
    // update time
    m_time = static_cast<float>(glfwGetTime());
    return m_time;
}