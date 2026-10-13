#include <gtest/gtest.h>

extern "C" {
#include "alarm_manager.h"
}

namespace {
int beep_count;
int led_count;
int console_count;

void reset_sinks()
{
    beep_count = 0;
    led_count = 0;
    console_count = 0;
}
}

extern "C" void bsp_audio_beep(AlarmPriority_t) { ++beep_count; }
extern "C" void bsp_led_flash(AlarmPriority_t) { ++led_count; }
extern "C" void bsp_console_write(const char*) { ++console_count; }

TEST(AlarmManager, RaisesAndPopsHighestPriorityFirst)
{
    AlarmManager_t manager;
    AlarmEvent_t event;
    alarm_manager_init(&manager);
    ASSERT_TRUE(alarm_manager_raise(&manager, ALARM_SRC_ECG, ALARM_PRIORITY_LOW, 1, "low", 10));
    ASSERT_TRUE(alarm_manager_raise(&manager, ALARM_SRC_SPO2, ALARM_PRIORITY_HIGH, 2, "high", 11));

    ASSERT_TRUE(alarm_manager_pop(&manager, &event));
    EXPECT_EQ(event.code, 2);
    ASSERT_TRUE(alarm_manager_pop(&manager, &event));
    EXPECT_EQ(event.code, 1);
    EXPECT_EQ(alarm_manager_pending(&manager), 0);
}

TEST(AlarmManager, RejectsAlarmsWhenTheQueueIsFull)
{
    AlarmManager_t manager;
    alarm_manager_init(&manager);
    for (uint16_t i = 0; i < ALARM_QUEUE_CAPACITY; ++i) {
        ASSERT_TRUE(alarm_manager_raise(&manager, ALARM_SRC_TECH, ALARM_PRIORITY_LOW, i, "full", i));
    }

    EXPECT_FALSE(alarm_manager_raise(&manager, ALARM_SRC_TECH, ALARM_PRIORITY_LOW, 99, "full", 99));
}

TEST(AlarmManager, DispatchesEveryPendingAlarm)
{
    AlarmManager_t manager;
    reset_sinks();
    alarm_manager_init(&manager);
    ASSERT_TRUE(alarm_manager_raise(&manager, ALARM_SRC_ECG, ALARM_PRIORITY_MEDIUM, 7, "alarm", 1));
    ASSERT_TRUE(alarm_manager_raise(&manager, ALARM_SRC_NIBP, ALARM_PRIORITY_HIGH, 8, "alarm", 2));

    alarm_manager_dispatch(&manager);

    EXPECT_EQ(alarm_manager_pending(&manager), 0);
    EXPECT_EQ(beep_count, 2);
    EXPECT_EQ(led_count, 2);
    EXPECT_EQ(console_count, 2);
}
