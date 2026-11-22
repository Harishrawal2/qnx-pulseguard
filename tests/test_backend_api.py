#!/usr/bin/env python3
"""
PulseGuard Backend API & Schema Integration Tests
Verifies HTTP endpoints, JSON schemas, CSV exports, and stress control.
"""

import unittest
import urllib.request
import urllib.error
import json
import time

BASE_URL = "http://127.0.0.1:8080"

class TestPulseGuardApi(unittest.TestCase):

    def test_01_telemetry_endpoint_schema(self):
        """Verify /api/telemetry returns valid schema and required QNX fields"""
        url = f"{BASE_URL}/api/telemetry"
        req = urllib.request.Request(url)
        with urllib.request.urlopen(req, timeout=3) as resp:
            self.assertEqual(resp.status, 200)
            self.assertEqual(resp.headers.get("Content-Type"), "application/json")
            data = json.loads(resp.read().decode('utf-8'))

            # Check primary keys
            self.assertIn("data_mode", data)
            self.assertIn("qnx_target_info", data)
            self.assertIn("system_metrics", data)
            self.assertIn("tasks", data)
            self.assertIn("latency_history", data)

            # Check QNX target metadata
            target = data["qnx_target_info"]
            self.assertEqual(target["os"], "QNX Neutrino RTOS 8.0")
            self.assertIn("ClockCycles()", target["timer_source"])

            # Check task schema
            self.assertGreaterEqual(len(data["tasks"]), 3)
            task_names = [t["name"] for t in data["tasks"]]
            self.assertIn("BrakeByWire_Actuator", task_names)
            self.assertIn("SteeringAngle_Sensor", task_names)
            self.assertIn("Vehicle_Telematics_GW", task_names)

    def test_02_ipc_benchmark_endpoint(self):
        """Verify /api/ipc_benchmark returns valid QNX message passing round-trip distributions"""
        url = f"{BASE_URL}/api/ipc_benchmark"
        req = urllib.request.Request(url)
        with urllib.request.urlopen(req, timeout=3) as resp:
            self.assertEqual(resp.status, 200)
            data = json.loads(resp.read().decode('utf-8'))

            self.assertIn("benchmark", data)
            self.assertIn("primitives", data)
            self.assertIn("MsgSend()", data["primitives"])
            self.assertIn("results", data)
            self.assertGreaterEqual(len(data["results"]), 4)

            # Validate payload progression
            payloads = [r["payload_bytes"] for r in data["results"]]
            self.assertIn(64, payloads)
            self.assertIn(1024, payloads)
            self.assertIn(16384, payloads)

            # Confirm round-trip latency sanity
            for r in data["results"]:
                self.assertGreater(r["avg_us"], 0.0)
                self.assertGreaterEqual(r["max_us"], r["min_us"])
                self.assertGreater(r["throughput_msg_sec"], 1000)

    def test_03_audit_export_endpoint(self):
        """Verify /api/export_audit returns performance evidence package"""
        url = f"{BASE_URL}/api/export_audit"
        req = urllib.request.Request(url)
        with urllib.request.urlopen(req, timeout=3) as resp:
            self.assertEqual(resp.status, 200)
            data = json.loads(resp.read().decode('utf-8'))

            self.assertIn("audit_metadata", data)
            self.assertEqual(data["audit_metadata"]["target_os"], "QNX Neutrino RTOS 8.0.0")
            self.assertIn("Performance Analysis Evidence", data["audit_metadata"]["compliance_domain"])
            self.assertIn("timing_safety_metrics", data)

    def test_04_csv_export_endpoint(self):
        """Verify /api/export_csv returns valid CSV format with headers"""
        url = f"{BASE_URL}/api/export_csv"
        req = urllib.request.Request(url)
        with urllib.request.urlopen(req, timeout=3) as resp:
            self.assertEqual(resp.status, 200)
            self.assertEqual(resp.headers.get("Content-Type"), "text/csv")
            content = resp.read().decode('utf-8')
            lines = content.strip().split("\n")

            self.assertGreaterEqual(len(lines), 2)
            self.assertIn("Timestamp,BrakeByWire_Latency_us", lines[0])

    def test_05_stress_toggle(self):
        """Verify /api/toggle_stress toggles workload contention"""
        url = f"{BASE_URL}/api/toggle_stress"
        req = urllib.request.Request(url, data=b"", method="POST")
        with urllib.request.urlopen(req, timeout=3) as resp:
            self.assertEqual(resp.status, 200)
            data1 = json.loads(resp.read().decode('utf-8'))
            self.assertIn("stress_active", data1)

        # Toggle back to restore clean state
        with urllib.request.urlopen(req, timeout=3) as resp:
            self.assertEqual(resp.status, 200)
            data2 = json.loads(resp.read().decode('utf-8'))
            self.assertEqual(data2["stress_active"], not data1["stress_active"])

if __name__ == "__main__":
    unittest.main()
