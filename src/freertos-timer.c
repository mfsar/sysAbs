#include <system/system.h>
#include <system/timer.h>

#include <FreeRTOS/FreeRTOS.h>
#include <FreeRTOS/timers.h>

#include <stdbool.h>

bool
tmr_create(tmr_t *t, bool autoreload, timer_callback_t callback,
             const char *name)
{
    xTimerHandle handle;

    handle = xTimerCreate((signed char *)name, 1, autoreload, t, callback);
    if (handle == NULL) {
        return (false);
    }

    t->handle = handle;
    t->period = 0;

    return (true);
}

bool
tmr_start(tmr_t *t, system_tick_t period)
{

    if (period != t->period) {
        if (xTimerChangePeriod(t->handle, period, SYSTEM_NO_WAIT) != pdPASS) {
            return (false);
        }
        t->period = period;
    }

    return (xTimerStart(t->handle, SYSTEM_NO_WAIT) == pdPASS);
}

bool
tmr_reset(tmr_t *t)
{

    if (t->period == 0) {
        return (false);
    }

    return (xTimerReset(t->handle, SYSTEM_NO_WAIT) == pdPASS);
}

bool
tmr_stop(tmr_t *t)
{

    return (xTimerStop(t->handle, SYSTEM_NO_WAIT) == pdPASS);
}

bool
tmr_is_running(tmr_t *t)
{

    return (xTimerIsTimerActive(t->handle) != pdFALSE);
}
