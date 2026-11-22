# QNX SDP 8.0 Native Compilation & Deployment Guide

This guide details the verified procedure for cross-compiling **QNX PulseGuard** with the official QNX SDP 8.0 toolchain, generating a VirtualBox target image via `mkqnximage`, running the VM, and streaming real-time telemetry.

---

## 1. Toolchain & Environment Configuration

The build host requires **QNX Software Development Platform (SDP) 8.0** installed on Windows or Linux.

### Loading QNX Environment (Windows Host):
```cmd
call "C:\Users\DELL\qnx800\qnxsdp-env.bat"
```
Verify the environment variables:
```cmd
echo QNX_HOST = %QNX_HOST%
echo QNX_TARGET = %QNX_TARGET%
qcc -V
```
Expected output:
```text
qcc: gcc (GCC) 12.2.0 (QNX SDP 8.0.0)
Default target: gcc_ntox86_64
```

---

## 2. Cross-Compiling Native Binaries

Navigate to the project root directory:
```cmd
cd /d C:\Users\DELL\.gemini\antigravity\scratch\qnx-pulseguard
```

### 1. Compile Telemetry Daemon (`qnx_pulseguard`):
```cmd
qcc -Vgcc_ntox86_64 -O3 -Iinclude src/qnx_pulseguard.c src/main.c -lsocket -o qnx_pulseguard
```
* Generates native ELF64 executable: `qnx_pulseguard` (19,344 bytes).
* Uses `-lsocket` for UDP broadcast networking.

### 2. Compile Synchronous IPC Benchmark (`qnx_ipc_bench`):
```cmd
qcc -Vgcc_ntox86_64 -O3 -Iinclude src/qnx_ipc_bench.c -o qnx_ipc_bench
```
* Generates native ELF64 executable: `qnx_ipc_bench` (18,752 bytes).
* Directly tests `ChannelCreate()`, `ConnectAttach()`, `MsgSend()`, `MsgReceive()`, and `MsgReply()`.

---

## 3. Generating & Running the QNX VirtualBox VM

QNX SDP 8.0 includes `mkqnximage`, which generates ready-to-run virtual machine disks.

### 1. Generate SSH Key on Host:
```cmd
ssh-keygen -q -t ed25519 -N "" -f "C:\Users\DELL\.ssh\id_ed25519"
```

### 2. Create Target Directory & Generate Disk:
```cmd
mkdir qnx_vbox_vm
cd qnx_vbox_vm
mkqnximage --type=vbox --arch=x86_64 --ssh-ident=C:/Users/DELL/.ssh/id_ed25519.pub --noprompt
```
* Output files generated in `output/`:
  - `disk-vbox.vmdk` (VirtualBox disk descriptor)
  - `disk-vbox` (raw flat disk image)
  - `ifs.bin` (QNX Initial File System image)
  - `procnto-smp-instr.sym` (instrumented kernel symbols)

### 3. Launch QNX VM in Headless Mode:
```cmd
mkqnximage --run=-h
```

### 4. Resolve VM IP Address:
```cmd
mkqnximage --getip
```
Expected output:
```text
192.168.56.102
```

---

## 4. Deploying & Running on QNX Target

### 1. Verify SSH Connectivity:
```cmd
ssh -i "C:\Users\DELL\.ssh\id_ed25519" -o Ciphers=aes128-ctr -o MACs=hmac-sha2-256 root@192.168.56.102 "uname -a"
```
Output:
```text
QNX noname 8.0.0 2026/10/30-09:10:05EDT x86pc x86_64
```

### 2. Copy Binaries to Target:
```cmd
scp -i "C:\Users\DELL\.ssh\id_ed25519" -o Ciphers=aes128-ctr -o MACs=hmac-sha2-256 qnx_ipc_bench root@192.168.56.102:/tmp/
scp -i "C:\Users\DELL\.ssh\id_ed25519" -o Ciphers=aes128-ctr -o MACs=hmac-sha2-256 qnx_pulseguard root@192.168.56.102:/tmp/
ssh -i "C:\Users\DELL\.ssh\id_ed25519" -o Ciphers=aes128-ctr -o MACs=hmac-sha2-256 root@192.168.56.102 "chmod +x /tmp/qnx_ipc_bench /tmp/qnx_pulseguard"
```

### 3. Run IPC Benchmark on Target:
```cmd
ssh -i "C:\Users\DELL\.ssh\id_ed25519" -o Ciphers=aes128-ctr -o MACs=hmac-sha2-256 root@192.168.56.102 "/tmp/qnx_ipc_bench"
```

### 4. Stream Native Telemetry to Host Dashboard:
Ensure the host dashboard is running:
```cmd
py host_tooling\dashboard.py
```
Then start the telemetry emitter on QNX target, directing UDP packets to the host adapter (`192.168.56.1`):
```cmd
ssh -i "C:\Users\DELL\.ssh\id_ed25519" -o Ciphers=aes128-ctr -o MACs=hmac-sha2-256 root@192.168.56.102 "/tmp/qnx_pulseguard 192.168.56.1 9876"
```
The dashboard at `http://127.0.0.1:8080` will automatically transition to **`REAL QNX MODE`** displaying live metrics from the QNX target.
