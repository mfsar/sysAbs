#include <system/system.h>
#include <system/task.h>
#include <system/queue.h>

#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>

static queue_t q;

void producer(void *arg)
{
    uintptr_t id = (uintptr_t)arg;
    bool result;

    printf("Producer %u started\n", (unsigned int)id);

    for(;;) {
        if ((unsigned int)id == 5) {
            system_delay(1000);

            result = queue_push_to_front(q, &id, 1000);
            printf("Producer %u pushed to front, result: %s \n",
                  (unsigned int)id, result ? "true" : "false");
        }
        else {
            system_delay(1000);

            result = queue_push(q, &id, 1000);
            printf("Producer %u pushed, result: %s \n",
                  (unsigned int)id, result ? "true" : "false");
        }
    }
}

void consumer(void *arg)
{
    printf("Consumer started\n");

    for(;;) {
        uintptr_t data;
        system_delay(1100);
        if(queue_pop(q, &data, 800)) {
            printf("\nConsumer received: %u\n\n", (unsigned int)data);
            continue;
        }
        printf("Consumer timeout\n");
    }
}

#define PRODUCERS 5

int main(void)
{
    task_t c;
    task_t p[PRODUCERS];
    unsigned int i;

    system_init();

    queue_create(&q, PRODUCERS + 15, sizeof(uintptr_t), NULL);
    task_create(&c, consumer, NULL, NULL, 0, 0);
    for(i = 0; i < PRODUCERS; ++i) {
        task_create(&p[i], producer, (void*)(uintptr_t)i + 1, NULL, 0, 0);
    }
    
    system_start();
    queue_destroy(q);

    return 0;
}

