/*
 * spo2_monitor.h
 *
 * Pulse oximetry (SpO2) processing.
 * Takes red and infra-red photodiode counts, extracts the AC/DC ratio
 * for each channel and maps R = (AC_red/DC_red)/(AC_ir/DC_ir) to SpO2 %.
 */
#ifndef SPO2_MONITOR_H
#define SPO2_MONITOR_H

#include <stdint.h>
#include <stdbool.h>

#define SPO2_WINDOW_SIZE    64
#define SPO2_MIN_VALID_PCT  50
#define SPO2_MAX_VALID_PCT 100

typedef struct {
    uint16_t red_samples[SPO2_WINDOW_SIZE];
    uint16_t ir_samples[SPO2_WINDOW_SIZE];
    uint16_t index;
    uint32_t red_dc;
    uint32_t ir_dc;
    uint16_t red_ac;
    uint16_t ir_ac;
    uint8_t  spo2_percent;
    uint16_t pulse_rate_bpm;
    bool     motion_artifact;
} Spo2Monitor_t;

void    spo2_monitor_init(Spo2Monitor_t* mon);
void    spo2_monitor_push_sample(Spo2Monitor_t* mon, uint16_t red, uint16_t ir);
void    spo2_monitor_update(Spo2Monitor_t* mon);
uint8_t spo2_monitor_get_saturation(const Spo2Monitor_t* mon);
uint16_t spo2_monitor_get_pulse(const Spo2Monitor_t* mon);
bool    spo2_monitor_has_motion(const Spo2Monitor_t* mon);

#endif /* SPO2_MONITOR_H */
