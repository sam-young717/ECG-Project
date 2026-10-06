/*
 * alarm_manager.h
 *
 * Priority-based physiological and technical alarm handling.
 * Alarms are enqueued from the vital-sign modules and dispatched to
 * audible / visual / remote sinks according to IEC 60601-1-8 priority.
 */
#ifndef ALARM_MANAGER_H
#define ALARM_MANAGER_H

#include <stdint.h>
#include <stdbool.h>

#define ALARM_QUEUE_CAPACITY   32
#define ALARM_MESSAGE_MAX      64

typedef enum {
    ALARM_PRIORITY_LOW    = 1,
    ALARM_PRIORITY_MEDIUM = 2,
    ALARM_PRIORITY_HIGH   = 3
} AlarmPriority_t;

typedef enum {
    ALARM_SRC_ECG   = 0,
    ALARM_SRC_SPO2  = 1,
    ALARM_SRC_NIBP  = 2,
    ALARM_SRC_TECH  = 3
} AlarmSource_t;

typedef struct {
    AlarmSource_t   source;
    AlarmPriority_t priority;
    uint16_t        code;
    uint32_t        raised_tick;
    char            message[ALARM_MESSAGE_MAX];
    bool            acknowledged;
} AlarmEvent_t;

typedef struct {
    AlarmEvent_t queue[ALARM_QUEUE_CAPACITY];
    uint16_t     head;
    uint16_t     tail;
    uint16_t     count;
    uint32_t     total_raised;
} AlarmManager_t;

void  alarm_manager_init(AlarmManager_t* mgr);
bool  alarm_manager_raise(AlarmManager_t* mgr, AlarmSource_t src, AlarmPriority_t prio,
                          uint16_t code, const char* msg, uint32_t tick_ms);
bool  alarm_manager_pop(AlarmManager_t* mgr, AlarmEvent_t* out);
void  alarm_manager_acknowledge_all(AlarmManager_t* mgr);
void  alarm_manager_dispatch(AlarmManager_t* mgr);
uint16_t alarm_manager_pending(const AlarmManager_t* mgr);

#endif /* ALARM_MANAGER_H */
