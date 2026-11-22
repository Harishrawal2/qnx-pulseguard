# QNX PulseGuard — Video Demonstration Script (Strictly Under 3 Minutes)

> [!IMPORTANT]
> **Official Hackathon Submission Rule:**  
> The video demonstration **must be uploaded to YouTube with Public visibility** and must be **strictly under 3:00 minutes** in total length. Unlisted or private YouTube links, Vimeo, Google Drive, or external video players do not qualify under official submission criteria.

**Target Duration:** 2 minutes 45 seconds (Buffer: 15 seconds safely under the 3:00 hard ceiling)  
**Platform:** Public YouTube Video  
**Target Category:** Developer Tooling & Host Tools  
**Target Platform:** BlackBerry QNX Software Development Platform (SDP) 8.0  

---

## Shot-by-Shot Demonstration Timeline

```
+---------------------------------------------------------------------------------------+
|  0:00 - 0:25   |  Introduction & Autonomous ECU Timing Problem                        |
|  0:25 - 0:55   |  QNX SDP 8.0 Toolchain & Target VM Boot (`mkqnximage` / VirtualBox)  |
|  0:55 - 1:30   |  Native Synchronous IPC Benchmark (`MsgSend` / `MsgReceive`)          |
|  1:30 - 2:10   |  Live UDP Telemetry Stream & Real-Time Dashboard (`REAL QNX MODE`)    |
|  2:10 - 2:35   |  Simulate CPU Contention & `SCHED_FIFO` Priority Preemption          |
|  2:35 - 2:45   |  Audit Report Export & Conclusion                                     |
+---------------------------------------------------------------------------------------+
```

---

### Segment 1: Introduction & Problem Statement (0:00 – 0:25 | 25s)
- **Visual:** Split screen or title card: "QNX PulseGuard — Real-Time Deterministic Latency & Timing Analysis Tool for QNX SDP 8.0". Transition to live dashboard.
- **Narration:**
  > "In autonomous driving and automotive control systems, deterministic timing is safety-critical. On modern multicore platforms running QNX SDP 8.0, safety-critical tasks like Brake-by-Wire share hardware resources with background telematics. Proving microsecond determinism and catching deadline breaches before deployment is essential. This is **QNX PulseGuard**."

---

### Segment 2: QNX SDP 8.0 Toolchain & Target VM Boot (0:25 – 0:55 | 30s)
- **Visual:** Open terminal showing `qcc -V` and the QNX VirtualBox VM booting over SSH. Show `uname -a` returning `QNX noname 8.0.0 2026/10/30-09:10:05EDT x86pc x86_64`.
- **Narration:**
  > "PulseGuard is built natively for QNX SDP 8.0. Here in the terminal, our native C components are cross-compiled using `qcc` with GCC 12.2.0. Using QNX's official `mkqnximage` utility, we deployed a bootable x86_64 target image running live inside Oracle VM VirtualBox. Connected over host-only networking at 192.168.56.102, we have a running QNX 8.0 microkernel."

---

### Segment 3: Native Synchronous IPC Benchmark (0:55 – 1:30 | 35s)
- **Visual:** In the SSH terminal, execute `/tmp/qnx_ipc_bench`. Highlight the formatted output table displaying 64B to 16KB results.
- **Narration:**
  > "Next, let's run our native synchronous IPC benchmark directly on the QNX target. 
  > This exercises QNX's core message-passing primitives: `ChannelCreate`, `ConnectAttach`, and synchronous `MsgSend`, `MsgReceive`, and `MsgReply` across 10,000 iterations per payload.
  > On 64-byte automotive control messages, we observe minimum round-trip latencies of just 1.48 microseconds, a 95th-percentile of 3.78 microseconds, and peak throughput exceeding 204,000 round-trips per second. This demonstrates the ultra-low latency of the QNX microkernel."

---

### Segment 4: Live Telemetry & Real-Time Dashboard (1:30 – 2:10 | 40s)
- **Visual:** Switch to terminal, run `/tmp/qnx_pulseguard 192.168.56.1 9876`. Immediately switch to the browser (`http://localhost:8080`). Show the status pill transition from `DEMO MODE` to **`REAL QNX MODE (Live Target [192.168.56.102])`**. Point out the waterfall charts updating at 10 Hz.
- **Narration:**
  > "Now, we launch the native PulseGuard telemetry daemon on QNX. It broadcasts binary UDP frames at 10 Hz directly to our host workstation.
  > Notice the dashboard immediately recognizes the incoming hardware stream and switches to **REAL QNX MODE**.
  > Using zero-syscall timestamping with `ClockCycles()` and `SYSPAGE_ENTRY(qtime)`, PulseGuard monitors three real-time automotive tasks under `SCHED_FIFO` priorities: BrakeByWire at priority 63, SteeringAngle at priority 50, and Telematics at priority 15. All three run nominally within their execution budgets."

---

### Segment 5: CPU Contention & Priority Preemption (2:10 – 2:35 | 25s)
- **Visual:** Click the **Simulate CPU Contention** button. Show state flip to `Contention Active`. Point out the slight jitter on the lowest-priority telematics task while BrakeByWire stays rock-solid.
- **Narration:**
  > "Let's inject synthetic CPU contention. Notice that our best-effort Telematics task experiences slight latency increases due to resource contention. 
  > But our highest-priority safety task—BrakeByWire at priority 63—maintains zero deadline breaches and steady sub-microsecond determinism. This proves `SCHED_FIFO` priority preemption in action."

---

### Segment 6: Timing Evidence Report & Conclusion (2:35 – 2:45 | 10s)
- **Visual:** Click the **Reports** tab, click **Export Timing Evidence Report** (JSON downloads). Show GitHub repository tab (`https://github.com/Harishrawal2/qnx-pulseguard`).
- **Narration:**
  > "Finally, developers can export complete timing evidence reports in JSON and CSV for automated CI/CD safety verification. 
  > QNX PulseGuard: deterministic timing visibility for the next generation of software-defined vehicles on QNX SDP 8.0."

---

## Recording Tips for Compliance:
1. **Timer Check:** Use a phone stopwatch while recording to ensure the final edit finishes between **2:40 and 2:50**. Do NOT exceed 2:59.
2. **Resolution:** Record at 1080p (1920x1080) at 30 or 60 fps.
3. **Visibility:** When publishing on YouTube, set Visibility to **Public** (not Unlisted, not Private).
4. **Description:** Include the GitHub link in the video description: `https://github.com/Harishrawal2/qnx-pulseguard`.
