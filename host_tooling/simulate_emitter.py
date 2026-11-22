#!/usr/bin/env python3
"""
PulseGuard - Python Simulator / Fallback Emitter
Allows running and testing the full real-time telemetry stream without needing a local C compiler.
Broadcasts UDP frames matching the exact binary struct of the C daemon.
"""

import socket
import struct
import time
import math
import random

PORT_UDP = 9876
HOST = "127.0.0.1"

# Struct matching PulseGuardFrame in C:
# magic[4], sequence_num(I), timestamp_ns(Q), active_tasks_count(I),
# qnx_kernel_ticks(I), total_deadline_misses(Q), system_jitter_us(d), failsafe_triggered(?)
HEADER_FORMAT = "<4sIQIIQd?"

# TaskTelemetry struct format:
# task_name[32], task_id(I), level(i), priority(I), target_period_ns(Q),
# worst_latency_ns(Q), best_latency_ns(Q), avg_latency_ns(Q), deadline_misses(Q),
# total_executions(Q), cpu_utilization_pct(d), is_healthy(?)
TASK_FORMAT = "<32sIiIQQQQQQd?"

def run_simulation(max_frames=None):
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    print(f"[Simulator] Emitting QNX 8.0 Telemetry to UDP {HOST}:{PORT_UDP}...")

    tasks = [
        {"name": b"BrakeByWire_Actuator\x00", "id": 1, "level": 2, "priority": 63, "period": 1000000, "base_lat": 12500},
        {"name": b"SteeringAngle_Sensor\x00", "id": 2, "level": 1, "priority": 50, "period": 5000000, "base_lat": 24000},
        {"name": b"Vehicle_Telematics_GW\x00", "id": 3, "level": 0, "priority": 15, "period": 20000000, "base_lat": 115000}
    ]

    seq = 0
    total_misses = 0

    try:
        while max_frames is None or seq < max_frames:
            seq += 1
            ts = time.time_ns()
            system_jitter = 1.15 + 0.4 * math.sin(seq * 0.1) + random.uniform(0, 0.2)
            failsafe = False

            header = struct.pack(
                HEADER_FORMAT,
                b"QPUL",
                seq,
                ts,
                len(tasks),
                seq * 100,
                total_misses,
                system_jitter,
                failsafe
            )

            task_payload = bytearray()
            for t in tasks:
                lat = int(t["base_lat"] + random.uniform(-1000, 2500))
                miss = 1 if lat > t["period"] else 0
                t_bytes = struct.pack(
                    TASK_FORMAT,
                    t["name"].ljust(32, b'\x00'),
                    t["id"],
                    t["level"],
                    t["priority"],
                    t["period"],
                    int(t["base_lat"] * 1.4), # worst
                    int(t["base_lat"] * 0.9), # best
                    lat,                      # avg
                    miss,
                    seq,
                    random.uniform(2.0, 15.0),
                    True
                )
                task_payload.extend(t_bytes)

            sock.sendto(header + task_payload, (HOST, PORT_UDP))
            if seq % 10 == 0:
                print(f"[Simulator] Emitted frame #{seq:04d} | Jitter: {system_jitter:.2f} µs")
            time.sleep(0.1)

    except KeyboardInterrupt:
        print("\n[Simulator] Stopped.")

if __name__ == "__main__":
    run_simulation()
