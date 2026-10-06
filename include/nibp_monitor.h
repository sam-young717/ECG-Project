/*
 * nibp_monitor.h
 *
 * Non-invasive blood pressure (oscillometric) state machine.
 * Drives the cuff pump/valve and computes systolic/diastolic/MAP from the
 * pulse-amplitude envelope captured during the deflation ramp.
 */
#ifndef NIBP_MONITOR_H
#define NIBP_MONITOR_H

#include <stdint.h>
#include <stdbool.h>

#define NIBP_MAX_OSCILLATIONS  64
#define NIBP_TARGET_INFLATE_MMHG 180

typedef enum {
    NIBP_STATE_IDLE       = 0,
    NIBP_STATE_INFLATING  = 1,
    NIBP_STATE_DEFLATING  = 2,
    NIBP_STATE_COMPUTING  = 3,
    NIBP_STATE_COMPLETE   = 4,
    NIBP_STATE_ERROR      = 5
} NibpState_t;

typedef struct {
    uint16_t    cuff_pressure_mmhg;
    uint16_t    amplitude;
} NibpOscillation_t;

typedef struct {
    NibpState_t         state;
    NibpOscillation_t   pulses[NIBP_MAX_OSCILLATIONS];
    uint16_t            pulse_count;
    uint16_t            systolic_mmhg;
    uint16_t            diastolic_mmhg;
    uint16_t            map_mmhg;
    uint16_t            target_inflate_mmhg;
    uint32_t            start_tick;
} NibpMonitor_t;

void    nibp_monitor_init(NibpMonitor_t* nibp);
void    nibp_monitor_start(NibpMonitor_t* nibp, uint16_t target_mmhg);
void    nibp_monitor_step(NibpMonitor_t* nibp, uint16_t cuff_mmhg, uint16_t pulse_amp, uint32_t tick_ms);
bool    nibp_monitor_result_ready(const NibpMonitor_t* nibp);
uint16_t nibp_monitor_get_systolic(const NibpMonitor_t* nibp);
uint16_t nibp_monitor_get_diastolic(const NibpMonitor_t* nibp);
uint16_t nibp_monitor_get_map(const NibpMonitor_t* nibp);

#endif /* NIBP_MONITOR_H */
