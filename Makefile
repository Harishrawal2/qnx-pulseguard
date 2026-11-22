# QNX PulseGuard Makefile
# Supports native compilation on Windows (gcc/clang/cl), Linux, and QNX SDP 8.0 (qcc)

CC ?= gcc
CFLAGS ?= -O2 -Wall -Wextra -Iinclude
LDFLAGS ?= -lpthread -lm

ifeq ($(OS),Windows_NT)
    TARGET = qnx_pulseguard.exe
    LDFLAGS = -lws2_32 -lpthread
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),QNX)
        CC = qcc -Vgcc_ntoarmv7le # or x86_64
        TARGET = qnx_pulseguard
        LDFLAGS = -lsocket
    else
        TARGET = qnx_pulseguard
        LDFLAGS = -lpthread -lm
    endif
endif

SRC = src/qnx_pulseguard.c src/main.c
OBJ = $(SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f src/*.o $(TARGET) *.exe
