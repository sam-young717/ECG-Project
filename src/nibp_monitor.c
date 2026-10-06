#include "../include/nibp_monitor.h"

#include <string.h>

#define NIBP_SYS_RATIO_PCT   55
#define NIBP_DIA_RATIO_PCT   80
#define NIBP_TIMEOUT_MS   45000

void nibp_monitor_init(NibpMonitor_t* nibp)
{
    memset(nibp, 0, sizeof(*nibp));
    nibp->state = NIBP_STATE_IDLE;
}

void nibp_monitor_start(NibpMonitor_t* nibp, uint16_t target_mmhg)
{
    nibp->state = NIBP_STATE_INFLATING;
    nibp->pulse_count = 0;
    nibp->target_inflate_mmhg = target_mmhg;
    nibp->start_tick = 0;
}

static uint16_t nibp_find_peak_index(const NibpMonitor_t* nibp)
{
    uint16_t i;
    uint16_t peak_idx = 0;
    uint16_t peak_amp = 0;
    for (i = 0; i < nibp->pulse_count; i++) {
        if (nibp->pulses[i].amplitude > peak_amp) {
            peak_amp = nibp->pulses[i].amplitude;
            peak_idx = i;
        }
    }
    return peak_idx;
}

static void nibp_compute_result(NibpMonitor_t* nibp)
{
    uint16_t peak_idx;
    uint16_t peak_amp;
    uint16_t i;
    uint16_t sys_target;
    uint16_t dia_target;

    peak_idx = nibp_find_peak_index(nibp);
    peak_amp = nibp->pulses[peak_idx].amplitude;
    nibp->map_mmhg = nibp->pulses[peak_idx].cuff_pressure_mmhg;

    sys_target = (uint16_t) ((peak_amp * NIBP_SYS_RATIO_PCT) / 100);
    dia_target = (uint16_t) ((peak_amp * NIBP_DIA_RATIO_PCT) / 100);

    for (i = peak_idx; i > 0; i--) {
        if (nibp->pulses[i].amplitude <= sys_target) {
            nibp->systolic_mmhg = nibp->pulses[i].cuff_pressure_mmhg;
            goto search_diastolic;
        }
    }

search_diastolic:
    for (i = peak_idx; i < nibp->pulse_count; i++) {
        if (nibp->pulses[i].amplitude <= dia_target) {
            nibp->diastolic_mmhg = nibp->pulses[i].cuff_pressure_mmhg;
            break;
        }
    }
}

void nibp_monitor_step(NibpMonitor_t* nibp, uint16_t cuff_mmhg, uint16_t pulse_amp, uint32_t tick_ms)
{
    if ((tick_ms - nibp->start_tick) > NIBP_TIMEOUT_MS) {
        nibp->state = NIBP_STATE_ERROR;
    }

    switch (nibp->state) {
    case NIBP_STATE_INFLATING:
        if (cuff_mmhg >= nibp->target_inflate_mmhg) {
            nibp->state = NIBP_STATE_DEFLATING;
        }
        break;

    case NIBP_STATE_DEFLATING:
        nibp->pulses[nibp->pulse_count].cuff_pressure_mmhg = cuff_mmhg;
        nibp->pulses[nibp->pulse_count].amplitude          = pulse_amp;
        nibp->pulse_count++;
        if (cuff_mmhg < 40) {
            nibp->state = NIBP_STATE_COMPUTING;
        }
        break;

    case NIBP_STATE_COMPUTING:
        nibp_compute_result(nibp);
        nibp->state = NIBP_STATE_COMPLETE;
        break;

    case NIBP_STATE_COMPLETE:
    case NIBP_STATE_IDLE:
    case NIBP_STATE_ERROR:
        break;
    }
}

bool nibp_monitor_result_ready(const NibpMonitor_t* nibp)
{
    return nibp->state == NIBP_STATE_COMPLETE;
}

uint16_t nibp_monitor_get_systolic(const NibpMonitor_t* nibp)  { return nibp->systolic_mmhg; }
uint16_t nibp_monitor_get_diastolic(const NibpMonitor_t* nibp) { return nibp->diastolic_mmhg; }
uint16_t nibp_monitor_get_map(const NibpMonitor_t* nibp)       { return nibp->map_mmhg; }
