#include <system/system.h>
#include <system/task.h>

#include <stddef.h>
#include <stdio.h>

void test(void* arg)
{
    while(true) {
        system_delay(500);
        printf("test\n");
    }
}

int main(void)
{
    task_t test_task;
    printf("System abstraction test - POSIX implementation\n");

    system_init();
    task_create(&test_task, test, NULL, NULL, 0, 0);
    system_start();

    return 0;
}
