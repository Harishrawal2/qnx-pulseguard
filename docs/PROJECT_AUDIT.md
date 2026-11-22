# QNX PulseGuard: Comprehensive Project Audit

**Document:** Phase 1 Project Audit  
**Project:** QNX PulseGuard  
**Hackathon:** BlackBerry QNX Global Hackathon 2026  
**Category:** Developer Tooling & Host Tools  
**Target:** QNX Software Development Platform (SDP) 8.0  
**Audit Date:** 2026-11-22  
**Auditor:** Principal Embedded Systems Engineer & QA Architect  

---

## 1. Executive Summary

QNX PulseGuard is an observability and real-time performance analysis platform designed to evaluate deterministic task scheduling, latency jitter, and deadline compliance on QNX Neutrino RTOS 8.0 applications.

This audit assesses the current state of the repository, analyzes existing assets, evaluates the local execution environment, checks toolchain availability, and identifies the gap between the current prototype and production-ready hackathon criteria.

---

## 2. Environment & Toolchain Inspection

| Tool / Runtime | Status in Local Environment | Version / Path | Impact |
|---|---|---|---|
| **Node.js** | Available | v24.13.0 (`C:\Program Files\nodejs\node.exe`) | Can run modern React + TS build pipelines & Node backends |
| **npm** | Available | v11.x (`C:\Program Files\nodejs\npm.ps1`) | Package installation and frontend builds supported |
| **Python** | Available | v3.14.7 (`C:\WINDOWS\py.exe`) | Existing Python telemetry server & tooling supported |
| **Git** | Available | v2.46.2 (`C:\Program Files\Git\cmd\git.exe`) | Version control active (`origin: https://github.com/Harishrawal2/qnx-pulseguard`) |
| **QNX SDP 8.0 Toolchain (`qcc`)** | **BLOCKED** | Not installed in host PATH | Native QNX cross-compilation cannot execute on this host |
| **QNX Target / QEMU** | **BLOCKED** | Not installed / Not configured | Execution on physical or virtual QNX 8.0 target blocked locally |
| **Native C Compiler (`gcc`/`clang`/`cl`)** | **BLOCKED** | Not installed in host PATH | Host native C compilation must be performed via CI / Docker / WSL |

> **Audit Finding (Environment Compliance):**  
> Per strict hackathon guidelines, real QNX compilation and target execution are marked as **BLOCKED by environment**. QNX PulseGuard includes genuine, standard-compliant QNX Neutrino 8.0 C source code, but this report explicitly certifies that native compilation on this machine was not performed due to the absence of the proprietary QNX SDP 8.0 license and toolchain.

---

## 3. Current Repository Structure

```text
qnx-pulseguard/
├── include/
│   └── qnx_pulseguard.h       # C header defining telemetry structs & QNX pulse codes
├── src/
│   ├── qnx_pulseguard.c       # Core daemon with ChannelCreate(), ClockCycles(), UDP emitter
│   └── main.c                 # Multi-priority vehicle ECU demonstrator (SCHED_FIFO)
├── host_tooling/
│   ├── dashboard.py           # Python HTTP REST API + UDP listener + rolling history buffer
│   ├── cli_monitor.py         # Terminal live monitor
│   ├── simulate_emitter.py    # Standalone UDP binary telemetry simulator
│   └── web/
│       └── index.html         # Tailwind CSS + Chart.js responsive live monitoring dashboard
├── CMakeLists.txt             # Cross-compilation definitions for qcc (aarch64le / x86_64)
├── Makefile                   # Makefile for QNX and host builds
└── README.md                  # Project overview documentation
```

---

## 4. Feature-by-Feature Audit Matrix

