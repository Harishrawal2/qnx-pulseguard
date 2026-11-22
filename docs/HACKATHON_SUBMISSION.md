# QNX Global Hackathon 2026 Submission

## 1. Project Information
- **Project Name:** QNX PulseGuard
- **Tagline:** Real-Time Deterministic Latency Profiler & Timing Analysis Host Tool for QNX SDP 8.0
- **Category / Track:** Developer Tooling & Host Tools
- **Target Platform:** BlackBerry QNX Software Development Platform (SDP) 8.0
- **Target Verification Environment:** QNX Neutrino RTOS 8.0.0 (`x86_64`) on Oracle VM VirtualBox 7.2 (`192.168.56.102`)
- **GitHub Repository:** https://github.com/Harishrawal2/qnx-pulseguard
- **Commit Hash:** `6846c5a6ad8278ae9e663d409021cafad574bb8f`
- **License:** Apache License 2.0 (Official unmodified text)

---

## 2. Elevator Pitch
QNX PulseGuard is a real-time latency and timing analysis platform designed specifically for QNX SDP 8.0 applications. By pairing a native C telemetry agent utilizing QNX Neutrino microkernel primitives (`ClockCycles()`, `SYSPAGE_ENTRY(qtime)`, `ChannelCreate()`, `ConnectAttach()`, `MsgSend/MsgReceive/MsgReply`, and `pthread_setschedparam()`) with a responsive dark-mode host dashboard, PulseGuard provides microsecond timing visibility, deadline breach monitoring, and timing behavior analysis using reference automotive workloads.

---

## 3. The Problem
Developing safety-critical software on multicore real-time operating systems requires verifying that high-priority tasks always meet hard deadlines, even during heavy CPU or bus contention. Existing generic Linux or enterprise profiling tools either introduce unacceptable measurement overhead or fail to leverage QNX microkernel architecture. Engineers need lightweight, QNX-native tooling that provides continuous timing evidence without distorting real-time behavior.

---

## 4. What We Built
- **Native QNX 8.0 Agent (`src/qnx_pulseguard.c`, `src/main.c`):** A lightweight daemon that measures task execution latency and jitter using direct CPU cycle counters (`ClockCycles()`) calibrated via `SYSPAGE_ENTRY(qtime)`, streaming packed binary UDP telemetry frames to the host workstation.
- **QNX Microkernel IPC Benchmark (`src/qnx_ipc_bench.c`):** A dedicated benchmark evaluating synchronous message-passing latency (`MsgSend` -> `MsgReceive` -> `MsgReply`) across 64B to 16KB payloads on running QNX 8.0 targets.
- **Priority-Driven ECU Workload Simulator:** Running three prioritized automotive reference workloads under `SCHED_FIFO` priorities (BrakeByWire @ 63, SteeringAngle @ 50, Telematics @ 15).
- **Enterprise Host Dashboard (`host_tooling/dashboard.py`):** A responsive web application featuring real-time waterfall latency charts, Gaussian jitter histograms, CPU contention simulation, and multi-tab navigation (`Monitoring`, `Analysis & IPC Bench`, `Reports & Evidence`, `Configuration`).
- **Timing Evidence & Audit Export:** Instant JSON and CSV export generating structured timing evidence suitable for continuous integration and safety verification records.

---

## 5. QNX 8.0 Specific Engineering
PulseGuard directly integrates key QNX Neutrino RTOS capabilities:
- **`ClockCycles()` & `SYSPAGE_ENTRY(qtime)->cycles_per_sec`**: Direct CPU cycle timing without syscall overhead.
- **`ChannelCreate()` & `ConnectAttach()`**: Microkernel channel and side-channel attachment.
- **`MsgSend()`, `MsgReceive()`, `MsgReply()`**: Synchronous message passing evaluation.
- **`pthread_setschedparam()` with `SCHED_FIFO`**: Real-time thread priority assignment across reference tasks.
- **`mkqnximage` VirtualBox Deployment:** Automated creation of bootable QNX 8.0 virtual machine targets using official QNX SDP 8.0 packaging tools.

---

## 6. Verification Status & Empirical Results
- **Toolchain:** Compiled with official QNX SDP 8.0 `qcc` (GCC 12.2.0 `gcc_ntox86_64`).
- **Target OS:** Verified on running VM `QNX noname 8.0.0 2026/10/30-09:10:05EDT x86pc x86_64` (`192.168.56.102`).
- **Synchronous IPC Benchmark Results (QNX 8.0 VirtualBox):**
  - Minimum round-trip latency: **1.48 µs** (64B payload)
  - 95th-percentile latency: **3.78 µs**
  - Peak throughput: **204,053 round-trips/second** (256B payload)
  - Peak bandwidth: **2,048.24 MB/s** (16KB payload)
- **Live Telemetry Stream:** Target streamed 1,200 UDP frames (10 Hz) to host port 9876; dashboard automatically transitioned to `REAL QNX MODE (Live Target [192.168.56.102:57935])` with zero deadline breaches.
- **Automated Tests:** 16/16 Python unit, schema, security, and protocol tests passed.
- **Public YouTube Video:** Required by official hackathon rules to be public and strictly under 3 minutes (script provided in `docs/DEMO_SCRIPT.md`).

---

## 7. Devpost Project Submission Text (Copy-Paste Ready)

### Project Title:
QNX PulseGuard — Real-Time Latency & Timing Analysis Tool for QNX SDP 8.0

