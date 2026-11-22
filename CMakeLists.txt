cmake_minimum_required(VERSION 3.15)
project(qnx_pulseguard C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

include_directories(include)

# Source files
set(SOURCES
    src/qnx_pulseguard.c
    src/main.c
)

add_executable(qnx_pulseguard ${SOURCES})

# Target-specific linking
if(QNXNTO)
    message(STATUS "Configuring for QNX Neutrino RTOS 8.0 target (qcc)")
    target_link_libraries(qnx_pulseguard socket)
elseif(WIN32)
    message(STATUS "Configuring for Windows host simulation")
    target_link_libraries(qnx_pulseguard ws2_32)
else()
    message(STATUS "Configuring for Linux / POSIX simulation")
    target_link_libraries(qnx_pulseguard pthread m)
endif()

# Installation target
install(TARGETS qnx_pulseguard DESTINATION bin)
