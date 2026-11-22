/*
 * QNX PulseGuard - Core Real-Time Daemon
 * Demonstrates QNX Neutrino RTOS 8.0 Architecture:
 * - ChannelCreate() & ConnectAttach() IPC
 * - High-resolution ClockCycles() / clock_gettime(CLOCK_MONOTONIC)
 * - Microkernel Real-time Scheduling (SCHED_FIFO, Priority Preemption)
 * - Microsecond Jitter Analysis for SDV / Medical systems
 */

#include "qnx_pulseguard.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef int socklen_t;
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <sched.h>
#endif

static int g_telemetry_socket = -1;
static struct sockaddr_in g_broadcast_addr;
static PulseGuardFrame g_current_frame;
static pthread_mutex_t g_state_mutex = PTHREAD_MUTEX_INITIALIZER;
static bool g_running = false;
static int g_channel_id = -1;
static int g_connection_id = -1;

/* High-precision timestamp in nanoseconds */
uint64_t pulseguard_get_time_ns(void) {
#if QNX_NATIVE
    /* Under QNX Neutrino, ClockCycles() yields CPU clock cycle counter */
    static uint64_t cps = 0;
    if (cps == 0) {
        cps = SYSPAGE_ENTRY(qtime)->cycles_per_sec;
    }
    uint64_t cycles = ClockCycles();
    return (cycles * 1000000000ULL) / cps;
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ((uint64_t)ts.tv_sec * 1000000000ULL) + ts.tv_nsec;
#endif
}

int pulseguard_init(int telemetry_port, bool verbose) {
    memset(&g_current_frame, 0, sizeof(PulseGuardFrame));
    memcpy(g_current_frame.magic, "QPUL", 4);
    g_current_frame.sequence_num = 0;
    g_running = true;

#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        fprintf(stderr, "[PulseGuard] WSAStartup failed\n");
        return -1;
    }
#endif

    /* Setup UDP telemetry broadcast socket */
    g_telemetry_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (g_telemetry_socket < 0) {
        perror("[PulseGuard] Failed to create telemetry socket");
        return -1;
    }

    int broadcast_enable = 1;
    setsockopt(g_telemetry_socket, SOL_SOCKET, SO_BROADCAST, 
               (const char*)&broadcast_enable, sizeof(broadcast_enable));

    memset(&g_broadcast_addr, 0, sizeof(g_broadcast_addr));
    g_broadcast_addr.sin_family = AF_INET;
    g_broadcast_addr.sin_port = htons(telemetry_port);
    const char *env_host = getenv("PULSEGUARD_HOST");
    if (env_host && strlen(env_host) > 0) {
        g_broadcast_addr.sin_addr.s_addr = inet_addr(env_host);
    } else {
        g_broadcast_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); /* Send to localhost or subnet */
    }

#if QNX_NATIVE
    /* Create QNX Neutrino Synchronous IPC Channel */
    g_channel_id = ChannelCreate(0);
    if (g_channel_id < 0) {
        perror("[PulseGuard] QNX ChannelCreate failed");
        return -1;
    }
    g_connection_id = ConnectAttach(ND_LOCAL_NODE, 0, g_channel_id, _NTO_SIDE_CHANNEL, 0);
    if (g_connection_id < 0) {
        perror("[PulseGuard] QNX ConnectAttach failed");
        return -1;
    }
    printf("[PulseGuard] QNX 8.0 Kernel IPC Channel Initialized (ChID: %d, CoID: %d)\n", 
           g_channel_id, g_connection_id);
#else
    printf("[PulseGuard] Host Simulation Environment Initialized (QNX Fallback Layer Active)\n");
#endif

    if (verbose) {
        printf("[PulseGuard] Version %s ready. Telemetry listening on UDP port %d\n", 
               PULSEGUARD_VERSION, telemetry_port);
    }
    return 0;
}

int pulseguard_register_task(const char *name, CriticalityLevel level, int priority, uint64_t period_ns) {
    pthread_mutex_lock(&g_state_mutex);
    if (g_current_frame.active_tasks_count >= MAX_REGISTERED_TASKS) {
        pthread_mutex_unlock(&g_state_mutex);
        return -1;
    }

    int idx = g_current_frame.active_tasks_count;
    TaskTelemetry *t = &g_current_frame.tasks[idx];
    strncpy(t->task_name, name, MAX_TASK_NAME_LEN - 1);
    t->task_id = idx + 1;
    t->level = level;
    t->priority = priority;
    t->target_period_ns = period_ns;
    t->best_latency_ns = (uint64_t)-1;
    t->worst_latency_ns = 0;
    t->avg_latency_ns = 0;
    t->deadline_misses = 0;
    t->total_executions = 0;
    t->is_healthy = true;

    g_current_frame.active_tasks_count++;
    pthread_mutex_unlock(&g_state_mutex);

    printf("[PulseGuard] Registered Task #%d '%s' [Priority: %d, ASIL: %d, Period: %.2f ms]\n",
           t->task_id, t->task_name, t->priority, t->level, (double)period_ns / 1000000.0);
    return t->task_id;
}

