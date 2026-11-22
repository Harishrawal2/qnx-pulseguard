cmake_minimum_required(VERSION 3.15)
project(qnx_pulseguard C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

include_directories(include)

# Primary PulseGuard Real-Time Daemon
add_executable(qnx_pulseguard
    src/qnx_pulseguard.c
    src/main.c
)

# Synchronous Microkernel IPC Latency Benchmark
add_executable(qnx_ipc_bench
    src/qnx_ipc_bench.c
)

# Target-specific linking
if(QNXNTO)
    message(STATUS "Configuring for QNX Neutrino RTOS 8.0 target (qcc)")
    target_link_libraries(qnx_pulseguard socket)
    target_link_libraries(qnx_ipc_bench socket)
elseif(WIN32)
    message(STATUS "Configuring for Windows host simulation")
    target_link_libraries(qnx_pulseguard ws2_32)
    target_link_libraries(qnx_ipc_bench ws2_32)
else()
    message(STATUS "Configuring for Linux / POSIX simulation")
    target_link_libraries(qnx_pulseguard pthread m)
    target_link_libraries(qnx_ipc_bench pthread m)
endif()

# Installation target
install(TARGETS qnx_pulseguard qnx_ipc_bench DESTINATION bin)
