#include <system/system.h>
#include <system/task.h>
#include <system/mutex.h>
#include <system/smphr.h>

#include "posix-timer.h"

#include <stdbool.h>
#include <stdlib.h>

static mutex_t psx_tmr_mtx;
static smphr_t psx_tmr_s;

static tmr_t *timer_list;

static void tmr_run(void* arg);
static bool delete_from_timer_list(tmr_t *timer_list, tmr_t *delete_timer);
static bool add_to_timer_list(tmr_t *timer_list, tmr_t *new_timer);

static bool tmr_stop_locked(tmr_t *t);
static bool tmr_start_locked(tmr_t *t, system_tick_t period);

bool 
tmr_init(task_t *t)
{
    
    task_create(t, tmr_run, NULL, NULL, 0, 0);
    mutex_init(&psx_tmr_mtx, NULL);
    smphr_init(&psx_tmr_s, true, NULL);
    
    timer_list = (tmr_t*) malloc(sizeof(tmr_t));
    timer_list->handle = (posix_timer_t*) malloc(sizeof(posix_timer_t));
    ((posix_timer_t*)(timer_list->handle))->next_timer = NULL;
    ((posix_timer_t*)(timer_list->handle))->prev_timer = NULL;
    ((posix_timer_t*)(timer_list->handle))->wakeup_time = 0;
    return (true);
}

void 
tmr_run(void* arg)
{
    tmr_t *current_timer;
    posix_timer_t *current_posix_timer;
    system_tick_t ticks_to_callback, current_ticks;
    
    while (1) {
        mutex_lock(&psx_tmr_mtx, SYSTEM_MAX_WAIT);
        current_timer = ((posix_timer_t*)(timer_list->handle))->next_timer;
        if (current_timer!=NULL) {
            current_posix_timer = ((posix_timer_t*)(current_timer->handle));
            current_ticks = system_get_tick_count();
            if (current_posix_timer->wakeup_time > current_ticks) {    
                ticks_to_callback = current_posix_timer->wakeup_time - current_ticks;
            } else {
                ticks_to_callback = SYSTEM_NO_WAIT;
            }
        } else {
            ticks_to_callback = SYSTEM_MAX_WAIT;
        }
        mutex_unlock(&psx_tmr_mtx);

        if (!smphr_take(&psx_tmr_s, ticks_to_callback)) {
            mutex_lock(&psx_tmr_mtx, SYSTEM_MAX_WAIT);
            if (current_timer != NULL) {
                current_posix_timer->callback(NULL);
                tmr_stop_locked(current_timer);
                if (current_posix_timer->autoreload) {
                    tmr_start_locked(current_timer, current_timer->period);
                }
            }
            mutex_unlock(&psx_tmr_mtx);
        }    
    }
}

//-------------------------------------------------------------------------------

bool
tmr_create(tmr_t *t, bool autoreload, timer_callback_t callback,
                  const char *name)
{
    posix_timer_t *new_timer;
    new_timer = (posix_timer_t*) malloc(sizeof(posix_timer_t));
    
    if (!new_timer) {
        return (false);
    }
    
    new_timer->callback = callback;
    new_timer->autoreload = autoreload;
    new_timer->wakeup_time = SYSTEM_MAX_WAIT;
    new_timer->is_running = (false);
    new_timer->next_timer = NULL;
    new_timer->prev_timer = NULL;
    
    t->handle = (void*)new_timer;
    t->period = 0;
    
    return (true);                  
}

bool 
tmr_start(tmr_t *t, system_tick_t period)
{
    posix_timer_t *current_timer;
    
    mutex_lock(&psx_tmr_mtx, SYSTEM_MAX_WAIT);
    current_timer = ((posix_timer_t*)(t->handle));
    
    if (current_timer->is_running == (true)) {
        tmr_stop_locked(t);
    }
    
    t->period = period;
    current_timer->wakeup_time = t->period + system_get_tick_count();
    current_timer->is_running = (true);
    
    add_to_timer_list(timer_list, t);
    smphr_give(&psx_tmr_s);
    mutex_unlock(&psx_tmr_mtx);
    return (true);
}

