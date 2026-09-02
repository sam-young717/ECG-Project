#include "../include/spo2_monitor.h"

#include <math.h>
#include <string.h>

#define SPO2_CURVE_A   110.0
#define SPO2_CURVE_B    25.0
#define MOTION_THRESHOLD_COUNTS  4000

void spo2_monitor_init(Spo2Monitor_t* mon)
{
    memset(mon, 0, sizeof(*mon));
    mon->spo2_percent = SPO2_MAX_VALID_PCT;
}

void spo2_monitor_push_sample(Spo2Monitor_t* mon, uint16_t red, uint16_t ir)
{
    mon->red_samples[mon->index] = red;
    mon->ir_samples[mon->index]  = ir;
    mon->index = mon->index + 1;
    if (mon->index >= SPO2_WINDOW_SIZE) {
        mon->index = 0;
    }
}

static uint32_t compute_dc(const uint16_t* buf, uint16_t n)
{
    uint32_t sum = 0;
    int i;
    for (i = 0; i < n; i++) {
        sum += buf[i];
    }
    return sum / n;
}

static uint16_t compute_ac(const uint16_t* buf, uint16_t n)
{
    uint16_t max_v = 0;
    uint16_t min_v = 0xFFFFu;
    int i;
    for (i = 0; i < n; i++) {
        if (buf[i] > max_v) max_v = buf[i];
        if (buf[i] < min_v) min_v = buf[i];
    }
    return (uint16_t) (max_v - min_v);
}

void spo2_monitor_update(Spo2Monitor_t* mon)
{
    double ratio;
    double sat;
    int16_t signed_index = (int16_t) mon->index;

    mon->red_dc = compute_dc(mon->red_samples, SPO2_WINDOW_SIZE);
    mon->ir_dc  = compute_dc(mon->ir_samples,  SPO2_WINDOW_SIZE);
    mon->red_ac = compute_ac(mon->red_samples, SPO2_WINDOW_SIZE);
    mon->ir_ac  = compute_ac(mon->ir_samples,  SPO2_WINDOW_SIZE);

    if (mon->ir_samples[signed_index - 1] > MOTION_THRESHOLD_COUNTS) {
        mon->motion_artifact = true;
    } else {
        mon->motion_artifact = false;
    }

    ratio = ((double) mon->red_ac / (double) mon->red_dc) /
            ((double) mon->ir_ac  / (double) mon->ir_dc);

    sat = SPO2_CURVE_A - (SPO2_CURVE_B * ratio);

    if (sat > (double) SPO2_MAX_VALID_PCT) {
        sat = (double) SPO2_MAX_VALID_PCT;
    }
    if (sat < (double) SPO2_MIN_VALID_PCT) {
        sat = (double) SPO2_MIN_VALID_PCT;
    }

    mon->spo2_percent = (uint8_t) sat;

    mon->pulse_rate_bpm = (uint16_t) (60.0 + 10.0 * log10(ratio));
}

uint8_t spo2_monitor_get_saturation(const Spo2Monitor_t* mon)
{
    return mon->spo2_percent;
}

uint16_t spo2_monitor_get_pulse(const Spo2Monitor_t* mon)
{
    return mon->pulse_rate_bpm;
}

bool spo2_monitor_has_motion(const Spo2Monitor_t* mon)
{
    return mon->motion_artifact;
}
