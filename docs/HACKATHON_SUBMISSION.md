# QNX Global Hackathon 2026 Submission

## 1. Project Information
- **Project Name:** QNX PulseGuard
- **Tagline:** Real-Time Deterministic Latency Profiler & Timing Analysis Host Tool for QNX SDP 8.0
- **Category / Track:** Developer Tooling & Host Tools
- **Target Platform:** BlackBerry QNX Software Development Platform (SDP) 8.0
- **GitHub Repository:** https://github.com/Harishrawal2/qnx-pulseguard

---

## 2. Elevator Pitch
QNX PulseGuard is a real-time deterministic latency and jitter analysis platform designed specifically for QNX SDP 8.0 applications. By pairing a native C telemetry agent utilizing QNX Neutrino microkernel primitives (`ClockCycles()`, `SYSPAGE_ENTRY(qtime)`, `ChannelCreate()`, `MsgSend/MsgReceive/MsgReply`) with a modern dark-mode host dashboard, PulseGuard provides sub-microsecond timing visibility, deadline breach monitoring, and CPU contention analysis for mission-critical automotive workloads.

---

## 3. The Problem
Developing safety-critical software on multicore real-time operating systems requires verifying that high-priority tasks always meet hard deadlines, even during heavy CPU or bus contention. Existing generic Linux or enterprise profiling tools either introduce unacceptable measurement overhead or fail to leverage QNX microkernel architecture. Engineers need lightweight, QNX-native tooling that provides continuous timing evidence without distorting real-time behavior.

---

## 4. What We Built
- **Native QNX 8.0 Agent:** A lightweight daemon measuring task execution latency and jitter using direct CPU cycle counters (`ClockCycles()`) and QNX system page calibration.
- **QNX Microkernel IPC Benchmark:** A dedicated benchmark evaluating synchronous message-passing latency (`MsgSend` -> `MsgReceive` -> `MsgReply`) across 64B to 16KB payloads.
- **Priority-Driven ECU Workload Simulator:** A demonstration model running three simulated automotive workloads under `SCHED_FIFO` priorities (BrakeByWire @ 63, SteeringAngle @ 50, Telematics @ 15).
- **Enterprise Host Dashboard:** A responsive web application featuring real-time waterfall latency charts, Gaussian jitter histograms, CPU contention toggles, and multi-tab navigation (`Monitoring`, `Analysis`, `Reports`, `Configuration`).
- **Audit & Evidence Export:** Instant JSON and CSV export generating structured timing evidence suitable for continuous integration and safety verification records.

---

## 5. QNX 8.0 Specific Engineering
PulseGuard directly integrates key QNX Neutrino RTOS capabilities:
- **`ClockCycles()` & `SYSPAGE_ENTRY(qtime)->cycles_per_sec`**: Direct sub-microsecond CPU cycle timing without syscall overhead.
- **`ChannelCreate()` & `ConnectAttach()`**: Microkernel channel and side-channel attachment.
- **`MsgSend()`, `MsgReceive()`, `MsgReply()`**: Synchronous message passing evaluation.
- **`pthread_setschedparam()` with `SCHED_FIFO`**: Real-time priority preemption verification.

---

## 6. Verification Status & Transparent Engineering Claims
- **Host Tooling & Analysis Engine:** 100% verified. 16/16 unit and integration tests passing covering percentile mathematics, circular history buffer, UDP schema validation, and REST APIs.
- **Target Microkernel Primitives:** Native C source code implements genuine QNX 8.0 APIs (`ClockCycles()`, `SYSPAGE_ENTRY(qtime)`, `ChannelCreate()`, `MsgSend/MsgReceive/MsgReply`, `SCHED_FIFO`).
- **Target Runtime Execution:** Transparently marked as pending execution on a target machine with an active QNX SDP 8.0 license. Simulated demo datasets are strictly labeled as such to prevent confusing synthetic benchmarks with actual hardware execution logs.
- **Video Submission Requirement:** Video demonstration must be published as a **Public YouTube video under 3:00 minutes** (script provided in `docs/DEMO_SCRIPT.md`).

---

## 7. Submission Checklist Before Devpost Finalization
- [ ] Public YouTube video link (under 3 minutes) uploaded and added to Devpost.
- [ ] GitHub repository public: https://github.com/Harishrawal2/qnx-pulseguard
- [ ] Performance claims explicitly qualified as reference model evidence without claiming formal ISO 26262 certification.
- [ ] Native compilation steps documented in `docs/QNX_BUILD.md` for judges reviewing code reproducibility.
