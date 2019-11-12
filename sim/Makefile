CC = g++
LDFLAGS = -Wall -O2

all: sim run swap assemble preassemble run_label

sim: sim.cc
	$(CC) $(LDFLAGS) $< -o $@

run: run.cc
	$(CC) $(LDFLAGS) $< -o $@

run_label: run_label.cc
	$(CC) $(LDFLAGS) $< -o $@

preassemble: preassemble.cc
	$(CC) $(LDFLAGS) $< -o $@

swap: swap.c
	$(CC) $(LDFLAGS) $< -o $@

assemble: assemble.cc
	$(CC) $(LDFLAGS) $< -o $@

clean:
	rm sim run swap assemble preassemble run_label
