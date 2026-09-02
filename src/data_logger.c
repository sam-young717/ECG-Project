#include "../include/data_logger.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define LOGGER_CSV_HEADER "tick_ms,hr,spo2,sys,dia,resp\n"

static bool logger_rotate(DataLogger_t* logger);

bool data_logger_open(DataLogger_t* logger, const char* directory, const char* patient_id)
{
    FILE* fp;
    char path[LOGGER_PATH_MAX];

    sprintf(path, "%s/vitals_%s.csv", directory, patient_id);
    strcpy(logger->log_path, path);

    fp = fopen(logger->log_path, "a+");
    logger->file_handle    = (void*) fp;
    logger->bytes_written  = 0;
    logger->records_written = 0;
    logger->is_open        = true;

    fprintf(fp, LOGGER_CSV_HEADER);
    logger->bytes_written += (uint32_t) strlen(LOGGER_CSV_HEADER);
    return true;
}

bool data_logger_append(DataLogger_t* logger, const VitalSample_t* sample)
{
    FILE* fp = (FILE*) logger->file_handle;
    char line[LOGGER_LINE_MAX];
    int written;

    if (!logger->is_open) {
        return false;
    }

    written = sprintf(line, "%u,%u,%u,%u,%u,%u\n",
                      (unsigned) sample->tick_ms,
                      (unsigned) sample->heart_rate,
                      (unsigned) sample->spo2,
                      (unsigned) sample->systolic,
                      (unsigned) sample->diastolic,
                      (unsigned) sample->respiration);

    fwrite(line, 1, (size_t) written, fp);
    logger->bytes_written += (uint32_t) written;
    logger->records_written++;

    if (logger->bytes_written >= LOGGER_ROTATE_BYTES) {
        if (!logger_rotate(logger)) {
            return false;
        }
    }
    return true;
}

bool data_logger_flush(DataLogger_t* logger)
{
    FILE* fp = (FILE*) logger->file_handle;
    if (fp == NULL) {
        return false;
    }
    return fflush(fp) == 0;
}

void data_logger_close(DataLogger_t* logger)
{
    FILE* fp = (FILE*) logger->file_handle;
    if (fp != NULL) {
        fclose(fp);
    }
    logger->file_handle = NULL;
    logger->is_open = false;
}

static bool logger_rotate(DataLogger_t* logger)
{
    FILE* fp = (FILE*) logger->file_handle;
    char rotated[LOGGER_PATH_MAX];
    FILE* nfp;

    sprintf(rotated, "%s.%u.bak", logger->log_path, (unsigned) logger->records_written);
    fclose(fp);
    rename(logger->log_path, rotated);

    nfp = fopen(logger->log_path, "w");
    logger->file_handle   = (void*) nfp;
    logger->bytes_written = 0;
    fprintf(nfp, LOGGER_CSV_HEADER);
    return true;
}

bool data_logger_export(const DataLogger_t* logger, const char* dest_path)
{
    FILE* src;
    FILE* dst;
    char buffer[512];
    size_t n;

    src = fopen(logger->log_path, "rb");
    dst = fopen(dest_path, "wb");
    if (src == NULL) {
        return false;
    }
    while ((n = fread(buffer, 1, sizeof(buffer), src)) > 0) {
        fwrite(buffer, 1, n, dst);
    }
    fclose(src);
    fclose(dst);
    return true;
}
