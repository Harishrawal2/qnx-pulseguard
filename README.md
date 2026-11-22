# QNX PulseGuard 🛡️
### Real-Time Deterministic Latency Profiler & Timing Analysis Host Tool for QNX SDP 8.0

[![License](https://img.shields.io/badge/License-Apache_2.0-blue.svg)](LICENSE)
[![Target](https://img.shields.io/badge/Target-QNX_SDP_8.0-red.svg)](https://blackberry.qnx.com)
[![Category](https://img.shields.io/badge/Category-Developer_Tooling_%26_Host_Tools-green.svg)]()
[![Status](https://img.shields.io/badge/Tests-9%2F9_Passing-brightgreen.svg)](docs/TEST_RESULTS.md)

---

## 📌 Overview

**QNX PulseGuard** is a real-time deterministic performance monitoring and timing analysis platform engineered for **BlackBerry QNX Software Development Platform (SDP) 8.0**.

In autonomous automotive systems (ISO 26262 ASIL-D), hard real-time tasks such as Brake-by-Wire and Steering Control must execute deterministically without deadline breaches—even when competing with lower-priority background telematics under heavy CPU contention.

PulseGuard pairs an ultra-low-overhead native QNX C telemetry agent with an enterprise host dashboard to deliver **sub-microsecond execution latency measurement**, **Gaussian jitter distribution analysis**, **microkernel IPC benchmarking**, and **exportable performance verification evidence**.

---

## ✨ Key Features

- **Sub-Microsecond Cycle Timestamping:** Utilizes QNX `ClockCycles()` and `SYSPAGE_ENTRY(qtime)` for high-resolution timing without system call overhead.
- **Automotive Workload Matrix:** Evaluates multi-priority tasks under `SCHED_FIFO`:
  - `BrakeByWire_Actuator` (Priority 63, ASIL-D, 1,000 µs deadline)
  - `SteeringAngle_Sensor` (Priority 50, ASIL-B, 5,000 µs deadline)
  - `Vehicle_Telematics_GW` (Priority 15, Best Effort, 20,000 µs deadline)
- **Synchronous Microkernel IPC Benchmarking:** Directly measures round-trip `MsgSend() -> MsgReceive() -> MsgReply()` latency across payload sizes (64B to 16KB).
- **CPU Contention Stress Testing:** Dynamic injection of controlled CPU contention to empirically verify `SCHED_FIFO` priority preemption.
- **Enterprise Dark-Mode Dashboard:** Developed with Tailwind CSS, Chart.js, and Lucide vector icons, featuring persistent circular buffer history that survives page reloads.
- **Dual Operating Modes:** Transparently switches between **REAL QNX MODE** (live target UDP frames) and **DEMO MODE** (synthetic datasets for workstation testing).
- **Audit & Evidence Export:** Instant JSON and CSV export of statistical timing evidence, worst-case execution time (WCET), and percentile distributions ($P_{50}, P_{95}, P_{99}, P_{99.9}$).

---

## 🏛️ System Architecture

```text
+-------------------------------------------------------------+
|                     QNX SDP 8.0 Target                      |
|                                                             |
|  +-------------------------------------------------------+  |
|  |           Real-Time ECU Tasks (SCHED_FIFO)            |  |
|  |   BrakeByWire (Prio 63)  |  SteeringAngle (Prio 50)   |  |
|  +-------------------------------------------------------+  |
|                             |                               |
|              ClockCycles()  |  SYSPAGE_ENTRY(qtime)         |
|                             v                               |
|  +-------------------------------------------------------+  |
|  |        QNX PulseGuard Native Agent (C99 / QCC)        |  |
|  |      ChannelCreate()     |    MsgSend / MsgReceive    |  |
|  +-------------------------------------------------------+  |
+------------------------------|------------------------------+
                               | Binary UDP Telemetry (Port 9876)
                               v
+-------------------------------------------------------------+
|                   Host Workstation Tooling                  |
|                                                             |
|  +-------------------------------------------------------+  |
|  |       Python / Node.js Ingestion & Analytics Engine    |  |
|  |       - Percentile Evaluator (P50, P95, P99, P99.9)   |  |
|  |       - Circular History Buffer (30s Rolling Window)  |  |
|  |       - REST API & WebSocket Publisher (:8080)        |  |
|  +-------------------------------------------------------+  |
|                             |                               |
|                             v                               |
|  +-------------------------------------------------------+  |
|  |          PulseGuard Enterprise Web Dashboard          |  |
|  |    (Monitoring | Analysis | Reports | Configuration)   |  |
|  +-------------------------------------------------------+  |
+-------------------------------------------------------------+
```

---

## 🚀 Quickstart

### 1. Host Dashboard (Zero-Dependency)
```bash
# Clone the repository
git clone https://github.com/Harishrawal2/qnx-pulseguard.git
cd qnx-pulseguard

# Launch the host telemetry server & web interface
py -u host_tooling\dashboard.py       # Windows
# or: python3 -u host_tooling/dashboard.py # Linux / macOS
```
Open **[http://127.0.0.1:8080](http://127.0.0.1:8080)** in any modern web browser.

### 2. Run Test Suite
```bash
py -m unittest discover tests         # Windows
# or: python3 -m unittest discover tests   # Linux / macOS
```
*All 9 unit and integration tests validate percentile mathematics, WCET tracking, deadline detection, and REST endpoints.*

---

## 🛠️ Cross-Compiling for QNX 8.0

Source the QNX SDP 8.0 environment and compile using `qcc`:

```bash
# Set up QNX SDP 8.0 environment
source ~/qnx800/qnxsdp-env.sh

# Cross-compile for ARM64 target (AArch64)
qcc -Vgcc_ntoaarch64le -O3 -Iinclude src/qnx_pulseguard.c src/main.c -lsocket -o qnx_pulseguard
qcc -Vgcc_ntoaarch64le -O3 -Iinclude src/qnx_ipc_bench.c -o qnx_ipc_bench

# Deploy and run on QNX target
scp qnx_pulseguard root@192.168.1.100:/tmp/
ssh root@192.168.1.100 "/tmp/qnx_pulseguard <HOST_IP> 9876"
```
*Detailed build instructions for CMake and QEMU are available in [docs/QNX_BUILD.md](docs/QNX_BUILD.md).*

---

## 📚 Documentation Index

- [Project Audit & Requirements Matrix](docs/PROJECT_AUDIT.md)
- [System Architecture & Wire Protocol](docs/ARCHITECTURE.md)
- [Host & Target Setup Guide](docs/SETUP.md)
- [QNX SDP 8.0 Build & Deployment](docs/QNX_BUILD.md)
- [QNX 8.0 API Evidence & Trace](docs/QNX_API_EVIDENCE.md)
- [Test Execution & Verification Results](docs/TEST_RESULTS.md)
- [Measurement & Benchmark Methodology](docs/BENCHMARK_METHODOLOGY.md)
- [Video Demonstration Script](docs/DEMO_SCRIPT.md)
- [Hackathon Submission Record](docs/HACKATHON_SUBMISSION.md)

---

## 📄 License

Distributed under the **Apache License 2.0**. See [`LICENSE`](LICENSE) for details.
