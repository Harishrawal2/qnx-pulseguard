# QNX PulseGuard — Host & Target Setup Guide

This guide details how to set up the host development workstation and target QNX SDP 8.0 execution environment for **QNX PulseGuard**.

---

## 1. System Requirements

### Host Workstation
- **Operating System:** Windows 10/11 (64-bit), Ubuntu 22.04 LTS+, or macOS 13+
- **Python:** Python 3.10+ (standard library only; no third-party pip dependencies required)
- **Node.js (Optional):** Node.js 18+ (for optional React/TypeScript tooling)
- **Web Browser:** Modern Chromium or Firefox browser (supports Canvas / WebSockets)
- **Network Ports:** 
  - `8080/TCP`: HTTP Web UI & REST API
  - `9876/UDP`: Telemetry ingestion listener

### QNX Target / Emulation Environment
- **Target OS:** BlackBerry QNX Software Development Platform (SDP) 8.0
- **Architectures:** ARM64 (`aarch64le`) or x86_64 (`x86_64le`)
- **BSP / Target Platform:** 
  - QNX Neutrino RTOS 8.0 Reference Target (e.g., NXP S32G, Renesas R-Car, Raspberry Pi 4)
  - QEMU 7.0+ QNX Virtual Machine (`qemu-system-aarch64` / `qemu-system-x86_64`)
- **Toolchain:** QNX QCC (`qcc -Vgcc_ntoarmv7le` or `qcc -Vgcc_ntoaarch64le` or `qcc -Vgcc_ntox86_64`)

---

## 2. Host Quickstart (5 Minutes)

QNX PulseGuard includes a zero-dependency host daemon (`host_tooling/dashboard.py`) with an embedded web server, UDP telemetry listener, statistical analyzer, and interactive browser dashboard.

### Step 1: Clone Repository
```bash
git clone https://github.com/Harishrawal2/qnx-pulseguard.git
cd qnx-pulseguard
```

### Step 2: Launch Host Daemon
```bash
# Windows
py -u host_tooling\dashboard.py

# Linux / macOS
python3 -u host_tooling/dashboard.py
```
Expected output:
```text
======================================================================
  QNX PulseGuard Enterprise Telemetry Host Server v2.4.0
======================================================================
[INFO] HTTP Dashboard server listening on http://127.0.0.1:8080
[INFO] Native QNX UDP Telemetry listener bound on 0.0.0.0:9876
[INFO] Static web UI hosted from host_tooling/web
======================================================================
```

### Step 3: Open Browser Dashboard
Navigate to:
```
http://127.0.0.1:8080
```
- The dashboard automatically begins receiving synthetic telemetry in **DEMO MODE** if no native QNX hardware is transmitting on UDP port 9876.
- As soon as genuine binary UDP frames arrive from a QNX target, the mode badge flips automatically to **REAL QNX MODE**.

---

## 3. Running Test Suites

Verify host calculation logic, percentile mathematics, and REST endpoints:

```bash
# Windows
py -m unittest discover tests

# Linux / macOS
python3 -m unittest discover tests
```
All 9 unit and integration tests validate:
- Real-time Gaussian jitter standard deviation calculation
- Percentile latency calculations ($P_{50}$, $P_{95}$, $P_{99}$, $P_{99.9}$)
- ASIL-D deadline breach identification
- Circular buffer persistence across page refreshes
- Telemetry endpoint serialization and CORS compliance

---

## 4. Connecting a Target QNX 8.0 Machine

### Network Configuration
Ensure the host machine IP (e.g., `192.168.1.50`) is reachable from the QNX target:

1. **Verify host UDP firewall:**
   Allow inbound UDP traffic on port `9876`.
2. **Configure target emitter:**
   Pass the host IP address as an argument or environment variable to the QNX binary:
   ```sh
   # On QNX 8.0 Shell:
   ./qnx_pulseguard 192.168.1.50 9876
   ```
3. **Verify Connection:**
   The Web UI status banner will indicate:
   `STREAM ACTIVE (10.0 Hz) - REAL QNX MODE` with source `SYS_PAGE_ENTRY(qtime)` and target `aarch64le`.
