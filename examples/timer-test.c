#include <system/system.h>
#include <system/timer.h>
#include <system/task.h>

#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>

tmr_t test_timer, test_timer_2, test_timer_3, test_timer_4, test_timer_5, test_timer_6;
int global_flag = 0;
int test_timer_4_flag = 3;
int full_time = 0;

void start()
{
    if (global_flag == 30){
        test_timer_4_flag = 0;
    }
    else if (global_flag == 34){
        test_timer_4_flag = 1;
    }
    else if (global_flag > 38){
        test_timer_4_flag = 2;
    }
    global_flag++;
}

void test()
{    
    full_time +=250;
    printf("--------------- 250 (%d) ms ---------------\n", full_time);
    start();
}
void test_2()
{
    printf("Timer 2 wake up at -> %d (%d) tick \n", system_get_tick_count(), test_timer_2.period);
}
void test_3()
{
    printf("Timer 3 wake up at -> %d (%d) tick \n", system_get_tick_count(), test_timer_3.period);
}
void test_4()
{    
    printf("Timer 4 wake up at -> %d (%d) tick \n", system_get_tick_count(), test_timer_4.period);
}
void test_5()
{    
    printf("Timer 5 wake up at -> %d (%d) tick \n", system_get_tick_count(), test_timer_5.period);
}
void test_6()
{    
    printf("Timer 6 wake up at -> %d (%d) tick \n", system_get_tick_count(), test_timer_6.period);
}

void test_reset(void* arg)
{
    bool flag = true;

    
    while (1){
        system_delay(99);
        if (full_time > 2500 && full_time < 12500)
        {
                tmr_stop(&test_timer_2);
                system_delay(250);
        }
        else if (full_time > 500 && full_time <2501){
            if (flag){
                tmr_reset(&test_timer_2);
            }
            flag = !flag;
        }
    }
}

void test_others(void* arg)
{
    printf("-------------------Schedule-----------------\n");
    printf("Timer 2 reset test starts at ----> 500 ms \n");
    printf("Timer 2 reset test ends at   ----> 2500 ms \n");
    printf("Timer 3 starts at            ----> 2000 ms \n");
    printf("Timer 3 stops at             ----> 3500 ms \n");
    printf("Timer 3 starts at            ----> 4500 ms \n");
    printf("Timer 5 starts at            ----> 5000 period 1000  ms \n");
    printf("Timer 6 starts at            ----> 5250 period 500 ms \n");
    printf("Timer 4 starts at            ----> 7500 ms \n");
    printf("Timer 4 stops at             ----> 8500 ms \n");
    printf("Timer 4 reset stop at        ----> 9500 ms \n");
    printf("Timer 4 starts at            ----> 12500 ms \n");
    printf("Timer 1 stops at             ----> 12500 ms \n");
    printf("--------------------------------------------\n");

    while(true) {
        system_delay(50);
        switch (full_time){
            case 2000:
                tmr_start(&test_timer_3,250);
                break;
            case 3500:
                tmr_stop(&test_timer_3);
                break;
            case 4500:
                tmr_start(&test_timer_3,500);
                break;
            case 5000:
                tmr_start(&test_timer_5, 1000);
                break;
            case 5250:
                tmr_start(&test_timer_6, 500);
                break;
            case 12500:
                tmr_stop(&test_timer);
                tmr_start(&test_timer_2,250);
                full_time += 1;
                break;
        }
        
        if (test_timer_4_flag == 0){
            tmr_start(&test_timer_4, 500);
            test_timer_4_flag = 3;
        }
        else if (test_timer_4_flag == 1){
            tmr_stop(&test_timer_4);
            test_timer_4_flag = 3;
        }
        else if (test_timer_4_flag == 2){
            tmr_reset(&test_timer_4);
            test_timer_4_flag = 3;
        }
    }
}

int main(void)
{
    task_t test_task, test_task_2;

    printf("System abstraction test - POSIX implementation\n");

    system_init();

    tmr_create(&test_timer, true, test, NULL);
    tmr_start(&test_timer, 250);
    tmr_create(&test_timer_2, true, test_2, NULL);
    tmr_start(&test_timer_2, 100);
    tmr_create(&test_timer_3, true, test_3, NULL);
    tmr_create(&test_timer_4, true, test_4, NULL);
    tmr_create(&test_timer_5, true, test_5, NULL);
    tmr_create(&test_timer_6, true, test_6, NULL);
    task_create(&test_task, test_others, NULL, NULL, 0, 0);
    task_create(&test_task_2, test_reset, NULL, NULL, 0, 0);

    system_start();

    tmr_destroy(test_timer);
    tmr_destroy(test_timer_2);
    tmr_destroy(test_timer_3);
    tmr_destroy(test_timer_4);
    tmr_destroy(test_timer_5);
    tmr_destroy(test_timer_6);

    return 0;
}
