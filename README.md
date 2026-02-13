# Adaptive Arena 🏟️

**High-Performance Memory Management & Visualization System** for Latency-Critical Applications.

## Overview
Adaptive Arena is a C++17 memory management framework designed to replace standard heap allocation (`malloc/free`) with a **smart, learning-based arena**. It is specifically optimized for high-throughput scenarios like **Ultrasound Beamforming** and **Real-Time Signal Processing**.

## Key Features 🚀

### 1. **Predictive Allocation (EMA)**
   - Uses **Exponential Moving Average** to learn memory usage patterns.
   - Pre-allocates memory "Super-Pages" during idle times to prevent allocation spikes during critical processing windows.

### 2. **Ultrasound RF Mode (Zero-Copy)**
   - **Structure of Arrays (SoA)** layout for maximum cache efficiency.
   - **Jitter-Adaptive Ring Buffer**: Automatically expands to absorb system latency spikes (Elastic Expansion).

### 3. **Hybrid Acceleration**
   - **CUDA Present**: Uses `cudaHostAlloc` for **Zero-Copy** GPU access (PCIe bypass).
   - **CPU Only**: Falls back to `VirtualAlloc` (Windows) for OS-level Pinned Memory.
   - **Runtime Detection**: No compile-time CUDA dependency required.

### 4. **Safety Hardening**
   - **Thread-Safe**: `std::shared_mutex` for high-concurrency MRSW access.
   - **Resource Limits**: Strict `Hard Limit` enforcement to prevent OOM.
   - **Graceful Failure**: Robust error handling for allocation failures.

### 5. **Real-Time Visualization**
   - Built-in **Dear ImGui** dashboard.
   - Live telemetry: **Latency (ns)**, **Throughput (GB/s)**, **Memory Usage**, **Ring Buffer Lag**.

## Getting Started 🛠️

### Prerequisites
- **C++17 Compiler** (MSVC 2017+, GCC 7+, Clang 5+)
- **CMake 3.20+**
- **CUDA Toolkit** (Optional, for GPU acceleration)

### Build Instructions
```bash
git clone https://github.com/ES98/AdaptiveArena.git
cd AdaptiveArena
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

### Running the Demo
```bash
./Release/AdaptiveArena.exe
```
Launch the application to see the dashboard. Use the "Push RF Frame" button to simulate data ingestion and observe the ring buffer's adaptive behavior.

## Documentation 📚
- **[Technical Reference](docs/technical_reference.md)**: Detailed architecture and performance metrics.
- **[Project Info](docs/project_info.md)**: General project background.

## License
Apache License 2.0. See `LICENSE` for details.
---
*Created by PJS*
