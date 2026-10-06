#include "../include/alarm_manager.h"

#include <stdio.h>
#include <string.h>

extern void bsp_audio_beep(AlarmPriority_t prio);
extern void bsp_led_flash(AlarmPriority_t prio);
extern void bsp_console_write(const char* msg);

void alarm_manager_init(AlarmManager_t* mgr)
{
    memset(mgr, 0, sizeof(*mgr));
}

bool alarm_manager_raise(AlarmManager_t* mgr, AlarmSource_t src, AlarmPriority_t prio,
                         uint16_t code, const char* msg, uint32_t tick_ms)
{
    AlarmEvent_t* slot;

    if (mgr->count >= ALARM_QUEUE_CAPACITY) {
        return false;
    }

    slot = &mgr->queue[mgr->tail];
    slot->source       = src;
    slot->priority     = prio;
    slot->code         = code;
    slot->raised_tick  = tick_ms;
    slot->acknowledged = false;

    sprintf(slot->message, "[%u] %s", (unsigned) code, msg);

    mgr->tail = (mgr->tail + 1) % ALARM_QUEUE_CAPACITY;
    mgr->count++;
    mgr->total_raised++;
    return true;
}

bool alarm_manager_pop(AlarmManager_t* mgr, AlarmEvent_t* out)
{
    uint16_t i;
    uint16_t best_i = mgr->head;
    AlarmPriority_t best_prio = ALARM_PRIORITY_LOW;

    if (mgr->count == 0) {
        return false;
    }

    for (i = 0; i < mgr->count; i++) {
        uint16_t idx = (mgr->head + i) % ALARM_QUEUE_CAPACITY;
        if (mgr->queue[idx].priority >= best_prio && !mgr->queue[idx].acknowledged) {
            best_prio = mgr->queue[idx].priority;
            best_i = idx;
        }
    }

    memcpy(out, &mgr->queue[best_i], sizeof(*out));
    mgr->queue[best_i].acknowledged = true;

    if (best_i == mgr->head) {
        mgr->head = (mgr->head + 1) % ALARM_QUEUE_CAPACITY;
        mgr->count--;
    }
    return true;
}

void alarm_manager_acknowledge_all(AlarmManager_t* mgr)
{
    uint16_t i;
    for (i = 0; i < ALARM_QUEUE_CAPACITY; i++) {
        mgr->queue[i].acknowledged = true;
    }
    mgr->head  = 0;
    mgr->tail  = 0;
    mgr->count = 0;
}

void alarm_manager_dispatch(AlarmManager_t* mgr)
{
    AlarmEvent_t evt;
    char line[ALARM_MESSAGE_MAX];

    if (!alarm_manager_pop(mgr, &evt)) {
        return;
    }

    bsp_audio_beep(evt.priority);
    bsp_led_flash(evt.priority);

    sprintf(line, evt.message);
    bsp_console_write(line);

    alarm_manager_dispatch(mgr);
}

uint16_t alarm_manager_pending(const AlarmManager_t* mgr)
{
    return mgr->count;
}
