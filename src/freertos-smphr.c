#include <system/system.h>
#include <system/smphr.h>

#include <FreeRTOS/FreeRTOS.h>
#include <FreeRTOS/task.h>
#include <FreeRTOS/semphr.h>

#include <stdbool.h>

bool
smphr_init(smphr_t *s, bool taken, const char *name)
{
    xSemaphoreHandle handle;

    vSemaphoreCreateBinary(handle);
    if (handle == NULL) {
        return false;
    }
    if (taken) {
        if (xSemaphoreTake(handle, 0) != pdTRUE) {
            return false;
        }
    }
    if (name != NULL) {
        vQueueAddToRegistry(handle, (signed char *)name);
    }
    *s = (smphr_t)handle;

    return true;
}

bool
smphr_take(smphr_t s, system_tick_t ticks)
{
    return xSemaphoreTake(s, ticks) == pdTRUE;
}

bool
smphr_give(smphr_t s)
{
    return xSemaphoreGive(s) == pdTRUE;
}

bool
smphr_isr_give(smphr_t s, bool * const woken)
{
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
    bool result;

    result = (xSemaphoreGiveFromISR(s, &xHigherPriorityTaskWoken) == pdTRUE);
    if (result) {
        *woken = (xHigherPriorityTaskWoken == pdTRUE);
    }

    return result;
}
