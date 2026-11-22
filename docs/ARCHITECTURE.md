# QNX PulseGuard: System Architecture

## 1. Architectural Philosophy

QNX PulseGuard is engineered as a zero-perturbation, real-time observability suite for applications executing on the **QNX Neutrino Real-Time Operating System (RTOS) 8.0**.

In real-time operating systems, standard Linux observability approaches (such as high-frequency kernel tracing, verbose logging, or dynamic tracing probes) can introduce unquantified scheduling jitter, polluting the very deterministic behavior they intend to measure. PulseGuard addresses this through a split architecture:

1. **In-Target QNX Real-Time Agent**:
   - Ultra-lightweight C daemon utilizing high-resolution hardware cycle registers (`ClockCycles()`).
   - Microkernel synchronous channel/pulse IPC primitives (`ChannelCreate()`, `ConnectAttach()`, `MsgReceivePulse()`).
   - Strict real-time thread priority scheduling (`SCHED_FIFO` with priority preemption).
   - Non-blocking UDP telemetry streaming over localhost or private network bridges.

2. **Out-of-Target Host Observability Service**:
   - High-throughput binary frame unpacker decoding raw QNX telemetry.
   - Circular historical buffer maintaining microsecond latency metrics and timestamped wall-clock progression.
   - REST and WebSocket interfaces serving real-time analytics to workstations.
   - ISO 26262 ASIL-D timing budget watchdog and statistical percentiles engine (P50, P95, P99, P99.9).

3. **Enterprise Web Telemetry Client**:
   - Hardware-calibrated multi-task latency waterfall display.
   - Gaussian jitter histogram visualizing deterministic cycle clustering.
   - Controlled workload contention benchmark trigger for preemption verification.
   - Synchronous QNX message-passing round-trip latency analyzer.

---

## 2. Component Diagram

```text
+-----------------------------------------------------------------------------------------+
|                               QNX NEUTRINO RTOS 8.0 TARGET                              |
|                                                                                         |
|  +-----------------------------+   +------------------------+   +--------------------+  |
|  | Task 1: BrakeByWire_Actuator|   | Task 2: SteeringAngle  |   | Task 3: Telematics |  |
|  | Priority 63 (SCHED_FIFO)    |   | Priority 50 (SCHED_FIFO|   | Priority 15 (FIFO) |  |
|  | Period: 1000 us (ASIL-D)    |   | Period: 5000 us(ASIL-B)|   | Period: 20000 us   |  |
|  +--------------+--------------+   +-----------+------------+   +---------+----------+  |
|                 |                              |                          |             |
|                 +------------------------------+--------------------------+             |
|                                                |                                        |
|                                                v                                        |
|                          +------------------------------------------+                   |
|                          |     QNX PulseGuard Real-Time Daemon      |                   |
|                          | - ClockCycles() hardware calibration     |                   |
|                          | - ChannelCreate(0) & side-channel IPC    |                   |
|                          | - Non-blocking UDP binary frame emitter  |                   |
|                          +---------------------+--------------------+                   |
|                                                |                                        |
+------------------------------------------------+----------------------------------------+
                                                 | UDP Port 9876
                                                 | (Binary Struct: QPUL)
                                                 v
+-----------------------------------------------------------------------------------------+
|                             HOST OBSERVABILITY SERVICE                                  |
|                                                                                         |
|  +-----------------------------------------------------------------------------------+  |
|  |                     UdpTelemetryListener (Socket Thread)                          |  |
|  +---------------------------------------------+-------------------------------------+  |
|                                                |                                        |
|                                                v                                        |
|  +-----------------------------------------------------------------------------------+  |
|  |                               TelemetryState Engine                              |  |
|  | - Transparent Mode Controller: [REAL QNX MODE] vs [DEMO MODE]                     |  |
|  | - 30-sample persistent circular buffer with wall-clock time alignment             |  |
|  | - Statistical Percentile Engine (P50, P95, P99, P99.9)                            |  |
|  | - Synchronous IPC Benchmark Evaluator (64B to 16KB payload sweeps)                |  |
|  +---------------------------------------------+-------------------------------------+  |
|                                                |                                        |
|                                                v                                        |
|  +-----------------------------------------------------------------------------------+  |
|  |                 REST API & Web Server (http://localhost:8080)                     |  |
|  |   /api/telemetry | /api/ipc_benchmark | /api/export_audit | /api/export_csv       |  |
|  +---------------------------------------------+-------------------------------------+  |
+------------------------------------------------+----------------------------------------+
                                                 | HTTP / JSON Stream
                                                 v
+-----------------------------------------------------------------------------------------+
|                               ENTERPRISE WEB CLIENT                                     |
|  - Real-time Task Latency Waterfall (0-150 us range)                                    |
|  - Gaussian Jitter Distribution Histogram (Nominal +/- 0.4 us)                          |
|  - Autonomous Vehicle Task Matrix (WCET, ACET, Deadlines, Headroom %)                   |
|  - QNX Neutrino Kernel Event Trace Stream (/dev/pulseguard)                             |
|  - Interactive "Simulate CPU Contention" Preemption Verification Trigger                |
+-----------------------------------------------------------------------------------------+
```