bool 
tmr_reset(tmr_t *t)
{
    
    mutex_lock(&psx_tmr_mtx, SYSTEM_MAX_WAIT);
    if (!t->period){
        mutex_unlock(&psx_tmr_mtx);
        return (false);
    }
    mutex_unlock(&psx_tmr_mtx);
    return tmr_start(t, t->period);
}

bool 
tmr_stop(tmr_t *t)
{
    posix_timer_t *posix_timer;
    
    mutex_lock(&psx_tmr_mtx, SYSTEM_MAX_WAIT);
    posix_timer = ((posix_timer_t*)(t->handle));
    if (posix_timer->is_running) {
        delete_from_timer_list(timer_list, t);
        smphr_give(&psx_tmr_s);
        
        posix_timer->wakeup_time = 0;
        posix_timer->is_running = (false);
        mutex_unlock(&psx_tmr_mtx);
        
        return (true);
    }
    mutex_unlock(&psx_tmr_mtx);
    return (true);
}

bool 
tmr_is_running(tmr_t *t)
{
    bool state;
    
    mutex_lock(&psx_tmr_mtx, SYSTEM_MAX_WAIT);
    state = ((posix_timer_t*)(t->handle))->is_running;
    mutex_unlock(&psx_tmr_mtx);
    return state;
}

//-------------------------------------------------------------------------------

bool
add_to_timer_list(tmr_t *timer_list, tmr_t *new_timer)
{
    tmr_t *pointer, *next;
    posix_timer_t *current, *new;
    pointer = timer_list; 
    
    new = ((posix_timer_t*)(new_timer->handle));
    while (((posix_timer_t*)(pointer->handle))->next_timer != NULL)    {
        current = ((posix_timer_t*)(pointer->handle));
        next = current->next_timer;    
        if (((posix_timer_t*)(next->handle))->wakeup_time > new->wakeup_time) {
            new->prev_timer = pointer;
            new->next_timer = current->next_timer;
            current->next_timer = new_timer;
            ((posix_timer_t*)(next->handle))->prev_timer = new_timer;
            return (true);  
        } 
        pointer = current->next_timer;
    }
    current = ((posix_timer_t*)(pointer->handle));
    new->prev_timer = pointer;
    new->next_timer = NULL;
    current->next_timer = new_timer;

    return (true);
}

bool
delete_from_timer_list(tmr_t *timer_list, tmr_t *delete_timer)
{
    tmr_t *pointer, *next;
    posix_timer_t *current, *delete;
    pointer = timer_list; 
    
    delete = ((posix_timer_t*)(delete_timer->handle));
    current = ((posix_timer_t*)(pointer->handle));
        
    while (current->next_timer != NULL)    {    
        current = ((posix_timer_t*)(pointer->handle));
        if (current->next_timer == delete_timer) {
            next = delete->next_timer;
            current->next_timer = delete->next_timer;
            if (next != NULL){
                ((posix_timer_t*)(next->handle))->prev_timer = delete->prev_timer;
            }
            return (true);  
        } 
        pointer = current->next_timer;
    }
    return (false);
}

bool 
tmr_start_locked(tmr_t *t, system_tick_t period)
{
    posix_timer_t *current_timer;
    
    current_timer = ((posix_timer_t*)(t->handle));
        
    t->period = period;
    current_timer->wakeup_time = t->period + system_get_tick_count();
    current_timer->is_running = (true);
    
    add_to_timer_list(timer_list, t);
    smphr_give(&psx_tmr_s);
    return (true);
}

bool 
tmr_stop_locked(tmr_t *t)
{
    posix_timer_t *posix_timer;
    
    posix_timer = ((posix_timer_t*)(t->handle));
    if (posix_timer->is_running) {
        delete_from_timer_list(timer_list, t);
        smphr_give(&psx_tmr_s);
        
        posix_timer->wakeup_time = 0;
        posix_timer->is_running = (false);
        
        return (true);
    }
    return (true);
}






