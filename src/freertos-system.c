#include <system/system.h>

#include <FreeRTOS/FreeRTOS.h>
#include <FreeRTOS/task.h>

#include <stdbool.h>

void
system_init()
{
    __no_operation();
}

bool
system_start(void)
{
	vTaskStartScheduler();
	return true;
}

void
system_delay(system_tick_t ticks)
{
    vTaskDelay(ticks);
}
