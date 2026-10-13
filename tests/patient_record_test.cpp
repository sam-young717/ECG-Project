#include <gtest/gtest.h>

extern "C" {
#include "patient_record.h"
}

TEST(PatientRecord, CreatesAndUpdatesDemographicInformation)
{
    PatientRecord_t* record = patient_record_create("P-001", "Jane Doe", 42);
    ASSERT_NE(record, nullptr);

    EXPECT_STREQ(record->id, "P-001");
    EXPECT_EQ(patient_record_get_age(record), 42);
    EXPECT_EQ(record->status, PATIENT_STATUS_ADMITTED);
    ASSERT_TRUE(patient_record_assign_room(record, "ICU-01"));
    patient_record_update_status(record, PATIENT_STATUS_MONITORING);
    EXPECT_STREQ(record->room, "ICU-01");
    EXPECT_EQ(record->status, PATIENT_STATUS_MONITORING);

    patient_record_destroy(record);
}

TEST(PatientRecord, RegistersAndFindsAnActivePatient)
{
    const uint32_t initial_count = patient_record_active_count();
    PatientRecord_t* record = patient_record_create("P-002", "John Doe", 61);
    ASSERT_NE(record, nullptr);

    ASSERT_TRUE(patient_record_register(record));

    EXPECT_EQ(patient_record_active_count(), initial_count + 1);
    EXPECT_EQ(patient_record_lookup("P-002"), record);
    EXPECT_EQ(patient_record_lookup("unknown"), nullptr);
}
