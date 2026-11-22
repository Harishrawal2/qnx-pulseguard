# QNX PulseGuard — Video Demonstration Script (Under 3 Minutes)

> [!IMPORTANT]
> **Official Hackathon Submission Rule:**  
> The video demonstration **must be uploaded to YouTube with Public visibility** and must be **strictly under 3:00 minutes** in total length. Unlisted/private YouTube links, Vimeo, or external video players do not qualify under official submission criteria.

**Video Target Duration:** 2 minutes 40 seconds  
**Platform:** Public YouTube Video  
**Target Audience:** BlackBerry QNX Hackathon Evaluators, Embedded Systems Architects  

---

## 0:00 – 0:30 | Introduction & Problem Statement
- **Visual:** Title slide and quick transition to the live QNX PulseGuard dashboard (`http://localhost:8080`).
- **Narrator:**
  > "In autonomous vehicle systems, deterministic timing isn't just a metric—it's safety-critical. On modern multicore platforms running QNX SDP 8.0, safety tasks like brake-by-wire share CPU resources with complex telematics. How do engineers prove microsecond determinism and catch deadline breaches before they hit the road? Introducing **QNX PulseGuard**."

---

## 0:30 – 1:15 | Architecture & Real-Time Monitoring
- **Visual:** Zoom in on the header status chips: `procnto-smp-instr`, `ClockCycles() @ 1.8GHz`, `STREAM ACTIVE (10.0 Hz)`. Then point out the four KPI metric cards.
- **Narrator:**
  > "QNX PulseGuard pairs a native C agent utilizing QNX 8.0 Neutrino microkernel APIs with an enterprise real-time host dashboard. 
  > Here on the monitoring view, we track 99.9th percentile timing jitter at 0.95 microseconds, zero ASIL-D deadline breaches, and worst-case execution time under 18 microseconds. 
  > The lower waterfall chart visualizes execution times across three prioritized ECU tasks running under QNX `SCHED_FIFO` scheduling."

---

## 1:15 – 1:55 | CPU Contention & Microkernel Preemption
- **Visual:** Click the **Simulate CPU Contention** button. Show button state update to `Contention Active` and observe the waterfall chart.
- **Narrator:**
  > "Now, let's inject synthetic CPU contention. Notice what happens: our low-priority Telematics task experiences slight latency increases due to resource contention. 
  > But our highest-priority safety task—BrakeByWire at priority 63—maintains zero deadline breaches and steady execution time. This demonstrates deterministic priority preemption in action."

---

## 1:55 – 2:25 | Microkernel IPC Benchmark & Navigation
- **Visual:** Click the **Analysis** tab to view the QNX Microkernel IPC Benchmark table and chart.
- **Narrator:**
  > "Switching to the Analysis view, PulseGuard integrates native QNX microkernel IPC benchmarking. By measuring `MsgSend`, `MsgReceive`, and `MsgReply` across payloads from 64 bytes to 16 kilobytes, developers can inspect round-trip microkernel latency and throughput."

---

## 2:25 – 2:45 | Audit Export & Conclusion
- **Visual:** Click the **Reports** tab and demonstrate clicking **Export Timing Evidence Report** to download the JSON report.
- **Narrator:**
  > "Finally, engineering teams can export full performance audit reports in JSON and CSV formats for integration into their continuous verification pipelines. 
  > QNX PulseGuard delivers actionable, deterministic timing visibility for the next generation of software-defined vehicles on QNX SDP 8.0."
