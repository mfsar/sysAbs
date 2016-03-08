#include <system/base.h>
#include "posix-system.h"

#include <time.h>
#include <stddef.h>

void
system_delay(system_tick_t ticks)
{
    struct timespec delay;

    system_ticks_to_timespec(ticks, &delay);
    nanosleep(&delay, NULL);
}
