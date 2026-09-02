/*
 * data_logger.h
 *
 * Trend-log persistence for vital signs.
 * Appends compact numeric records to an on-device file that can later be
 * offloaded to the hospital PACS via the communication link.
 */
#ifndef DATA_LOGGER_H
#define DATA_LOGGER_H

#include <stdint.h>
#include <stdbool.h>

#define LOGGER_PATH_MAX      128
#define LOGGER_LINE_MAX      160
#define LOGGER_ROTATE_BYTES  (256 * 1024)

typedef struct {
    uint32_t tick_ms;
    uint16_t heart_rate;
    uint8_t  spo2;
    uint16_t systolic;
    uint16_t diastolic;
    uint16_t respiration;
} VitalSample_t;

typedef struct {
    char   log_path[LOGGER_PATH_MAX];
    void*  file_handle;     /* FILE* kept opaque to callers */
    uint32_t bytes_written;
    uint32_t records_written;
    bool   is_open;
} DataLogger_t;

bool  data_logger_open(DataLogger_t* logger, const char* directory, const char* patient_id);
bool  data_logger_append(DataLogger_t* logger, const VitalSample_t* sample);
bool  data_logger_flush(DataLogger_t* logger);
void  data_logger_close(DataLogger_t* logger);
bool  data_logger_export(const DataLogger_t* logger, const char* dest_path);

#endif /* DATA_LOGGER_H */
