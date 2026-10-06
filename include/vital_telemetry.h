#ifndef VITAL_TELEMETRY_H
#define VITAL_TELEMETRY_H

#include "comm_link.h"
#include "data_logger.h"

typedef struct {
    CommLink_t*   link;
    VitalSample_t pending_sample;
    uint32_t      interval_ms;
    uint32_t      last_sent_ms;
    uint32_t      frames_sent;
    bool          has_sent;
    bool          has_pending_sample;
} VitalTelemetry_t;

void vital_telemetry_init(VitalTelemetry_t* telemetry, CommLink_t* link,
                          uint32_t interval_ms);
bool vital_telemetry_update(VitalTelemetry_t* telemetry,
                            const VitalSample_t* sample);
bool vital_telemetry_flush(VitalTelemetry_t* telemetry);
uint32_t vital_telemetry_get_frames_sent(const VitalTelemetry_t* telemetry);

#endif