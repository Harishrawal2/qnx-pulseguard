# QNX SDP 8.0 Cross-Compilation & Target Deployment Guide

This document provides complete instructions for cross-compiling the **QNX PulseGuard Native Agent** and the **QNX IPC Microkernel Benchmark** using BlackBerry QNX Software Development Platform (SDP) 8.0.

---

## 1. Prerequisites: QNX SDP 8.0 Environment

Cross-compilation requires an active QNX Software Development Platform 8.0 installation on a Linux (Ubuntu 20.04/22.04 LTS) or Windows host.

### Environment Variable Activation
Before invoking the compiler, source the QNX environment configuration script:

```bash
# On Linux Host:
source ~/qnx800/qnxsdp-env.sh

# On Windows Host:
C:\qnx800\qnxsdp-env.bat
```

Verify that the QNX compiler driver `qcc` is accessible:
```bash
qcc -V
# Expected: QCC 8.0.0 or compatible GCC cross-compiler driver
```

---

## 2. Source Code Architecture

The native C components reside in:
- `include/qnx_pulseguard.h`: Telemetry frame structures (`PulseGuardFrame`, `TaskTelemetry`), QNX pulse codes, and network wire-format definitions.
- `src/qnx_pulseguard.c`: Core daemon implementing Neutrino microkernel channel creation (`ChannelCreate`), non-blocking connection attachment (`ConnectAttach`), hardware cycle measurement (`ClockCycles`), and UDP network emission.
- `src/qnx_ipc_bench.c`: Standalone microkernel benchmark measuring `MsgSend() -> MsgReceive() -> MsgReply()` round-trip latency across message sizes (64B to 16KB).
- `src/main.c`: Synthetic real-time ECU workload emulator executing three priority-driven periodic tasks under `SCHED_FIFO`.

---

## 3. Cross-Compiling with CMake

PulseGuard includes standard CMake build configuration supporting cross-compilation toolchain files.

### For ARM64 (AArch64) Targets:
```bash
mkdir -p build-aarch64 && cd build-aarch64
cmake .. \
  -DCMAKE_SYSTEM_NAME=QNX \
  -DCMAKE_SYSTEM_PROCESSOR=aarch64 \
  -DCMAKE_C_COMPILER=qcc \
  -DCMAKE_C_FLAGS="-Vgcc_ntoaarch64le -O3 -Wall"
make -j$(nproc)
```

### For x86_64 Targets (QEMU / Intel / AMD):
```bash
mkdir -p build-x86_64 && cd build-x86_64
cmake .. \
  -DCMAKE_SYSTEM_NAME=QNX \
  -DCMAKE_SYSTEM_PROCESSOR=x86_64 \
  -DCMAKE_C_COMPILER=qcc \
  -DCMAKE_C_FLAGS="-Vgcc_ntox86_64 -O3 -Wall"
make -j$(nproc)
```

---

## 4. Direct QCC Compilation (Alternative)

If CMake is not configured, compile directly with the `qcc` compiler:

```bash
# 1. Compile QNX PulseGuard Daemon & Workload Emulator:
qcc -Vgcc_ntoaarch64le -O3 -Iinclude src/qnx_pulseguard.c src/main.c -lsocket -o qnx_pulseguard

# 2. Compile QNX Microkernel IPC Benchmark:
qcc -Vgcc_ntoaarch64le -O3 -Iinclude src/qnx_ipc_bench.c -o qnx_ipc_bench
```

*(Note: On QNX 8.0, socket APIs are part of `libsocket.so`).*

---

## 5. Deploying to QNX Target

Transfer the compiled binaries to your QNX 8.0 target via SCP or FTP:

```bash
# Set target IP address
export QNX_TARGET_IP=192.168.1.100

# Copy binaries to /tmp or /usr/bin
scp qnx_pulseguard qnx_ipc_bench root@${QNX_TARGET_IP}:/tmp/
```

### Executing on Target:
Log in to the QNX target shell via SSH or serial console:

```sh
ssh root@192.168.1.100
cd /tmp
chmod +x qnx_pulseguard qnx_ipc_bench

# Run the PulseGuard telemetry agent, directing frames to host workstation
./qnx_pulseguard 192.168.1.50 9876 &

# Run the synchronous microkernel IPC benchmark:
./qnx_ipc_bench
```

---

## 6. Running under QEMU Emulation

If physical hardware is unavailable, run QNX SDP 8.0 inside QEMU with network port-forwarding:

```bash
qemu-system-aarch64 \
  -M virt \
  -cpu cortex-a57 \
  -m 2048 \
  -kernel qnx-ifs-8.0.bin \
  -netdev user,id=net0,hostfwd=tcp::2222-:22,hostfwd=udp::9876-:9876 \
  -device virtio-net-pci,netdev=net0 \
  -nographic
```
