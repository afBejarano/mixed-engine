//
// Created by Yibuz Pokopodrozo on 2025-05-12.
//

#ifndef TIMER_H
#define TIMER_H

class Timer {
public:
    Timer();
    ~Timer();

    Timer(const Timer&) = delete;
    Timer(Timer&&) = delete;
    Timer& operator=(const Timer&) = delete;
    Timer& operator=(Timer&&) = delete;

    void Start();
    void UpdateFrameTicks();
    [[nodiscard]] float GetDeltaTime() const;
    [[nodiscard]] unsigned int GetSleepTime(unsigned int fps) const;
    [[nodiscard]] float GetCurrentTicks() const;
private:
    unsigned int prevTicks;
    unsigned int currTicks;
};

#endif

