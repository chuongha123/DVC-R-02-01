#pragma once
#include <Arduino.h>
#include <functional>

class TimerService
{
public:
    using Callback = std::function<void()>;

    enum TimerType
    {
        REPEAT,
        ONE_SHOT
    };

    struct TimerItem
    {
        bool active = false;
        TimerType type = REPEAT;
        uint32_t interval = 0;
        uint32_t lastRun = 0;
        Callback cb = nullptr;
        uint16_t id = 0;
    };

    // API chính
    static uint16_t setInterval(uint32_t ms, Callback cb);
    static uint16_t setTimeout(uint32_t ms, Callback cb);
    static void cancel(uint16_t id);
    static void pause(uint16_t id);
    static void resume(uint16_t id);
    static void tick();

private:
    static const int MAX_TIMERS = 16;
    static TimerItem timers[MAX_TIMERS];
    static uint16_t _nextId;

    static uint16_t addTimer(uint32_t ms, Callback cb, TimerType type);
};
