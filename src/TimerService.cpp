#include "TimerService.h"

TimerService::TimerItem TimerService::timers[MAX_TIMERS];
uint16_t TimerService::_nextId = 1;

uint16_t TimerService::addTimer(uint32_t ms, Callback cb, TimerType type)
{
    for (int i = 0; i < MAX_TIMERS; i++)
    {
        if (!timers[i].active)
        {
            timers[i].active = true;
            timers[i].type = type;
            timers[i].interval = ms;
            timers[i].lastRun = millis();
            timers[i].cb = cb;
            timers[i].id = _nextId++;

            if (_nextId == 0)
                _nextId = 1; // tránh tràn ID về 0

            return timers[i].id;
        }
    }
    return 0; // Hết slot
}

uint16_t TimerService::setInterval(uint32_t ms, Callback cb)
{
    return addTimer(ms, cb, REPEAT);
}

uint16_t TimerService::setTimeout(uint32_t ms, Callback cb)
{
    return addTimer(ms, cb, ONE_SHOT);
}

void TimerService::cancel(uint16_t id)
{
    for (auto &t : timers)
    {
        if (t.active && t.id == id)
        {
            t.active = false;
            t.cb = nullptr;
        }
    }
}

void TimerService::pause(uint16_t id)
{
    for (auto &t : timers)
    {
        if (t.active && t.id == id)
        {
            t.active = false;
        }
    }
}

void TimerService::resume(uint16_t id)
{
    for (auto &t : timers)
    {
        if (!t.active && t.id == id)
        {
            t.active = true;
            t.lastRun = millis();
        }
    }
}

void TimerService::tick()
{
    uint32_t now = millis();

    for (auto &t : timers)
    {
        if (!t.active || t.cb == nullptr)
            continue;

        if (now - t.lastRun >= t.interval)
        {
            t.lastRun = now;
            t.cb(); // Gọi callback

            if (t.type == ONE_SHOT)
            {
                t.active = false;
                t.cb = nullptr;
            }
        }
    }
}
