CC = g++
CFLAGS = -Wall -O3
LDFLAGS = -lm
SRCS = $(shell find . -type f -name "*.cc")
OBJS = $(shell find . -type f -name "*.cc" | grep -v "sim.cc" | grep -v "run.cc" | grep -v "run_label.cc" | grep -v "assemble.cc" | grep -v "preassemble.cc" | sed "s/\.cc/\.o/")
TARGETS = $(SRCS:%.cc=%)

all: sim run swap assemble preassemble run_label

%.o: %.cc
	$(CC) $(CFLAGS) -c $^ -o $@

preassemble: $(OBJS) preassemble.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

assemble: $(OBJS) assemble.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

sim: $(OBJS) sim.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

run: $(OBJS) run.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

run_label: $(OBJS) run_label.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

swap: swap.c
	$(CC) $(LDFLAGS) $< -o $@

clean:
	rm $(OBJS) sim run swap assemble preassemble run_label sim.o run.o assemble.o preassemble.o run_label.o

#sim: sim.cc
#	$(CC) $(LDFLAGS) $< -o $@
#
#run: run.cc
#	$(CC) $(LDFLAGS) $< -o $@
#
#run_label: run_label.cc
#	$(CC) $(LDFLAGS) $< -o $@
#
#preassemble: preassemble.cc
#	$(CC) $(LDFLAGS) $< -o $@
#
#swap: swap.c
#	$(CC) $(LDFLAGS) $< -o $@
#
#assemble: assemble.cc
#	$(CC) $(LDFLAGS) $< -o $@