| Feature / Module | Sub-component | Current Status | Technical Analysis & Evidence |
|---|---|---|---|
| **QNX Native Component** | Microkernel IPC (`ChannelCreate`, `ConnectAttach`) | **Implemented but unverified** | Syntactically correct in `src/qnx_pulseguard.c:92`. Needs SDP 8.0 compiler. |
| | Cycle Counter (`ClockCycles()`, `SYSPAGE_ENTRY`) | **Implemented but unverified** | Uses `ClockCycles()` & `SYSPAGE_ENTRY(qtime)->cycles_per_sec` (`src/qnx_pulseguard.c:42`). |
| | Scheduling (`pthread_setschedparam`, `SCHED_FIFO`) | **Implemented but unverified** | Set in `src/main.c:48` for priorities 63, 50, 15. |
| | Synchronous IPC Round-Trip Benchmark (`MsgSend/Receive/Reply`)| **Missing** | Dedicated round-trip micro-benchmark module needed. |
| | Target Compilation on QNX SDP 8.0 | **Blocked by environment** | `qcc` binary absent on host system. |
| | Target Execution on QNX 8.0 Target / QEMU | **Blocked by environment** | QNX image & QEMU absent on host system. |
| **Host Backend** | UDP Binary Frame Receiver | **Implemented and verified** | Successfully decodes binary struct `<4sIQIIQd?` on port 9876 (`dashboard.py:122`). |
| | Real-Time Rolling History Buffer | **Implemented and verified** | Maintains 30-sample circular deque with wall-clock timestamps (`dashboard.py:165`). |
| | Real-Time REST API (`/api/telemetry`) | **Implemented and verified** | Returns live state, tasks, rolling latency history, and event logs. |
| | CPU Contention Benchmark Trigger (`/api/toggle_stress`)| **Implemented and verified** | Toggles synthetic interference and records preemption verification events. |
| | ISO 26262 / IEC 62304 Audit Export (`/api/export_audit`)| **Implemented and verified** | Downloads structured JSON audit payload (`dashboard.py:230`). |
| | WebSocket Streaming Server | **Partially implemented** | Currently uses 1 Hz high-frequency REST polling; WebSocket endpoint needed for sub-100ms. |
| | Data Ingestion Rate & Packet Loss Tracker | **Partially implemented** | Tracks packet counts and byte counts; needs explicit ingestion rate (KB/s). |
| **Host Frontend** | Real-Time Dashboard Overview | **Implemented and verified** | Pixel-matched to design with live multi-task waterfall and Gaussian jitter histogram. |
| | Task Matrix Table (ECU workloads) | **Implemented and verified** | Tracks 3 tasks with ASIL grades, priorities, deadlines, and budget bars. |
| | Explicit "REAL QNX MODE" vs "DEMO MODE" Indicator | **Partially implemented** | Shows "STREAM ACTIVE" but lacks an explicit toggle and visual tag distinguishing modes. |
| | IPC Benchmark Visualization Screen | **Missing** | Dedicated view for message round-trip distributions needed. |
| | Kernel Event Trace Viewer with Filters | **Partially implemented** | Console stream exists; needs severity filtering (INFO/WARN/ERROR) and export. |
| | Comprehensive Multi-Screen Navigation | **Partially implemented** | Tabs exist in topbar (`Monitoring`, `Analysis`, `Reports`, `Configuration`); needs active views. |
| | React + TypeScript Enterprise Architecture | **Missing** | Currently implemented as single-file HTML+Tailwind+Chart.js app. |
| **Quality & Tests** | Backend API & Schema Validation Tests | **Partially implemented** | Ad-hoc Python verification scripts; needs structured test suite (`tests/`). |
| | Timing Conversion & Jitter Statistics Tests | **Missing** | Formal unit tests verifying P50/P95/P99 math needed. |
| | Frontend Component & Rendering Tests | **Missing** | Automated UI test suite needed. |
| | Comprehensive Documentation Suite | **Partially implemented** | `README.md` exists; formal architecture, setup, and rubric evidence docs needed in `docs/`. |

---

## 5. Identified Gaps & Technical Debt

1. **Explicit Data Mode Distinction**:
   - The UI must prominently declare whether it is receiving live data from a real QNX target (`REAL QNX MODE`) or running the calibrated synthetic engine (`DEMO MODE`).
   - It must never silently mask a broken QNX connection with simulated numbers.

2. **Dedicated QNX IPC Benchmark**:
   - Hackathon judges prioritize developer tooling that exercises QNX Neutrino's core capability: synchronous message passing.
   - We must add a dedicated QNX IPC benchmarking utility (`src/qnx_ipc_bench.c`) measuring round-trip microsecond latencies for `MsgSend()` -> `MsgReceive()` -> `MsgReply()` across configurable message payload sizes (64B, 512B, 4KB, 64KB).

3. **Multi-View Navigation**:
   - The navigation tabs (`Monitoring`, `Analysis`, `Reports`, `Configuration`) should switch dedicated full-featured views or deep-dive inspection panes.

4. **Automated Test Suite**:
   - Formal test scripts for packet struct packing/unpacking, math accuracy (P50, P95, P99, P99.9 percentile latency), API boundary conditions, and report generation.

5. **Formal Documentation Suite**:
   - Standardized documentation in `docs/` covering architecture, setup, QNX 8.0 build workflows, API evidence, and demo video script.

---

## 6. Prioritized Implementation Roadmap

- **Milestone 1:** Data Mode Architecture (Explicit REAL QNX vs DEMO MODE indicators & backend signaling).
- **Milestone 2:** Native QNX IPC Benchmark Component (`src/qnx_ipc_bench.c` & payload-sweep round-trip metrics).
- **Milestone 3:** Advanced Statistical Engine (P50, P95, P99, P99.9 latency, standard deviation, and data ingestion KB/s).
- **Milestone 4:** Multi-View Frontend Experience (Dedicated views for Monitoring, Task Matrix, IPC Benchmark, and Kernel Trace Logs).
- **Milestone 5:** Automated Test Suite (Unit and integration tests for backend, schema validation, and timing math).
- **Milestone 6:** Complete Documentation Suite (`docs/ARCHITECTURE.md`, `docs/SETUP.md`, `docs/QNX_BUILD.md`, etc.).
