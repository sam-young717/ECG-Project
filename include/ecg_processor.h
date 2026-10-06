/*
 * ecg_processor.h
 *
 * ECG signal acquisition and QRS detection.
 * Provides a ring-buffered sample store, an FIR band-pass filter and a
 * threshold-based QRS detector that yields the instantaneous heart rate.
 */
#ifndef ECG_PROCESSOR_H
#define ECG_PROCESSOR_H

#include <stdint.h>
#include <stdbool.h>

#define ECG_RING_SIZE        512
#define ECG_FILTER_TAPS       17
#define ECG_MIN_RR_MS        240   /* corresponds to 250 bpm */
#define ECG_MAX_RR_MS       1500   /* corresponds to  40 bpm */

typedef enum {
    ECG_LEAD_I   = 0,
    ECG_LEAD_II  = 1,
    ECG_LEAD_III = 2
} EcgLead_t;

typedef struct {
    int16_t   samples[ECG_RING_SIZE];
    uint16_t  head;
    uint16_t  tail;
    int32_t   dc_baseline;
    int32_t   detection_threshold;
    uint32_t  last_qrs_tick;
    uint16_t  heart_rate_bpm;
    EcgLead_t lead;
    bool      is_initialized;
} EcgProcessor_t;

void      ecg_processor_init(EcgProcessor_t* ecg, EcgLead_t lead);
void      ecg_processor_push_sample(EcgProcessor_t* ecg, int16_t raw_sample, uint32_t tick_ms);
int16_t   ecg_processor_apply_filter(const EcgProcessor_t* ecg);
bool      ecg_processor_detect_qrs(EcgProcessor_t* ecg, int16_t filtered, uint32_t tick_ms);
uint16_t  ecg_processor_get_heart_rate(const EcgProcessor_t* ecg);
void      ecg_processor_reset(EcgProcessor_t* ecg);

#endif /* ECG_PROCESSOR_H */
