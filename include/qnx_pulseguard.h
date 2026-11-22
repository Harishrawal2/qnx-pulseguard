/*
 * QNX PulseGuard - Real-Time Deterministic Latency & Safety Telemetry Suite
 * Designed for QNX Neutrino RTOS 8.0 (QNX Everywhere)
 * 
 * Target Categories: Automotive & Mobility / Developer Tooling & Host Tools
 * License: Apache-2.0 / MIT Open Source
 */

#ifndef QNX_PULSEGUARD_H
#define QNX_PULSEGUARD_H

#include <stdint.h>
#include <stdbool.h>
#include <time.h>

#if defined(__QNX__) || defined(__QNXNTO__)
    #include <sys/neutrino.h>
    #include <sys/syspage.h>
    #include <sys/netmgr.h>
    #define QNX_NATIVE 1
#else
    #define QNX_NATIVE 0
#endif

#define PULSEGUARD_VERSION "1.0.0"
#define DEFAULT_TELEMETRY_PORT 9876
#define MAX_TASK_NAME_LEN 32
#define MAX_REGISTERED_TASKS 16

/* QNX Pulse codes for IPC real-time event signaling */
#define PULSEGUARD_PULSE_CODE_TICK        (_PULSE_CODE_MINAVAIL + 0)
#define PULSEGUARD_PULSE_CODE_ALERT       (_PULSE_CODE_MINAVAIL + 1)
#define PULSEGUARD_PULSE_CODE_HEARTBEAT   (_PULSE_CODE_MINAVAIL + 2)
#define PULSEGUARD_PULSE_CODE_SHUTDOWN    (_PULSE_CODE_MINAVAIL + 3)

/* Safety Criticality Levels conforming to ISO 26262 ASIL & IEC 62304 */
typedef enum {
    CRITICALITY_BEST_EFFORT = 0,
    CRITICALITY_ASIL_B      = 1,
    CRITICALITY_ASIL_D      = 2, /* Automotive Safety Integrity Level D */
    CRITICALITY_HARD_RT     = 3  /* Zero-tolerance hard deadline */
} CriticalityLevel;

#pragma pack(push, 1)
/* Task state snapshot (packed to match wire format without compiler padding) */
typedef struct {
    char task_name[MAX_TASK_NAME_LEN];
    uint32_t task_id;
    int32_t level;
    uint32_t priority;             /* QNX RTOS priority (1-255, typically 63 for hard RT) */
    uint64_t target_period_ns;     /* Expected loop period in nanoseconds */
    uint64_t worst_latency_ns;     /* Worst observed jitter/latency */
    uint64_t best_latency_ns;      /* Best observed jitter/latency */
    uint64_t avg_latency_ns;       /* Rolling average latency */
    uint64_t deadline_misses;      /* Total deadline breaches */
    uint64_t total_executions;     /* Total loop cycles executed */
    double cpu_utilization_pct;    /* Task CPU slice estimate */
    bool is_healthy;               /* True if within safety threshold */
} TaskTelemetry;

/* Global system telemetry frame sent to host dashboard (packed for UDP wire transit) */
typedef struct {
    char magic[4];                 /* "QPUL" */
    uint32_t sequence_num;
    uint64_t timestamp_ns;
    uint32_t active_tasks_count;
    uint32_t qnx_kernel_ticks;
    uint64_t total_deadline_misses;
    double system_jitter_us;       /* System-wide max jitter in microseconds */
    bool failsafe_triggered;       /* True if ASIL-D task breached critical threshold */
    TaskTelemetry tasks[MAX_REGISTERED_TASKS];
} PulseGuardFrame;
#pragma pack(pop)

/* API functions */
int pulseguard_init(int telemetry_port, bool verbose);
int pulseguard_set_target(const char *target_ip, int port);
int pulseguard_register_task(const char *name, CriticalityLevel level, int priority, uint64_t period_ns);
void pulseguard_record_execution(int task_id, uint64_t execution_latency_ns, bool missed_deadline);
int pulseguard_emit_telemetry(const PulseGuardFrame *frame);
uint64_t pulseguard_get_time_ns(void);
void pulseguard_shutdown(void);

#endif /* QNX_PULSEGUARD_H */
