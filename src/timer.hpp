#ifndef TIMER_H
#define TIMER_H

class Timer
{
public:
    Timer();
    ~Timer() = default;

    // update time delta
    void update();
    // reset start time
    void reset();

    // gets the time in seconds as an int
    [[nodiscard]] int getTime() const;
    // gets the time in seconds as a float
    [[nodiscard]] float getTimeF() const;

    // get the start time of the timer
    [[nodiscard]] float getRawStartTime() const { return m_startTime; }
    // get the last time of the timer
    [[nodiscard]] float getRawLastTime() const { return m_lastTime; }

private:
    // start time (time since glfw initialized in seconds)
    float m_startTime{};
    // last time recorded
    float m_lastTime{};
};

#endif
