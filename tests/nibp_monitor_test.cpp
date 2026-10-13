#include <gtest/gtest.h>

extern "C" {
#include "nibp_monitor.h"
}

TEST(NibpMonitor, InitializesAndStartsInflation)
{
    NibpMonitor_t monitor;
    nibp_monitor_init(&monitor);
    EXPECT_EQ(monitor.state, NIBP_STATE_IDLE);

    nibp_monitor_start(&monitor, 180);
    EXPECT_EQ(monitor.state, NIBP_STATE_INFLATING);
    EXPECT_EQ(monitor.target_inflate_mmhg, 180);
}

TEST(NibpMonitor, CalculatesPressureResultsAfterDeflation)
{
    NibpMonitor_t monitor;
    nibp_monitor_init(&monitor);
    nibp_monitor_start(&monitor, 180);
    nibp_monitor_step(&monitor, 180, 0, 1);
    nibp_monitor_step(&monitor, 180, 20, 2);
    nibp_monitor_step(&monitor, 160, 60, 3);
    nibp_monitor_step(&monitor, 140, 100, 4);
    nibp_monitor_step(&monitor, 120, 80, 5);
    nibp_monitor_step(&monitor, 100, 50, 6);
    nibp_monitor_step(&monitor, 30, 20, 7);
    nibp_monitor_step(&monitor, 30, 0, 8);

    ASSERT_TRUE(nibp_monitor_result_ready(&monitor));
    EXPECT_EQ(nibp_monitor_get_systolic(&monitor), 180);
    EXPECT_EQ(nibp_monitor_get_diastolic(&monitor), 120);
    EXPECT_EQ(nibp_monitor_get_map(&monitor), 140);
}

TEST(NibpMonitor, RejectsMoreThanTheMaximumNumberOfOscillations)
{
    NibpMonitor_t monitor;
    nibp_monitor_init(&monitor);
    nibp_monitor_start(&monitor, 1);
    nibp_monitor_step(&monitor, 1, 0, 0);
    monitor.pulse_count = NIBP_MAX_OSCILLATIONS;

    nibp_monitor_step(&monitor, 100, 50, 1);

    EXPECT_EQ(monitor.state, NIBP_STATE_ERROR);
}
