#include "timer.hpp"

// for glfwGetTime()
#include <GLFW/glfw3.h>

Timer::Timer()
{
    // initialize start time and last time
    reset();
}

// update last time
void Timer::update() { m_lastTime = static_cast<float>(glfwGetTime()); }

// reset start time and last time
void Timer::reset()
{
    m_startTime = static_cast<float>(glfwGetTime());
    m_lastTime = m_startTime;
}

// get time as int
int Timer::getTime() const { return static_cast<int>(m_lastTime - m_startTime); }

float Timer::getTimeF() const { return m_lastTime - m_startTime; }
