# QNX 8.0 Native API Evidence & Implementation Trace

This document demonstrates genuine QNX Neutrino RTOS 8.0 microkernel architecture integration within the **QNX PulseGuard** codebase.

---

## 1. High-Resolution Hardware Cycle Timestamping

Real-time deterministic jitter profiling requires sub-microsecond precision. QNX PulseGuard accesses target CPU cycle counters directly rather than incurring POSIX user-space context switch penalties.

### `ClockCycles()` & `SYSPAGE_ENTRY(qtime)`
- **Header:** `<sys/neutrino.h>`, `<sys/syspage.h>`
- **Source File:** [`src/qnx_pulseguard.c`](file:///C:/Users/DELL/.gemini/antigravity/scratch/qnx-pulseguard/src/qnx_pulseguard.c)
- **Code:**
```c
#if defined(__QNXNTO__)
    qtime_entry_t *qtime = SYSPAGE_ENTRY(qtime);
    cycles_per_sec = qtime->cycles_per_sec;
    uint64_t start = ClockCycles();
    /* ... task execution ... */
    uint64_t end = ClockCycles();
    double latency_us = (double)(end - start) * 1000000.0 / (double)cycles_per_sec;
#endif
```
- **Technical Explanation:**
  `ClockCycles()` is an inline assembly macro reading the target CPU hardware counter (e.g., `CNTVCT_EL0` on ARMv8/ARM64, or `RDTSC` on x86_64). `SYSPAGE_ENTRY(qtime)` queries the QNX microkernel system page to obtain the calibrated hardware frequency without system call overhead.

---

## 2. Microkernel Inter-Process Communication (IPC) Architecture

QNX is built on a synchronous message-passing microkernel. IPC performance is the foundation of all deterministic system services.

### `ChannelCreate()` & `ConnectAttach()`
- **Header:** `<sys/neutrino.h>`, `<sys/netmgr.h>`
- **Source File:** [`src/qnx_ipc_bench.c`](file:///C:/Users/DELL/.gemini/antigravity/scratch/qnx-pulseguard/src/qnx_ipc_bench.c)
- **Code:**
```c
int chid = ChannelCreate(0);
int coid = ConnectAttach(ND_LOCAL_NODE, 0, chid, _NTO_SIDE_CHANNEL, 0);
```
- **Technical Explanation:**
  Creates a kernel message channel in the receiver thread. `ConnectAttach` establishes a connection handle bound to the local node (`ND_LOCAL_NODE`) without crossing network boundaries, enabling microkernel-mediated memory copy.

### `MsgSend()`, `MsgReceive()`, & `MsgReply()`
- **Header:** `<sys/neutrino.h>`
- **Source File:** [`src/qnx_ipc_bench.c`](file:///C:/Users/DELL/.gemini/antigravity/scratch/qnx-pulseguard/src/qnx_ipc_bench.c)
- **Code:**
```c
// Client Thread:
MsgSend(coid, tx_buf, payload_size, rx_buf, payload_size);

// Server Thread:
int rcvid = MsgReceive(chid, server_rx, sizeof(server_rx), &info);
MsgReply(rcvid, EOK, server_tx, payload_size);
```
- **Technical Explanation:**
  Measures pure synchronous round-trip latency. `MsgSend` transitions the client thread into `SEND_BLOCKED` state until the server executes `MsgReceive` (`REPLY_BLOCKED`), and subsequently resumes when `MsgReply` completes.

---

## 3. Deterministic Real-Time Priority Scheduling

Automotive safety workloads require strict priority preemption without priority inversion.

### `pthread_setschedparam()` with `SCHED_FIFO`
- **Header:** `<pthread.h>`, `<sched.h>`
- **Source File:** [`src/main.c`](file:///C:/Users/DELL/.gemini/antigravity/scratch/qnx-pulseguard/src/main.c)
- **Code:**
```c
struct sched_param param;
param.sched_priority = 63; // Highest priority for ASIL-D BrakeByWire
pthread_setschedparam(pthread_self(), SCHED_FIFO, &param);
```
- **Priority Architecture:**
  - `BrakeByWire_Actuator`: Priority **63** (Critical Safety, 1ms period)
  - `SteeringAngle_Sensor`: Priority **50** (Medium Safety, 5ms period)
  - `Vehicle_Telematics_GW`: Priority **15** (Best Effort, 20ms period)

When CPU contention occurs, `SCHED_FIFO` guarantees that Priority 63 threads immediately preempt lower priority tasks, maintaining determinism.
