#include "clock.hpp"

#include <GLFW/glfw3.h>

#include <algorithm>

Clock::Clock(EngineObject* engine) : EngineObject{"Clock", engine}
{
    // initialize delta time and starting time
    update();
}

void Clock::update()
{
    // recalculate delta time
    const float time{static_cast<float>(glfwGetTime())};
    m_deltaTime = (time - m_lastTime) * 60.f;
    m_lastTime = time;

    // update time
    m_time = static_cast<float>(glfwGetTime());
}

float Clock::getDeltaTime() const { return std::clamp(m_deltaTime, 0.1f, 4.f); }

float Clock::getTime()
{
    // update time
    m_time = static_cast<float>(glfwGetTime());
    return m_time;
}
