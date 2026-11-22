#!/usr/bin/env python3
"""
PulseGuard Statistical Engine & Binary Frame Unit Tests
Tests percentile calculations, binary frame struct serialization, deadline detection,
partial benchmark sample handling, and maximum observed execution time tracking.
"""

import unittest
import struct
import numpy as np

HEADER_FORMAT = "<4sIQIIQd?"
TASK_FORMAT = "<32sIiIQQQQQQd?"

def calculate_percentiles(samples, valid_count=None):
    """Calculates P50, P95, P99, P99.9 latencies in microseconds only over valid samples"""
    if valid_count is not None:
        samples = samples[:valid_count]
    if not samples:
        return {"p50": 0.0, "p95": 0.0, "p99": 0.0, "p99_9": 0.0}
    s = sorted(samples)
    return {
        "p50": round(float(np.percentile(s, 50)), 2),
        "p95": round(float(np.percentile(s, 95)), 2),
        "p99": round(float(np.percentile(s, 99)), 2),
        "p99_9": round(float(np.percentile(s, 99.9)), 2)
    }

def evaluate_deadline(execution_us, deadline_us):
    """Evaluates whether an execution breached the configured task deadline"""
    missed = execution_us > deadline_us
    utilization_pct = round((execution_us / deadline_us) * 100.0, 2)
    return missed, utilization_pct

class TestStatisticsAndProtocol(unittest.TestCase):

    def test_01_binary_header_packing(self):
        """Verify PulseGuardFrame header binary packing and unpacking"""
        magic = b"QPUL"
        seq = 1042
        ts = 1763800000123456789
        num_tasks = 3
        ticks = 50000
        misses = 0
        jitter = 1.345
        failsafe = False

        packed = struct.pack(HEADER_FORMAT, magic, seq, ts, num_tasks, ticks, misses, jitter, failsafe)
        self.assertEqual(len(packed), struct.calcsize(HEADER_FORMAT))

        # Unpack and verify fields
        u_magic, u_seq, u_ts, u_tasks, u_ticks, u_misses, u_jitter, u_failsafe = struct.unpack(HEADER_FORMAT, packed)
        self.assertEqual(u_magic, b"QPUL")
        self.assertEqual(u_seq, 1042)
        self.assertEqual(u_ts, ts)
        self.assertEqual(u_tasks, 3)
        self.assertEqual(u_misses, 0)
        self.assertAlmostEqual(u_jitter, 1.345, places=3)
        self.assertFalse(u_failsafe)

    def test_02_task_telemetry_packing(self):
        """Verify TaskTelemetry binary record serialization"""
        name = b"BrakeByWire_Actuator\x00".ljust(32, b'\x00')
        tid = 1
        level = 2 # ASIL-D Reference
        prio = 63
        period_ns = 1000000 # 1 ms
        worst_ns = 17800
        best_ns = 11200
        avg_ns = 12400
        misses = 0
        total_exec = 1000
        cpu_pct = 4.5
        healthy = True

        packed = struct.pack(TASK_FORMAT, name, tid, level, prio, period_ns, worst_ns, best_ns, avg_ns, misses, total_exec, cpu_pct, healthy)
        self.assertEqual(len(packed), struct.calcsize(TASK_FORMAT))

        u_name, u_tid, u_lvl, u_prio, u_period, u_worst, u_best, u_avg, u_misses, u_total, u_cpu, u_healthy = struct.unpack(TASK_FORMAT, packed)
        self.assertEqual(u_name.decode('utf-8').split('\x00')[0], "BrakeByWire_Actuator")
        self.assertEqual(u_prio, 63)
        self.assertEqual(u_period, 1000000)
        self.assertTrue(u_healthy)

    def test_03_percentile_calculations(self):
        """Verify mathematical accuracy of latency percentiles"""
        np.random.seed(42)
        samples = list(np.random.normal(loc=12.0, scale=0.8, size=1000))
        samples.append(18.5)
        samples.append(21.2)

        stats = calculate_percentiles(samples)
        self.assertGreater(stats["p50"], 11.5)
        self.assertLess(stats["p50"], 12.5)
        self.assertGreater(stats["p95"], stats["p50"])
        self.assertGreater(stats["p99"], stats["p95"])
        self.assertGreater(stats["p99_9"], stats["p99"])

    def test_04_deadline_evaluation(self):
        """Verify deadline detection and timing headroom calculations"""
        missed, util = evaluate_deadline(12.4, 1000.0)
        self.assertFalse(missed)
        self.assertAlmostEqual(util, 1.24, places=2)

        missed, util = evaluate_deadline(1050.0, 1000.0)
        self.assertTrue(missed)
        self.assertEqual(util, 105.0)

    def test_05_max_observed_execution_time_tracking(self):
        """Verify tracking of high-water mark / maximum observed execution time"""
        observed_samples = [12.1, 14.3, 11.8, 17.8, 13.2, 12.0]
        max_observed = max(observed_samples)
        self.assertEqual(max_observed, 17.8)

    def test_06_partial_benchmark_sample_handling(self):
        """Verify that partial benchmark failures calculate statistics only over successful samples"""
        allocated_capacity = 100
        buffer = [0.0] * allocated_capacity
        
        # Simulate partial execution where only 45 samples were successfully collected
        valid_samples = [1.1 + (i * 0.02) for i in range(45)]
        for i, val in enumerate(valid_samples):
            buffer[i] = val
        
        # If we wrongly sorted the full buffer, index 0 would be 0.0 (uninitialized)
        stats = calculate_percentiles(buffer, valid_count=45)
        self.assertGreater(stats["p50"], 1.0)
        self.assertAlmostEqual(stats["p50"], float(np.percentile(valid_samples, 50)), places=2)

    def test_07_zero_sample_graceful_handling(self):
        """Verify that zero samples cleanly return 0.0 metrics without exception"""
        stats = calculate_percentiles([], valid_count=0)
        self.assertEqual(stats["p50"], 0.0)
        self.assertEqual(stats["p95"], 0.0)
        self.assertEqual(stats["p99"], 0.0)

if __name__ == "__main__":
    unittest.main()