---

## 3. Telemetry Protocol Specification

The QNX native component communicates with host workstations using compact, non-blocking binary UDP frames. The packet consists of a fixed-size header followed by an array of task telemetry structures:

### Frame Header (`PulseGuardFrame`, 37 bytes)
```c
typedef struct {
    char magic[4];                 // "QPUL"
    uint32_t sequence_num;         // Monotonically increasing frame index
    uint64_t timestamp_ns;         // Nanoseconds timestamp via ClockCycles()
    uint32_t active_tasks_count;   // Number of serialized task records (1 to 16)
    uint32_t qnx_kernel_ticks;     // Total microkernel timer ticks observed
    uint64_t total_deadline_misses;// Global counter of task deadline breaches
    double system_jitter_us;       // Max timing jitter measured across cycle
    bool failsafe_triggered;       // True if an ASIL-D safety threshold was breached
} PulseGuardFrame;
```

### Task Record (`TaskTelemetry`, 101 bytes)
```c
typedef struct {
    char task_name[32];            // Null-terminated task identifier
    uint32_t task_id;              // Numerical task index
    int32_t level;                 // Safety grade: 0=Best-Effort, 1=ASIL-B, 2=ASIL-D
    uint32_t priority;             // QNX thread priority (1-255)
    uint64_t target_period_ns;     // Expected periodic loop interval (ns)
    uint64_t worst_latency_ns;     // Worst-Case Execution Time (WCET)
    uint64_t best_latency_ns;      // Best observed execution time
    uint64_t avg_latency_ns;       // Exponential moving average latency
    uint64_t deadline_misses;      // Count of execution times > target_period_ns
    uint64_t total_executions;     // Total iterations completed
    double cpu_utilization_pct;    // Thread CPU time allocation
    bool is_healthy;               // Deadline compliance status
} TaskTelemetry;
```

---

## 4. Measurement Precision & Overhead Analysis

1. **Timestamping Overhead**:
   - On x86_64, `ClockCycles()` translates directly to the `rdtsc` instruction (~15 to 25 CPU cycles, < 15 nanoseconds overhead).
   - On ARMv8 / aarch64, `ClockCycles()` accesses `CNTVCT_EL0` (~10 to 20 CPU cycles).
   - This eliminates the POSIX `clock_gettime()` syscall trap overhead.

2. **IPC Benchmark Methodology**:
   - The synchronous IPC benchmark (`src/qnx_ipc_bench.c`) measures full round-trip message passing:
     $$\Delta t_{\text{round-trip}} = t_{\text{reply\_received}} - t_{\text{send\_initiated}}$$
   - Measures true microkernel context-switch and copy overhead across message payload buffers ranging from 64 Bytes to 16 Kilobytes.
