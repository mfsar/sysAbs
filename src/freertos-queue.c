#include <system/system.h>
#include <system/queue.h>

#include <FreeRTOS/FreeRTOS.h>
#include <FreeRTOS/queue.h>

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

bool
queue_create(queue_t *q, size_t size, size_t elsize, const char *name)
{
    xQueueHandle handle;

    handle = xQueueCreate(size, elsize);
    if (name != NULL) {
        vQueueAddToRegistry(handle, (signed char *)name);
    }
    *q = (queue_t)handle;

    return handle != NULL;
}

size_t
queue_elements_count(queue_t q)
{
    return uxQueueMessagesWaiting(q);
}

bool
queue_push(queue_t q, void *el, system_tick_t ticks)
{
    return xQueueSendToBack(q, el, ticks) == pdTRUE;
}

bool
queue_push_to_front(queue_t q, void *el, system_tick_t ticks)
{

    return xQueueSendToFront(q, el, ticks) == pdTRUE;
}

bool
queue_isr_push(queue_t q, void *el, bool * const woken)
{
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
    bool result;

    result = (xQueueSendToBackFromISR(q, el,
                                      &xHigherPriorityTaskWoken) == pdTRUE);
    if (result) {
        *woken = (xHigherPriorityTaskWoken == pdTRUE);
    }

    return result;
}

bool
queue_pop(queue_t q, void *el, system_tick_t ticks)
{
    return xQueueReceive(q, el, ticks) == pdTRUE;
}
