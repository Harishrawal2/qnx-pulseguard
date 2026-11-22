# PulseGuard: Deterministic RTOS Telemetry Suite

A high-precision real-time telemetry daemon and host observability suite designed for real-time operating systems (QNX Neutrino RTOS 8.0, QEMU, and POSIX targets).

## Key Features
- **Microsecond Precision**: Measures periodic execution jitter and latency using high-resolution hardware counters.
- **Microkernel IPC Integration**: Uses synchronous messaging channels and pulse primitives for lightweight signaling.
- **Priority-Driven Scheduling**: Configurable task priorities with real-time FIFO scheduling (`SCHED_FIFO`).
- **Live Observability Dashboard**: Host-side terminal and web interfaces for real-time latency waterfalls and deadline audits.

## Project Structure
```text
qnx-pulseguard/
├── include/
│   └── qnx_pulseguard.h       # Public headers and telemetry data structures
├── src/
│   ├── qnx_pulseguard.c       # Core daemon and IPC logic
│   └── main.c                 # Multi-priority ECU task pipeline demonstrator
├── host_tooling/
│   ├── dashboard.py           # Host UDP telemetry bridge & web server
│   ├── cli_monitor.py         # Terminal-based live monitor
│   └── web/
│       └── index.html         # Real-time web visualization
├── CMakeLists.txt             # Cross-platform build configuration
└── Makefile                   # Alternative simple makefile
```

## How to Build and Run

### Option 1: Native Host Testing (Linux, macOS, or Windows)
You can compile and test the telemetry pipeline directly on your host environment:

```bash
# Using GCC / Clang
gcc -O2 -Iinclude src/qnx_pulseguard.c src/main.c -o qnx_pulseguard -lpthread -lm

# Run the daemon
./qnx_pulseguard
```

### Option 2: Building for Target (QNX SDP 8.0)
When targeting QNX Neutrino RTOS using the QNX Software Development Platform:
```bash
qcc -Vgcc_ntoarmv7le -O2 -Iinclude src/qnx_pulseguard.c src/main.c -o qnx_pulseguard -lsocket
```

### Option 3: Launching the Host Telemetry Dashboard
In a separate terminal on your host machine:
```bash
# Start the web visualizer
python host_tooling/dashboard.py
# Open your browser at http://localhost:8080

# Or run the lightweight CLI monitor
python host_tooling/cli_monitor.py
```

## License
Open source under the Apache-2.0 License.
