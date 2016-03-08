#include <system/system.h>
#include <system/smphr.h>
#include <system/task.h>

#include <stddef.h>
#include <stdio.h>

smphr_t s;

void task(void* arg)
{
    uintptr_t id = (uintptr_t)arg;
    smphr_take(&s, 1000);

    printf("Task %d has the semaphore\n", (unsigned int)id);
    system_delay(500);
    printf("Task %d gives the semaphore\n", (unsigned int)id);
    smphr_give(&s);
}

int main(void)
{
    task_t task1h;
    task_t task2h;

    system_init();

    printf("System abstraction test - POSIX implementation\n");
    printf("Semaphore example application\n");

    smphr_init(&s, 1, NULL);

    task_create(&task1h, task, (void*)1, NULL, 0, 0);
    task_create(&task2h, task, (void*)2, NULL, 0, 0);

    system_start();
    smphr_destroy(&s);

    return 0;
}
