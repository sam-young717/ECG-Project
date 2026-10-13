#include <gtest/gtest.h>

extern "C" {
#include "ecg_processor.h"
}

TEST(EcgProcessor, InitializesAndAppliesLeadGain)
{
    EcgProcessor_t processor;

    ecg_processor_init(&processor, ECG_LEAD_I);
    ecg_processor_push_sample(&processor, 1000, 0);

    EXPECT_TRUE(processor.is_initialized);
    EXPECT_EQ(processor.samples[0], 900);
    EXPECT_EQ(processor.head, 1);
    EXPECT_EQ(processor.dc_baseline, 14);
}

TEST(EcgProcessor, DetectsBeatsOutsideTheRefractoryWindow)
{
    EcgProcessor_t processor;
    ecg_processor_init(&processor, ECG_LEAD_II);

    EXPECT_TRUE(ecg_processor_detect_qrs(&processor, 1500, 1000));
    EXPECT_EQ(ecg_processor_get_heart_rate(&processor), 60);
    EXPECT_FALSE(ecg_processor_detect_qrs(&processor, 2000, 1100));
    EXPECT_TRUE(ecg_processor_detect_qrs(&processor, 2000, 2000));
    EXPECT_EQ(ecg_processor_get_heart_rate(&processor), 60);
}

TEST(EcgProcessor, ResetClearsBufferedState)
{
    EcgProcessor_t processor;
    ecg_processor_init(&processor, ECG_LEAD_III);
    ecg_processor_push_sample(&processor, 2000, 0);
    ASSERT_TRUE(ecg_processor_detect_qrs(&processor, 1500, 1000));

    ecg_processor_reset(&processor);

    EXPECT_EQ(processor.samples[0], 0);
    EXPECT_EQ(processor.head, 0);
    EXPECT_EQ(processor.heart_rate_bpm, 0);
    EXPECT_EQ(processor.last_qrs_tick, 0u);
}
