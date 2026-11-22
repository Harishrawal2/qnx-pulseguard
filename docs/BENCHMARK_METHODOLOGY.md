# Real-Time Measurement & Benchmark Methodology

This document details the mathematical models, measurement techniques, and benchmark protocols implemented in **QNX PulseGuard**.

---

## 1. Latency & Jitter Measurement Principles

### Microsecond Timebase
Timing measurements utilize the target processor's invariant cycle counter via QNX Neutrino's `ClockCycles()` API:

$$\Delta t_{\mu s} = \frac{\text{ClockCycles}_{\text{end}} - \text{ClockCycles}_{\text{start}}}{\text{SYSPAGE\_ENTRY}(qtime)\to\text{cycles\_per\_sec}} \times 10^6$$

### Jitter Calculation
Timing jitter represents the deviation of actual execution intervals from the configured deterministic period:

$$Jitter_i = | \Delta t_i - \Delta t_{\text{nominal}} |$$

The dashboard calculates a normalized Gaussian distribution across discrete bins ($[0.8-1.0\mu s], [1.0-1.2\mu s], \dots, [>2.0\mu s]$) to assess system determinism under varying system loads.

### Percentile Latency Metrics ($P_{50}, P_{95}, P_{99}, P_{99.9}$)
Percentiles are evaluated over rolling sample windows ($N = 1000$):
- **$P_{50}$ (Median):** Typical nominal execution latency.
- **$P_{99.9}$:** Extreme tail latency critical for hard real-time automotive deadlines.
- **WCET (Worst-Case Execution Time):** The absolute maximum observed latency:
$$\text{WCET} = \max_{i \in [1, N]} (\Delta t_i)$$

---

## 2. Microkernel Synchronous IPC Benchmark

The QNX IPC benchmark (`src/qnx_ipc_bench.c`) evaluates the round-trip overhead of synchronous message passing:

1. A client thread issues `MsgSend(coid, tx, len, rx, len)`.
2. The QNX microkernel schedules the receiving server thread blocked on `MsgReceive(chid, ...)`.
3. The server processes the request and executes `MsgReply(rcvid, EOK, tx, len)`.
4. The client unblocks and records elapsed cycle delta.

### Benchmark Parameters:
- **Payload Sizes:** 64B, 256B, 1KB, 4KB, 16KB
- **Iteration Count:** 10,000 round-trips per payload size
- **Overhead Accounting:** Direct cycle subtraction accounts for timer reading overhead.

---

## 3. CPU Contention Stress Protocol

To evaluate real-time priority preemption determinism:
1. **Baseline Phase:** Workloads execute under nominal CPU load; timing characteristics are sampled for 10 seconds.
2. **Contention Phase:** Controlled synthetic CPU worker threads are spawned at priority 10 (below ECU safety tasks).
3. **Observation:** 
   - `BrakeByWire` (Priority 63) maintains identical execution bounds ($<20\mu s$) due to `SCHED_FIFO` preemption.
   - `Vehicle_Telematics_GW` (Priority 15) exhibits increased latency due to contention, verifying priority isolation.
4. **Recovery Phase:** Contention threads terminate, verifying timing return to nominal baselines.
