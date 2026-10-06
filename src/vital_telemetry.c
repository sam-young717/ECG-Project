#include "../include/vital_telemetry.h"

#include <string.h>

#define VITAL_TELEMETRY_PAYLOAD_SIZE 13U

static void encode_u16(uint8_t* payload, uint16_t value)
{
    payload[0] = (uint8_t) (value >> 8);
    payload[1] = (uint8_t) value;
}

static void encode_u32(uint8_t* payload, uint32_t value)
{
    payload[0] = (uint8_t) (value >> 24);
    payload[1] = (uint8_t) (value >> 16);
    payload[2] = (uint8_t) (value >> 8);
    payload[3] = (uint8_t) value;
}

void vital_telemetry_init(VitalTelemetry_t* telemetry, CommLink_t* link,
                          uint32_t interval_ms)
{
    if (telemetry == NULL) {
        return;
    }

    memset(telemetry, 0, sizeof(*telemetry));
    telemetry->link = link;
    telemetry->interval_ms = interval_ms;
}

bool vital_telemetry_update(VitalTelemetry_t* telemetry,
                            const VitalSample_t* sample)
{
    if (telemetry == NULL || sample == NULL || telemetry->link == NULL) {
        return false;
    }

    telemetry->pending_sample = *sample;
    telemetry->has_pending_sample = true;

    if (telemetry->has_sent &&
        (uint32_t) (sample->tick_ms - telemetry->last_sent_ms) <
        telemetry->interval_ms) {
        return true;
    }

    return vital_telemetry_flush(telemetry);
}

bool vital_telemetry_flush(VitalTelemetry_t* telemetry)
{
    CommFrame_t frame = {0};
    const VitalSample_t* sample;

    if (telemetry == NULL || telemetry->link == NULL) {
        return false;
    }
    if (!telemetry->has_pending_sample) {
        return true;
    }

    sample = &telemetry->pending_sample;
    frame.type = COMM_MSG_VITALS;
    frame.length = VITAL_TELEMETRY_PAYLOAD_SIZE;

    encode_u32(&frame.payload[0], sample->tick_ms);
    encode_u16(&frame.payload[4], sample->heart_rate);
    frame.payload[6] = sample->spo2;
    encode_u16(&frame.payload[7], sample->systolic);
    encode_u16(&frame.payload[9], sample->diastolic);
    encode_u16(&frame.payload[11], sample->respiration);

    if (!comm_link_send(telemetry->link, &frame)) {
        return false;
    }

    telemetry->last_sent_ms = sample->tick_ms;
    telemetry->frames_sent++;
    telemetry->has_sent = true;
    telemetry->has_pending_sample = false;
    return true;
}

uint32_t vital_telemetry_get_frames_sent(const VitalTelemetry_t* telemetry)
{
    if (telemetry == NULL) {
        return 0;
    }

    return telemetry->frames_sent;
}