#include <gtest/gtest.h>

extern "C" {
#include "spo2_monitor.h"
}

TEST(Spo2Monitor, InitializesToTheMaximumValidSaturation)
{
    Spo2Monitor_t monitor;
    spo2_monitor_init(&monitor);

    EXPECT_EQ(spo2_monitor_get_saturation(&monitor), SPO2_MAX_VALID_PCT);
    EXPECT_EQ(spo2_monitor_get_pulse(&monitor), 0);
    EXPECT_FALSE(spo2_monitor_has_motion(&monitor));
}

TEST(Spo2Monitor, ComputesSaturationFromACompleteWindow)
{
    Spo2Monitor_t monitor;
    spo2_monitor_init(&monitor);
    for (uint16_t i = 0; i < SPO2_WINDOW_SIZE; ++i) {
        spo2_monitor_push_sample(&monitor, (i % 2 == 0) ? 900 : 1100,
                                  (i % 2 == 0) ? 800 : 1200);
    }

    spo2_monitor_update(&monitor);

    EXPECT_EQ(monitor.index, 0);
    EXPECT_EQ(spo2_monitor_get_saturation(&monitor), 97);
    EXPECT_FALSE(spo2_monitor_has_motion(&monitor));
}

TEST(Spo2Monitor, DetectsMotionFromTheMostRecentSample)
{
    Spo2Monitor_t monitor;
    spo2_monitor_init(&monitor);
    for (uint16_t i = 0; i < SPO2_WINDOW_SIZE; ++i) {
        spo2_monitor_push_sample(&monitor, 1000 + (i % 2) * 100,
                                  i == SPO2_WINDOW_SIZE - 1 ? 5000 : 1000 + (i % 2) * 100);
    }

    spo2_monitor_update(&monitor);

    EXPECT_TRUE(spo2_monitor_has_motion(&monitor));
}
