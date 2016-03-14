#include <system/system.h>
#include <system/timer.h>
#include <system/task.h>

#include <stddef.h>
#include <stdio.h>

void test()
{
    printf("Timer 1 wake up at -> %d tick\n", system_get_tick_count());
}
void test_2()
{
    printf("Timer 2 wake up at -> %d tick\n", system_get_tick_count());
}
void test_3()
{
    printf("-----------------------------\n");
    printf("Timer 3 wake up at -> %d tick\n", system_get_tick_count());
    printf("-----------------------------\n");
}

int main(void)
{
    tmr_t test_timer, test_timer_2, test_timer_3;

    printf("System abstraction test - POSIX implementation\n");

    system_init();

    tmr_create(&test_timer, true, test, NULL);
    tmr_start(&test_timer, 100);
    tmr_create(&test_timer_2, true, test_2, NULL);
    tmr_start(&test_timer_2, 275);
    tmr_create(&test_timer_3, false, test_3, NULL);
    tmr_start(&test_timer_3, 1000);
    tmr_stop(&test_timer);
    tmr_reset(&test_timer);

    system_start();

    tmr_destroy(test_timer);
    tmr_destroy(test_timer_2);
    tmr_destroy(test_timer_3);

    return 0;
}

