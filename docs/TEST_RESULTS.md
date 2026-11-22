# QNX PulseGuard — Test Execution Results & Verification Record

**Test Execution Date:** 2026-11-22  
**Target Platform:** BlackBerry QNX Neutrino RTOS 8.0.0 (`x86_64`) running under Oracle VM VirtualBox 7.2  
**Toolchain:** QNX Software Development Platform (SDP) 8.0 (`qcc` GCC 12.2.0 `gcc_ntox86_64`)  
**Host Environment:** Windows 11 Pro x64, Python 3.14.2, VirtualBox Host-Only Ethernet Adapter (`192.168.56.1`)  
**Target IP:** `192.168.56.102` (DHCP leased from VirtualBox host-only network)  

---

## 1. Test Summary Matrix

| Test Suite / Area | Tests Executed | Passed | Failed | Status | Notes |
|:---|:---:|:---:|:---:|:---:|:---|
| **Python Statistical & Schema Suite** | 16 | 16 | 0 | **PASS** | Validated $P_{50}, P_{95}, P_{99}, P_{99.9}$, WCET, binary packing, security |
| **QNX SDP 8.0 Cross-Compilation** | 2 | 2 | 0 | **PASS** | `qnx_pulseguard` (19,344 B) and `qnx_ipc_bench` (18,752 B) compiled cleanly via `qcc` |
| **QNX VirtualBox Target Boot** | 1 | 1 | 0 | **PASS** | Generated via `mkqnximage --type=vbox --arch=x86_64`, booted headless, IP assigned |
| **Native Synchronous IPC Benchmark** | 5 | 5 | 0 | **PASS** | `MsgSend()` -> `MsgReceive()` -> `MsgReply()` round-trip verified (1.48 µs min latency) |
| **Native UDP Telemetry Streaming** | 1 | 1 | 0 | **PASS** | Target streaming 10 Hz UDP packets to host port 9876; dashboard switched to `REAL_QNX` |

---

## 2. QNX SDP 8.0 Toolchain Verification

Environment loaded via `C:\Users\DELL\qnx800\qnxsdp-env.bat`:
```cmd
QNX_HOST=C:/Users/DELL/qnx800/host/win64/x86_64
QNX_TARGET=C:/Users/DELL/qnx800/target/qnx
qcc -V
qcc: gcc (GCC) 12.2.0 (QNX SDP 8.0.0)
Default target: gcc_ntox86_64
```

### Native Binary Compilation Logs:
```cmd
qcc -Vgcc_ntox86_64 -O3 -Iinclude src/qnx_pulseguard.c src/main.c -lsocket -o qnx_pulseguard
qcc -Vgcc_ntox86_64 -O3 -Iinclude src/qnx_ipc_bench.c -o qnx_ipc_bench
```
Exit code: `0` (Zero errors, zero warnings).

---

## 3. QNX VirtualBox Target Deployment & Boot

Target VM image built and booted via official QNX `mkqnximage`:
```cmd
mkqnximage --type=vbox --arch=x86_64 --ssh-ident=C:/Users/DELL/.ssh/id_ed25519.pub --noprompt
mkqnximage --run=-h
mkqnximage --getip
-> 192.168.56.102
```

Target operating system confirmation over SSH:
```text
$ ssh -i ~/.ssh/id_ed25519 root@192.168.56.102 "uname -a"
QNX noname 8.0.0 2026/10/30-09:10:05EDT x86pc x86_64
```

---

## 4. Genuine Native Microkernel IPC Benchmark Output

Executed directly on QNX Neutrino RTOS 8.0 target (`/tmp/qnx_ipc_bench`):

```text
=================================================================
    QNX PulseGuard - Synchronous Microkernel IPC Latency Benchmark
    Testing: MsgSend() -> MsgReceive() -> MsgReply() Round-Trip  
=================================================================

Payload    Target   Valid    Min (us)   Avg (us)   P95 (us)   Max (us)   Throughput   BW (MB/s) 
--------------------------------------------------------------------------------------------------
64         10000    10000    1.48       5.41       3.78       604.24     184741       11.28     
256        10000    10000    1.50       4.90       3.75       668.22     204053       49.82     
1024       10000    10000    1.53       5.83       3.87       702.13     171417       167.40    
4096       10000    10000    1.74       5.90       4.25       570.16     169439       661.87    
16384      10000    10000    2.13       7.63       5.08       627.97     131087       2048.24   

Benchmark complete. Round-trip microkernel latency evaluated.
```

### Analysis of Empirical Benchmark:
- **Baseline Microkernel Overhead:** 64-byte synchronous message passing round-trip minimum is **1.48 µs**, with a 95th-percentile of **3.78 µs**.
- **Sustained IPC Throughput:** Exceeds **204,000 round-trips/second** on 256-byte automotive control messages.
- **Microkernel Memory Bandwidth:** Reaches **2,048.24 MB/s** on 16 KB bulk telemetry payloads.

---

## 5. Live Native QNX Telemetry Ingestion Verification

Native telemetry daemon launched on QNX target:
```cmd
ssh root@192.168.56.102 "/tmp/qnx_pulseguard 192.168.56.1 9876"
```

Host Dashboard Server (`http://127.0.0.1:8080/api/telemetry`):
```json
{
  "data_mode": "REAL_QNX",
  "data_source": "Live Target [192.168.56.102:57935]",
  "connected": true,
  "qnx_target_info": {
    "os": "QNX Neutrino RTOS 8.0",
    "kernel_binary": "procnto-smp-instr (Instrumented Microkernel)",
    "architecture": "aarch64le / x86_64",
    "timer_source": "ClockCycles() via SYSPAGE_ENTRY(qtime)",
    "scheduler_policy": "SCHED_FIFO (Priority-preemptive)",
    "ipc_primitive": "Neutrino Synchronous Channel / Pulse",
    "channel_id": 1,
    "connection_id": 1
  },
  "tasks": [
    {
      "task_id": 1,
      "name": "BrakeByWire_Actuator",
      "asil": "ASIL-D Reference",
      "priority": 63,
      "target_period_us": 1000.0,
      "deadline_us": 1000.0,
      "current_latency_us": 4.8,
      "wcet_us": 145.8,
      "acet_us": 4.8,
      "deadline_misses": 0,
      "budget_utilization_pct": 0.48,
      "status": "NOMINAL"
    },
    {
      "task_id": 2,
      "name": "SteeringAngle_Sensor",
      "asil": "ASIL-B Reference",
      "priority": 50,
      "target_period_us": 5000.0,
      "deadline_us": 5000.0,
      "current_latency_us": 26.8,
      "wcet_us": 1680.3,
      "acet_us": 26.8,
      "deadline_misses": 0,
      "budget_utilization_pct": 0.54,
      "status": "NOMINAL"
    },
    {
      "task_id": 3,
      "name": "Vehicle_Telematics_GW",
      "asil": "Best-Effort Reference",
      "priority": 15,
      "target_period_us": 20000.0,
      "deadline_us": 20000.0,
      "current_latency_us": 28.2,
      "wcet_us": 565.9,
      "acet_us": 28.2,
      "deadline_misses": 0,
      "budget_utilization_pct": 0.14,
      "status": "NOMINAL"
    }
  ]
}
```

Dashboard UI status banner confirmed:  
**`REAL QNX MODE (Live Target [192.168.56.102:57935])`** — Ingestion rate 10.0 Hz, zero deadline breaches.

---

## 6. Host Unit & Integration Regression Test Log

```text
Ran 16 tests in 0.275s

OK
```
All 16 Python API, schema, security, and statistics tests pass cleanly.
