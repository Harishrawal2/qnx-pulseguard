/*
 * QNX PulseGuard - Vehicle / Medical ECU Simulation Runner
 * Demonstrates QNX 8.0 Real-Time Scheduling vs Workload Contention
 */

#include "qnx_pulseguard.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#if defined(__QNX__) || defined(__QNXNTO__)
    #include <sys/neutrino.h>
    #include <sched.h>
#endif

#ifdef _WIN32
    #include <windows.h>
    #define sleep_ms(x) Sleep(x)
    #define sleep_us(x) { LARGE_INTEGER f, t1, t2; QueryPerformanceFrequency(&f); QueryPerformanceCounter(&t1); do { QueryPerformanceCounter(&t2); } while ((t2.QuadPart - t1.QuadPart) * 1000000 / f.QuadPart < x); }
#else
    #define sleep_ms(x) usleep((x) * 1000)
    #define sleep_us(x) usleep(x)
#endif

typedef struct {
    int task_id;
    uint64_t period_ns;
    int priority;
    CriticalityLevel level;
    volatile bool *running;
} WorkerContext;

static volatile bool g_app_running = true;

/* High-priority real-time worker thread simulating an ECU loop */
void *ecu_worker_thread(void *arg) {
    WorkerContext *ctx = (WorkerContext*)arg;

#if QNX_NATIVE
    /* Set real-time FIFO scheduler with exact QNX priority */
    struct sched_param param;
    param.sched_priority = ctx->priority;
    if (pthread_setschedparam(pthread_self(), SCHED_FIFO, &param) != 0) {
        perror("[Worker] Warning: Could not set SCHED_FIFO");
    }
#endif

    uint64_t next_wakeup = pulseguard_get_time_ns();

    while (*(ctx->running)) {
        uint64_t start_time = pulseguard_get_time_ns();

        /* Simulate ECU computations (sensor processing, PID loop, CRC check) */
        volatile double val = 1.0001;
        int iterations = (ctx->level == CRITICALITY_ASIL_D) ? 1200 : 8000;
        for (int i = 0; i < iterations; ++i) {
            val *= 1.000001;
        }

        uint64_t end_time = pulseguard_get_time_ns();
        uint64_t elapsed_ns = end_time - start_time;

        /* Check for deadline violation (e.g. computation exceeded period) */
        bool missed = (elapsed_ns > ctx->period_ns);
        pulseguard_record_execution(ctx->task_id, elapsed_ns, missed);

        /* Sleep until next deterministic period */
        next_wakeup += ctx->period_ns;
        uint64_t now = pulseguard_get_time_ns();
        if (next_wakeup > now) {
            uint64_t sleep_duration_us = (next_wakeup - now) / 1000;
            sleep_us((long)sleep_duration_us);
        } else {
            /* Slippage detected - catch up to current time */
            next_wakeup = now;
        }
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    printf("=================================================================\n");
    printf("    QNX PulseGuard v%s - Real-Time Deterministic Latency Daemon  \n", PULSEGUARD_VERSION);
    printf("    Targeting: QNX Neutrino RTOS 8.0 & Autonomous Systems        \n");
    printf("=================================================================\n\n");

    const char *target_host = "192.168.56.1";
    int port = DEFAULT_TELEMETRY_PORT;
    if (argc > 1) {
        if (strchr(argv[1], '.')) {
            target_host = argv[1];
            if (argc > 2) port = atoi(argv[2]);
        } else {
            port = atoi(argv[1]);
            if (argc > 2) target_host = argv[2];
        }
    }

    if (pulseguard_init(port, true) != 0) {
        fprintf(stderr, "Failed to initialize PulseGuard daemon\n");
        return 1;
    }

    if (target_host && strlen(target_host) > 0) {
        pulseguard_set_target(target_host, port);
        printf("[PulseGuard] Telemetry broadcast destination: %s:%d\n", target_host, port);
    }

    /* Register automotive-grade real-time tasks */
    /* 1. Brake-By-Wire: ASIL-D, Priority 63 (High), 1000 µs (1 ms) hard deadline */
    int t1_id = pulseguard_register_task("BrakeByWire_Actuator", CRITICALITY_ASIL_D, 63, 1000000ULL);

    /* 2. Steering Feedback: ASIL-B, Priority 50, 5000 µs (5 ms) */
    int t2_id = pulseguard_register_task("SteeringAngle_Sensor", CRITICALITY_ASIL_B, 50, 5000000ULL);

    /* 3. Telematics & Logging: Best Effort, Priority 15, 20000 µs (20 ms) */
    int t3_id = pulseguard_register_task("Vehicle_Telematics_GW", CRITICALITY_BEST_EFFORT, 15, 20000000ULL);

    /* Launch worker threads */
    pthread_t thread1, thread2, thread3;
    WorkerContext ctx1 = {t1_id, 1000000ULL, 63, CRITICALITY_ASIL_D, &g_app_running};
    WorkerContext ctx2 = {t2_id, 5000000ULL, 50, CRITICALITY_ASIL_B, &g_app_running};
    WorkerContext ctx3 = {t3_id, 20000000ULL, 15, CRITICALITY_BEST_EFFORT, &g_app_running};

    pthread_create(&thread1, NULL, ecu_worker_thread, &ctx1);
    pthread_create(&thread2, NULL, ecu_worker_thread, &ctx2);
    pthread_create(&thread3, NULL, ecu_worker_thread, &ctx3);

    printf("\n[PulseGuard] All Real-Time Tasks dispatched. Broadcasting telemetry...\n");
    printf("[PulseGuard] Press Ctrl+C or run host_tooling/dashboard.py to inspect live!\n\n");

    PulseGuardFrame frame;
    memset(&frame, 0, sizeof(frame));
    memcpy(frame.magic, "QPUL", 4);
    frame.active_tasks_count = 3;

    /* Telemetry broadcast loop (10 Hz) */
    uint32_t seq = 0;
    while (g_app_running && seq < 1200) { /* Run or until interrupted */
        sleep_ms(100);
        seq++;
        frame.sequence_num = seq;
        frame.timestamp_ns = pulseguard_get_time_ns();
        pulseguard_emit_telemetry(&frame);

        if (seq % 20 == 0) {
            printf("[PulseGuard Live] Frame #%04d | Telemetry Active | QNX Microkernel OK\n", seq);
        }
    }

    g_app_running = false;
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    pthread_join(thread3, NULL);
    pulseguard_shutdown();

    return 0;
}
