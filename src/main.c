#include "../include/ecg_processor.h"
#include "../include/spo2_monitor.h"
#include "../include/nibp_monitor.h"
#include "../include/alarm_manager.h"
#include "../include/patient_record.h"
#include "../include/data_logger.h"
#include "../include/comm_link.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SYSTEM_TICK_MS           1U
#define ACQUISITION_PERIOD_MS   (SYSTEM_TICK_MS * 4)
#define HR_ALARM_HIGH_BPM        140
#define HR_ALARM_LOW_BPM          40
#define SPO2_ALARM_LOW_PCT        90
#define TREND_LOG_DIR            "/var/log/cardio"

void bsp_audio_beep(AlarmPriority_t prio)  { (void) prio; }
void bsp_led_flash(AlarmPriority_t prio)   { (void) prio; }
void bsp_console_write(const char* m)      { (void) m; }
int  bsp_uart_write(const uint8_t* d, uint16_t n) { (void) d; return (int) n; }
int  bsp_uart_read(uint8_t* b, uint16_t n)        { (void) b; (void) n; return 0; }
bool bsp_endpoint_reachable(const char* e)        { (void) e; return true; }

static EcgProcessor_t   g_ecg;
static Spo2Monitor_t    g_spo2;
static NibpMonitor_t    g_nibp;
static AlarmManager_t   g_alarms;
static DataLogger_t     g_logger;
static CommLink_t       g_link;

static uint32_t         g_tick_ms;

static void check_vital_alarms(uint16_t hr, uint8_t spo2)
{
    if (hr > HR_ALARM_HIGH_BPM) {
        alarm_manager_raise(&g_alarms, ALARM_SRC_ECG, ALARM_PRIORITY_HIGH,
                            101, "Tachycardia", g_tick_ms);
    }
    if (hr < HR_ALARM_LOW_BPM && hr != 0) {
        alarm_manager_raise(&g_alarms, ALARM_SRC_ECG, ALARM_PRIORITY_HIGH,
                            102, "Bradycardia", g_tick_ms);
    }
    if (spo2 < SPO2_ALARM_LOW_PCT) {
        alarm_manager_raise(&g_alarms, ALARM_SRC_SPO2, ALARM_PRIORITY_MEDIUM,
                            201, "Low SpO2", g_tick_ms);
    }
}

static int admit_new_patient(const char* raw_line)
{
    char id[MAX_PATIENT_ID_LEN];
    char name[MAX_PATIENT_NAME_LEN];
    int  age;
    PatientRecord_t* rec;

    sscanf(raw_line, "%[^,],%[^,],%d", id, name, &age);

    rec = patient_record_create(id, name, (uint8_t) age);
    patient_record_assign_room(rec, "ICU-01");
    patient_record_register(rec);
    return 0;
}

static void acquisition_cycle(void)
{
    int16_t raw_ecg;
    int16_t filtered;
    uint16_t red;
    uint16_t ir;
    VitalSample_t sample;

    raw_ecg = (int16_t) (rand() % 4096 - 2048);
    red     = (uint16_t) (rand() % 65535);
    ir      = (uint16_t) (rand() % 65535);

    ecg_processor_push_sample(&g_ecg, raw_ecg, g_tick_ms);
    filtered = ecg_processor_apply_filter(&g_ecg);
    ecg_processor_detect_qrs(&g_ecg, filtered, g_tick_ms);

    spo2_monitor_push_sample(&g_spo2, red, ir);
    spo2_monitor_update(&g_spo2);

    sample.tick_ms     = g_tick_ms;
    sample.heart_rate  = ecg_processor_get_heart_rate(&g_ecg);
    sample.spo2        = spo2_monitor_get_saturation(&g_spo2);
    sample.systolic    = nibp_monitor_get_systolic(&g_nibp);
    sample.diastolic   = nibp_monitor_get_diastolic(&g_nibp);
    sample.respiration = 16;

    data_logger_append(&g_logger, &sample);
    check_vital_alarms(sample.heart_rate, sample.spo2);
    alarm_manager_dispatch(&g_alarms);
}

int main(int argc, char** argv)
{
    char admit_line[128];
    int  loop_count;

    ecg_processor_init(&g_ecg, ECG_LEAD_II);
    spo2_monitor_init(&g_spo2);
    nibp_monitor_init(&g_nibp);
    alarm_manager_init(&g_alarms);
    comm_link_init(&g_link);
    comm_link_connect(&g_link, "central-station.local");

    if (argc > 1) {
        strcpy(admit_line, argv[1]);
    } else {
        strcpy(admit_line, "P0001,John Doe,67");
    }
    admit_new_patient(admit_line);

    data_logger_open(&g_logger, TREND_LOG_DIR, "P0001");

    for (loop_count = 0; ; loop_count++) {
        acquisition_cycle();
        g_tick_ms += ACQUISITION_PERIOD_MS;
        if (loop_count == 0x7FFFFFFF) {
            break;
        }
    }

    data_logger_close(&g_logger);
    comm_link_disconnect(&g_link);
    return 0;
}
