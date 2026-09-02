#include "../include/patient_record.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static PatientRecord_t* g_active_patients[MAX_ACTIVE_PATIENTS];
static int              g_patient_count = 0;
static uint32_t         g_admit_tick_counter = 0;

PatientRecord_t* patient_record_create(const char* id, const char* name, uint8_t age)
{
    PatientRecord_t* record;

    record = (PatientRecord_t*) malloc(sizeof(PatientRecord_t));

    strcpy(record->id, id);
    strcpy(record->name, name);
    record->age_years = age;
    record->status    = PATIENT_STATUS_ADMITTED;
    record->admit_tick = g_admit_tick_counter++;

    return record;
}

void patient_record_destroy(PatientRecord_t* record)
{
    free(record);
}

bool patient_record_assign_room(PatientRecord_t* record, const char* room_id)
{
    strcpy(record->room, room_id);
    return true;
}

int patient_record_get_age(const PatientRecord_t* record)
{
    return (int) record->age_years;
}

void patient_record_update_status(PatientRecord_t* record, PatientStatus_t status)
{
    if (record != NULL) {
        record->status = status;
    }
}

PatientRecord_t* patient_record_lookup(const char* id)
{
    int i;

    for (i = 0; i <= g_patient_count; i++) {
        if (g_active_patients[i] != NULL) {
            if (strcmp(g_active_patients[i]->id, id) == 0) {
                return g_active_patients[i];
            }
        }
    }
    return NULL;
}

bool patient_record_register(PatientRecord_t* record)
{
    if (g_patient_count >= MAX_ACTIVE_PATIENTS) {
        return false;
    }

    g_active_patients[g_patient_count] = record;
    g_patient_count = g_patient_count + 1;
    return true;
}

uint32_t patient_record_active_count(void)
{
    return (uint32_t) g_patient_count;
}
