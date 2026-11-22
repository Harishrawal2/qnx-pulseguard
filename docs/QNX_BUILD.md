# QNX SDP 8.0 Cross-Compilation, Target Deployment, & QEMU Guide

This guide details the verified, officially supported workflow for cross-compiling the **QNX PulseGuard Native Agent** and running it under **QNX SDP 8.0** using QEMU on Windows.

---

## 1. Environment Architecture & Pre-requisites

### Target Architecture Alignment
- **Host OS:** Microsoft Windows 11 Pro 64-bit (`AMD64`)
- **Recommended Target Architecture:** `x86_64` (matches host CPU architecture, enabling optimal emulation speed and stability with `qemu-system-x86_64`)
- **Alternative Target Architecture:** `aarch64le` (ARM64, software-emulated via TCG)
- **QNX Toolchain Driver:** `qcc` (using `-Vgcc_ntox86_64` for x86_64 or `-Vgcc_ntoaarch64le` for ARM64)

---

## 2. Official QNX SDP 8.0 Installation Workflow

BlackBerry QNX is a licensed commercial RTOS. The toolchain and system headers must be installed via the **QNX Software Center (QSC)**.

### Step 1: Obtain QSC & License
1. Log in to your [BlackBerry myQNX Developer Account](https://www.qnx.com/account/login.html) or the QNX Everywhere / Hackathon portal.
2. Download the Windows installer: `qsc-2.x-win32.exe` from the myQNX downloads section.
3. Run the installer and follow the desktop setup wizard.

### Step 2: Activate License & Install Packages
1. Launch **QNX Software Center**.
2. Navigate to **Configuration > Licenses > Add License** and input your QNX Hackathon license key or import your `.lic` license certificate file.
3. In the **Available** tab, select:
   - `QNX Software Development Platform 8.0 (Host Tools)`
   - `QNX Neutrino RTOS 8.0 Target Support (x86_64)`
   - `QNX Target Images & BSPs (x86_64 / QEMU)`
4. Specify your installation target path (e.g., `C:\qnx800` or `%USERPROFILE%\qnx800`).
5. Click **Install** and accept the license terms.

### Step 3: Activate Build Environment
Open a standard Windows Command Prompt (`cmd.exe`) or PowerShell session and source the environment script:

```cmd
:: If installed at C:\qnx800:
call C:\qnx800\qnxsdp-env.bat

:: Verify compiler driver availability
qcc -V
:: Expected output: QCC 8.0.0 (or compatible GCC driver for QNX 8.0)
```

---

## 3. Cross-Compiling Native Binaries

### Option A: Direct QCC Compilation (x86_64 Target)
From the repository root (`qnx-pulseguard`):

```cmd
:: 1. Compile QNX PulseGuard Telemetry Daemon & Workload Emulator:
qcc -Vgcc_ntox86_64 -O3 -Iinclude src/qnx_pulseguard.c src/main.c -lsocket -o qnx_pulseguard

:: 2. Compile Synchronous Microkernel IPC Benchmark:
qcc -Vgcc_ntox86_64 -O3 -Iinclude src/qnx_ipc_bench.c -o qnx_ipc_bench
```

### Option B: Cross-Compiling with CMake
```cmd
mkdir build-x86_64 && cd build-x86_64
cmake .. -DCMAKE_SYSTEM_NAME=QNX -DCMAKE_SYSTEM_PROCESSOR=x86_64 -DCMAKE_C_COMPILER=qcc -DCMAKE_C_FLAGS="-Vgcc_ntox86_64 -O3"
cmake --build .
```

---

## 4. QEMU Target Virtual Machine Setup

### Installing QEMU on Windows
QEMU can be installed via Windows Package Manager (`winget`):
```powershell
winget install SoftwareFreedomConservancy.QEMU --accept-source-agreements --accept-package-agreements
```

### Supported QNX QEMU Target Image Workflows
According to official QNX documentation, bootable target virtual machine disks are generated using either:
1. **The QNX Quick Start Target Image (QSTI):** Pre-built raw/qcow2 disk image distributed via the QNX Software Center.
2. **`mkqnximage` Tool:** The official QNX utility included with SDP 8.0 to generate bootable QEMU/VM images:
   ```cmd
   mkqnximage --type=qemu --arch=x86_64 --output=qnx800-x86_64.img
   ```

### Correct Networking Model (Guest-to-Host Telemetry)
In QEMU user networking (`-netdev user`), host and guest routing functions as follows:
- **`hostfwd` Parameter (`hostfwd=tcp::2222-:22`):** Forwards incoming *host-to-guest* traffic (e.g. host SSH connection to guest port 22). It does **not** forward guest traffic to the host.
- **SLIRP Virtual Gateway (`10.0.2.2`):** The host machine is always reachable from within the QNX guest at IP `10.0.2.2`.
- **Telemetry Transmission:** The native QNX agent running in the guest must send UDP packets to `10.0.2.2:9876`. QEMU automatically routes these packets to the host UDP socket at `127.0.0.1:9876`.

### Booting QNX in QEMU:
```powershell
qemu-system-x86_64 `
  -m 2048 -smp 2 `
  -drive file=qnx800-x86_64.img,format=raw,if=virtio `
  -netdev user,id=net0,hostfwd=tcp::2222-:22 `
  -device virtio-net-pci,netdev=net0 `
  -nographic
```

---

## 5. Deployment and Telemetry Verification

### Step 1: Transfer Binaries to Target
Using SCP from the host:
```cmd
scp -P 2222 qnx_pulseguard qnx_ipc_bench root@127.0.0.1:/tmp/
```

### Step 2: Run Microkernel IPC Benchmark on QNX Shell
```sh
ssh -p 2222 root@127.0.0.1
cd /tmp
chmod +x qnx_ipc_bench qnx_pulseguard

# Execute native synchronous IPC benchmark:
./qnx_ipc_bench
```
*Capture terminal output for evidence records.*

### Step 3: Launch Native PulseGuard Emitter
```sh
# Emit live telemetry directed to host gateway IP 10.0.2.2 on port 9876
./qnx_pulseguard 10.0.2.2 9876 &
```

### Step 4: Verify Dashboard Transition
1. Open `http://127.0.0.1:8080` in your host browser.
2. Observe the data mode indicator badge in the top-right header:
   - Transitions from `DEMO MODE` to **`REAL QNX MODE (Live Target [127.0.0.1])`**.
   - Timing Source reflects live microkernel `ClockCycles()` timestamps.
