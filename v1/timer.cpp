#include "timer.h"

Timer::Timer(EngineObject* parent)
 : EngineObject{parent, "Timer"}
{

}

// initialize start values
void Timer::init()
{
    m_startTime = static_cast<float>(glfwGetTime());
    m_lastDelta = 0.0f;
    m_time = m_startTime;
}

// update time stamp
void Timer::update()
{
    m_time = static_cast<float>(glfwGetTime());
}

void Timer::reset()
{
    m_startTime = static_cast<float>(glfwGetTime());
    m_lastDelta = 0.0f;
    m_time = m_startTime;
}

float Timer::getTime()
{
    m_lastDelta = m_time - m_startTime;
    return m_lastDelta;
}
