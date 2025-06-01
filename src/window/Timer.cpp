//
// Created by Yibuz Pokopodrozo on 2025-05-12.
//

#include <window/Timer.h>

Timer::Timer() {
    prevTicks = 0;
    currTicks = 0;
}

Timer::~Timer() = default;

void Timer::UpdateFrameTicks() {
    prevTicks = currTicks;
    currTicks = static_cast<unsigned int>(glfwGetTime() * 1000);
}

void Timer::Start() {
    prevTicks = static_cast<unsigned int>(glfwGetTime() * 1000);
    currTicks = static_cast<unsigned int>(glfwGetTime() * 1000);
}

float Timer::GetDeltaTime() const {
    return static_cast<float>(currTicks - prevTicks) / 1000.0f;
}

unsigned int Timer::GetSleepTime(const unsigned int fps) const {
    const unsigned int milliSecsPerFrame = 1000 / fps;
    if (milliSecsPerFrame == 0) {
        return 0;
    }

    const unsigned int sleepTime = milliSecsPerFrame - (static_cast<unsigned int>(glfwGetTime() * 1000) - currTicks);
    if (sleepTime > milliSecsPerFrame) {
        return milliSecsPerFrame;
    }

    return sleepTime;
}

float Timer::GetCurrentTicks() const {
    return static_cast<float>(currTicks) / 1000.0f;
}
