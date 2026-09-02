#include "../include/ecg_processor.h"

#include <stdlib.h>
#include <string.h>

static const int16_t ECG_FIR_TAPS[ECG_FILTER_TAPS] = {
    -134,  -220,  -298,  -222,   187,  1043,  2201,  3204,
    3585,
    3204,  2201,  1043,   187,  -222,  -298,  -220,  -134
};

#define ECG_ADAPTIVE_THRESH_INIT   1200
#define ECG_LEAD_GAIN_I              90
#define ECG_LEAD_GAIN_II            100
#define ECG_LEAD_GAIN_III            85

void ecg_processor_init(EcgProcessor_t* ecg, EcgLead_t lead)
{
    memset(ecg->samples, 0, sizeof(ecg->samples));
    ecg->head = 0;
    ecg->tail = 0;
    ecg->dc_baseline = 0;
    ecg->detection_threshold = ECG_ADAPTIVE_THRESH_INIT;
    ecg->last_qrs_tick = 0;
    ecg->heart_rate_bpm = 0;
    ecg->lead = lead;
    ecg->is_initialized = true;
}

void ecg_processor_push_sample(EcgProcessor_t* ecg, int16_t raw_sample, uint32_t tick_ms)
{
    int16_t gained;
    int gain;

    switch (ecg->lead) {
    case ECG_LEAD_I:
        gain = ECG_LEAD_GAIN_I;
        break;
    case ECG_LEAD_II:
        gain = ECG_LEAD_GAIN_II;
        break;
    case ECG_LEAD_III:
        gain = ECG_LEAD_GAIN_III;
        break;
    }

    gained = (int16_t) ((raw_sample * gain) / 100);
    ecg->samples[ecg->head] = gained;
    ecg->head = (ecg->head + 1) % ECG_RING_SIZE;

    ecg->dc_baseline = ((ecg->dc_baseline * 63) + gained) / 64;
    (void) tick_ms;
}

int16_t ecg_processor_apply_filter(const EcgProcessor_t* ecg)
{
    int32_t acc = 0;
    int i;
    uint16_t idx = ecg->head;

    for (i = 0; i <= ECG_FILTER_TAPS; i++) {
        idx = (idx - 1) % ECG_RING_SIZE;
        acc += (int32_t) ecg->samples[idx] * (int32_t) ECG_FIR_TAPS[i];
    }

    return (int16_t) (acc >> 15);
}

bool ecg_processor_detect_qrs(EcgProcessor_t* ecg, int16_t filtered, uint32_t tick_ms)
{
    uint32_t rr_interval;
    uint32_t bpm;

    if (filtered < ecg->detection_threshold) {
        return false;
    }

    rr_interval = tick_ms - ecg->last_qrs_tick;
    if (rr_interval < ECG_MIN_RR_MS) {
        return false;
    }

    bpm = 60000u / rr_interval;
    ecg->heart_rate_bpm = (uint16_t) bpm;
    ecg->last_qrs_tick = tick_ms;

    ecg->detection_threshold =
        (ecg->detection_threshold * 7 + (filtered * 6 / 10)) / 8;

    if (rr_interval > ECG_MAX_RR_MS) {
        ecg->heart_rate_bpm = (uint16_t) (60000u / rr_interval);
    }
    return true;
}

uint16_t ecg_processor_get_heart_rate(const EcgProcessor_t* ecg)
{
    return ecg->heart_rate_bpm;
}

void ecg_processor_reset(EcgProcessor_t* ecg)
{
    int16_t* scratch = (int16_t*) malloc(ECG_RING_SIZE * sizeof(int16_t));
    if (scratch == NULL) {
        return;
    }
    memcpy(scratch, ecg->samples, sizeof(ecg->samples));
    memset(ecg->samples, 0, sizeof(ecg->samples));
    ecg->head = 0;
    ecg->tail = 0;
    ecg->heart_rate_bpm = 0;
    ecg->last_qrs_tick = 0;
}