void pulseguard_record_execution(int task_id, uint64_t execution_latency_ns, bool missed_deadline) {
    pthread_mutex_lock(&g_state_mutex);
    int idx = task_id - 1;
    if (idx < 0 || idx >= (int)g_current_frame.active_tasks_count) {
        pthread_mutex_unlock(&g_state_mutex);
        return;
    }

    TaskTelemetry *t = &g_current_frame.tasks[idx];
    t->total_executions++;
    if (t->total_executions == 1) {
        t->worst_latency_ns = execution_latency_ns;
        t->best_latency_ns = execution_latency_ns;
        t->avg_latency_ns = execution_latency_ns;
    } else {
        if (execution_latency_ns > t->worst_latency_ns) {
            t->worst_latency_ns = execution_latency_ns;
        }
        if (execution_latency_ns < t->best_latency_ns) {
            t->best_latency_ns = execution_latency_ns;
        }
        /* Exponential Moving Average for jitter smoothing */
        t->avg_latency_ns = (uint64_t)(0.9 * (double)t->avg_latency_ns + 0.1 * (double)execution_latency_ns);
    }

    if (missed_deadline) {
        t->deadline_misses++;
        g_current_frame.total_deadline_misses++;
        if (t->level == CRITICALITY_ASIL_D) {
            g_current_frame.failsafe_triggered = true;
            fprintf(stderr, "[CRITICAL ALERT] ASIL-D Task '%s' breached deadline! Latency: %llu ns (Target: %llu ns)\n",
                    t->task_name, (unsigned long long)execution_latency_ns, (unsigned long long)t->target_period_ns);
        }
    }

    t->is_healthy = (t->deadline_misses == 0) || ((double)t->deadline_misses / t->total_executions < 0.005);
    pthread_mutex_unlock(&g_state_mutex);
}

int pulseguard_set_target(const char *target_ip, int port) {
    if (target_ip && strlen(target_ip) > 0) {
        g_broadcast_addr.sin_addr.s_addr = inet_addr(target_ip);
    }
    if (port > 0) {
        g_broadcast_addr.sin_port = htons(port);
    }
    return 0;
}

int pulseguard_emit_telemetry(const PulseGuardFrame *frame) {
    if (g_telemetry_socket < 0) return -1;
    PulseGuardFrame local_frame;
    pthread_mutex_lock(&g_state_mutex);
    if (frame == NULL) {
        local_frame = g_current_frame;
        local_frame.sequence_num++;
        local_frame.timestamp_ns = pulseguard_get_time_ns();
        g_current_frame.sequence_num = local_frame.sequence_num;
    } else {
        local_frame = *frame;
        for (uint32_t i = 0; i < g_current_frame.active_tasks_count; ++i) {
            local_frame.tasks[i] = g_current_frame.tasks[i];
        }
        local_frame.active_tasks_count = g_current_frame.active_tasks_count;
        local_frame.total_deadline_misses = g_current_frame.total_deadline_misses;
        local_frame.failsafe_triggered = g_current_frame.failsafe_triggered;
    }
    pthread_mutex_unlock(&g_state_mutex);

    size_t header_wire_size = 41; // 4 + 4 + 8 + 4 + 4 + 8 + 8 + 1
    size_t task_wire_size = 101;  // 32 + 4 + 4 + 4 + 8 + 8 + 8 + 8 + 8 + 8 + 8 + 1
    size_t send_size = header_wire_size + (local_frame.active_tasks_count * task_wire_size);

    ssize_t sent = sendto(g_telemetry_socket, (const char*)&local_frame, send_size, 0,
                          (struct sockaddr*)&g_broadcast_addr, sizeof(g_broadcast_addr));
    return (sent == (ssize_t)send_size) ? 0 : -1;
}

void pulseguard_shutdown(void) {
    g_running = false;
#if QNX_NATIVE
    if (g_connection_id >= 0) ConnectDetach(g_connection_id);
    if (g_channel_id >= 0) ChannelDestroy(g_channel_id);
#endif
    if (g_telemetry_socket >= 0) {
#ifdef _WIN32
        closesocket(g_telemetry_socket);
        WSACleanup();
#else
        close(g_telemetry_socket);
#endif
        g_telemetry_socket = -1;
    }
    printf("[PulseGuard] Telemetry engine successfully stopped.\n");
}