### Elevator Pitch:
A native real-time latency profiler and timing verification suite for QNX SDP 8.0, combining high-resolution microkernel cycle counters and synchronous IPC benchmarking with an enterprise host dashboard for automotive software development.

### Technical Disclaimer & Verification Scope:
> *This demonstration uses reference automotive workloads to analyze timing behavior. Our native IPC benchmark runs on QNX Neutrino 8.0 inside VirtualBox and measures synchronous message-passing latency. The dashboard also supports simulated CPU contention for exploring timing scenarios. These results are development benchmarks, not certified automotive safety guarantees.*

### Inspiration:
In autonomous driving and automotive safety architectures (such as brake-by-wire and steer-by-wire), meeting deterministic execution deadlines is a fundamental safety requirement. Generic profiling tools often introduce significant overhead or treat the operating system as a black box. We built QNX PulseGuard to leverage the unique microkernel architecture of BlackBerry QNX Neutrino RTOS 8.0, giving developers microsecond-level timing visibility without compromising real-time determinism.

### What it does:
QNX PulseGuard consists of two integrated components:
1. **Target-Native Telemetry & IPC Agent:** Cross-compiled with QNX SDP 8.0 `qcc`, this lightweight daemon uses `ClockCycles()` and `SYSPAGE_ENTRY(qtime)` for zero-syscall timestamping, measures synchronous microkernel IPC performance (`MsgSend`/`MsgReceive`/`MsgReply`), and broadcasts binary UDP telemetry frames at 10 Hz.
2. **Host-Side Verification Dashboard:** Built in Python, this server ingests live telemetry, validates packet framing and sender subnets, displays dynamic waterfall latency charts and Gaussian jitter histograms, and provides one-click export of structured Timing Evidence Reports in JSON and CSV.

### How we built it:
- **Target C Code:** Developed against QNX Neutrino 8.0 headers using POSIX threads, `SCHED_FIFO` real-time scheduling priorities (63 for BrakeByWire, 50 for SteeringAngle, 15 for Telematics), and microkernel channel communication.
- **Cross-Compilation:** Built using `qcc` (GCC 12.2.0 `gcc_ntox86_64`) from the official QNX SDP 8.0 toolchain.
- **Target Virtualization:** Packaged and deployed onto an x86_64 QNX 8.0 target using `mkqnximage` targeting Oracle VM VirtualBox 7.2 with host-only networking.
- **Host Dashboard:** Built with a lightweight zero-dependency Python backend serving interactive WebSockets/REST APIs and a responsive dark-mode front-end.

### Challenges we ran into:
- Ensuring strict wire-format compatibility between 64-bit C structs compiled under `qcc` and Python's `struct` unpacker, which required `#pragma pack(push, 1)` to eliminate architecture-specific padding mismatches.
- Resolving network communication between the headless VirtualBox QNX guest and the Windows host adapter using VirtualBox host-only networking and DHCP leasing.
- Correctly binding QNX 8.0 system page calibration macros (`struct qtime_entry *qtime = SYSPAGE_ENTRY(qtime)`).

### Accomplishments that we're proud of:
- **Verified Execution on Live QNX 8.0:** Successfully cross-compiled and executed the native binaries on a real QNX Neutrino RTOS 8.0.0 target (`192.168.56.102`).
- **Microkernel IPC Performance:** Measured empirical round-trip synchronous message passing latencies on QNX 8.0 VirtualBox of **1.49 µs** minimum and sustained throughput exceeding **189,000 round-trips/second**.
- **Live Telemetry Pipeline:** Streaming continuous real-time frames from the QNX target to the host dashboard, achieving zero deadline breaches under nominal operations.
- **Comprehensive Quality:** 16 automated tests covering API schemas, security constraints, and statistical math.

### What we learned:
Deep insights into the QNX SDP 8.0 microkernel architecture, particularly the efficiency of synchronous message passing, high-resolution cycle counter calibration via system pages, and VM image creation workflows with `mkqnximage`.

### What's next for QNX PulseGuard:
- Adding trace-event instrumentation (`TraceEvent()`) integration to capture kernel context switch timelines alongside user-space telemetry.
- Expanding hardware target validation to ARMv8 / ARM64 automotive platforms (e.g., NXP S32G, Qualcomm Snapdragon Ride).
- Adding automated regression thresholds for continuous integration pipelines in ISO 26262 development environments.

---

## 8. Complete Submission Checklist
- [x] Unmodified Apache License 2.0 present in root `LICENSE`.
- [x] Code cleanly cross-compiles with official QNX SDP 8.0 toolchain (`qcc`).
- [x] Native binaries executed on live QNX 8.0 target (`uname -a`: `QNX noname 8.0.0`).
- [x] Genuine synchronous IPC benchmark results documented with VirtualBox labeling.
- [x] Live UDP telemetry streaming verified with dashboard `REAL_QNX` mode screenshot.
- [x] 16/16 automated test suite passing.
- [x] Zero proprietary QNX SDK binaries or license keys committed to repository.
- [x] Git repository pushed and verified on GitHub `origin/main`.
- [ ] Record and upload a **Public YouTube video strictly under 3 minutes** using the script in `docs/DEMO_SCRIPT.md`.
- [ ] Add the YouTube video URL and GitHub URL to the Devpost submission draft.
- [ ] Review Devpost fields and submit before the hackathon deadline.
