CC = g++
LDFLAGS = -Wall -O2

all: sim swap assemble preassemble

sim: sim.cc
	$(CC) $(LDFLAGS) $< -o $@

preassemble: preassemble.cc
	$(CC) $(LDFLAGS) $< -o $@

swap: swap.c
	$(CC) $(LDFLAGS) $< -o $@

assemble: assemble.cc
	$(CC) $(LDFLAGS) $< -o $@

clean:
	rm sim swap assemble preassemble
