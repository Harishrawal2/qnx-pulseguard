# QNX PulseGuard Makefile
# Supports native compilation on QNX SDP 8.0 (qcc) and host systems

CC ?= gcc
CFLAGS ?= -O2 -Wall -Wextra -Iinclude
LDFLAGS ?= -lpthread -lm

ifeq ($(OS),Windows_NT)
    TARGET_DAEMON = qnx_pulseguard.exe
    TARGET_BENCH = qnx_ipc_bench.exe
    LDFLAGS = -lws2_32 -lpthread
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),QNX)
        CC = qcc -Vgcc_ntoarmv7le # Or -Vgcc_ntox86_64
        TARGET_DAEMON = qnx_pulseguard
        TARGET_BENCH = qnx_ipc_bench
        LDFLAGS = -lsocket
    else
        TARGET_DAEMON = qnx_pulseguard
        TARGET_BENCH = qnx_ipc_bench
        LDFLAGS = -lpthread -lm
    endif
endif

all: $(TARGET_DAEMON) $(TARGET_BENCH)

$(TARGET_DAEMON): src/qnx_pulseguard.o src/main.o
	$(CC) $^ -o $@ $(LDFLAGS)

$(TARGET_BENCH): src/qnx_ipc_bench.o
	$(CC) $^ -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f src/*.o $(TARGET_DAEMON) $(TARGET_BENCH) *.exe
