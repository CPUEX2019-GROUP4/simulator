CC = g++
LDFLAGS = -Wall -O2

all: sim swap assemble

sim: sim.cc
	$(CC) $(LDFLAGS) $< -o $@

swap: swap.c
	$(CC) $(LDFLAGS) $< -o $@

assemble: assemble.cc
	$(CC) $(LDFLAGS) $< -o $@

clean:
	rm sim swap assemble
