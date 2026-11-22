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
|  2:10 - 2:35   |  Simulated Contention & Dashboard Timing Exploration                 |
|  2:35 - 2:45   |  Audit Report Export & Conclusion                                     |
+---------------------------------------------------------------------------------------+
```

---

### Segment 1: Introduction & Problem Statement (0:00 – 0:25 | 25s)
- **Visual:** Split screen or title card: "QNX PulseGuard — Real-Time Latency & Timing Analysis Tool for QNX SDP 8.0". Transition to live dashboard.
- **Narration:**
  > "In embedded and automotive software systems, understanding real-time timing behavior is essential. On modern platforms running QNX SDP 8.0, real-time tasks like Brake-by-Wire models share compute resources with background telematics. Monitoring execution latencies, timing variations, and deadline margins during development is crucial. This is **QNX PulseGuard**."

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
  > This exercises QNX's core message-passing primitives: `ChannelCreate`, `ConnectAttach`, and synchronous `MsgSend`, `MsgReceive`, and `MsgReply` across 10,000 iterations per payload tier.
  > Running inside our QNX 8.0 VirtualBox environment, 64-byte messages achieve minimum round-trip latencies of approximately 1.49 microseconds, a 95th-percentile of 3.72 microseconds, and sustained throughput exceeding 189,000 round-trips per second. This provides empirical baseline measurements of synchronous microkernel message passing."

---

### Segment 4: Live Telemetry & Real-Time Dashboard (1:30 – 2:10 | 40s)
- **Visual:** Switch to terminal, run `/tmp/qnx_pulseguard 192.168.56.1 9876`. Immediately switch to the browser (`http://localhost:8080`). Show the status pill transition from `DEMO MODE` to **`REAL QNX MODE (Live Target [192.168.56.102])`**. Point out the waterfall charts updating at 10 Hz.
- **Narration:**
  > "Now, we launch the native PulseGuard telemetry daemon on QNX. It broadcasts binary UDP frames at 10 Hz directly to our host workstation.
  > Notice the dashboard immediately recognizes the incoming hardware stream and switches to **REAL QNX MODE**.
  > Using zero-syscall timestamping with `ClockCycles()` and `SYSPAGE_ENTRY(qtime)`, PulseGuard monitors three reference automotive execution loops under `SCHED_FIFO` priorities: BrakeByWire reference at priority 63, SteeringAngle reference at priority 50, and Telematics reference at priority 15. All three execute nominally within their allocated period budgets."

---

### Segment 5: Simulated Contention & Dashboard Timing Exploration (2:10 – 2:35 | 25s)
- **Visual:** Click the **Simulate CPU Contention** button. Show state update to `Simulated Contention Active`. Point out the simulated latency increase on the best-effort telematics reference task while the high-priority reference tasks remain within their period deadlines.
- **Narration:**
  > "For exploring developer tooling workflows, the dashboard also includes a simulated contention toggle. 
  > Clicking 'Simulate CPU Contention' allows developers to test how the UI handles timing variances and deadline monitoring during heavy workloads.
  > Notice the status updates to 'Simulated Contention Active', and the event log tracks the simulated scenario—illustrating how host tooling can visualize priority headroom."

---

### Segment 6: Timing Evidence Report & Conclusion (2:35 – 2:45 | 10s)
- **Visual:** Click the **Reports** tab, click **Export Timing Evidence Report** (JSON downloads). Show GitHub repository tab (`https://github.com/Harishrawal2/qnx-pulseguard`).
- **Narration:**
  > "Finally, developers can export complete timing evidence reports in JSON and CSV for automated CI/CD regression tracking. 
  > QNX PulseGuard: real-time timing visibility and host tooling for QNX SDP 8.0."

---

## Recording Tips for Compliance:
1. **Timer Check:** Use a phone stopwatch while recording to ensure the final edit finishes between **2:40 and 2:50**. Do NOT exceed 2:59.
2. **Resolution:** Record at 1080p (1920x1080) at 30 or 60 fps.
3. **Visibility:** When publishing on YouTube, set Visibility to **Public** (not Unlisted, not Private).
4. **Description:** Include the GitHub link and the official engineering scope disclaimer in the video description:
   ```text
   QNX PulseGuard — Real-Time Deterministic Latency & Timing Analysis Tool for QNX SDP 8.0
   BlackBerry QNX Global Hackathon 2026 Submission (Developer Tooling & Host Tools Track)

   GitHub Repository: https://github.com/Harishrawal2/qnx-pulseguard

   Disclaimer: This demonstration uses reference automotive workloads to analyze timing behavior. Our native IPC benchmark runs on QNX Neutrino 8.0 inside VirtualBox and measures synchronous message-passing latency. The dashboard also supports simulated CPU contention for exploring timing scenarios. These results are development benchmarks, not certified automotive safety guarantees.
   ```
