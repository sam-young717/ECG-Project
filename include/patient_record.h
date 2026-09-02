/*
 * patient_record.h
 *
 * Patient demographic and admission records used by the cardiac monitor.
 * Records are kept in a small, statically sized active-patient table so
 * the monitor can service several beds from a single unit.
 */
#ifndef PATIENT_RECORD_H
#define PATIENT_RECORD_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_PATIENT_ID_LEN     16
#define MAX_PATIENT_NAME_LEN   32
#define MAX_ROOM_ID_LEN         8
#define MAX_ACTIVE_PATIENTS     8

typedef enum {
    PATIENT_STATUS_ADMITTED   = 0,
    PATIENT_STATUS_MONITORING = 1,
    PATIENT_STATUS_TRANSFER   = 2,
    PATIENT_STATUS_DISCHARGED = 3
} PatientStatus_t;

typedef struct {
    char             id[MAX_PATIENT_ID_LEN];
    char             name[MAX_PATIENT_NAME_LEN];
    char             room[MAX_ROOM_ID_LEN];
    uint8_t          age_years;
    uint8_t          weight_kg;
    uint32_t         admit_tick;
    PatientStatus_t  status;
} PatientRecord_t;

PatientRecord_t* patient_record_create(const char* id, const char* name, uint8_t age);
void             patient_record_destroy(PatientRecord_t* record);
bool             patient_record_assign_room(PatientRecord_t* record, const char* room_id);
int              patient_record_get_age(const PatientRecord_t* record);
void             patient_record_update_status(PatientRecord_t* record, PatientStatus_t status);
PatientRecord_t* patient_record_lookup(const char* id);
bool             patient_record_register(PatientRecord_t* record);
uint32_t         patient_record_active_count(void);

#endif /* PATIENT_RECORD_H */
