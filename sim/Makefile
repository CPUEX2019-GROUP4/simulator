CC = g++
CFLAGS = -Wall -O3
LDFLAGS = -lm

OBJS = bit.o stringutils.o fpu.o format.o assembleutils.o

MANIPCONF = manip.conf
MANIPTARGETS = sim.manip run.manip run_label.manip

all: manip domanip sim run swap assemble preassemble run_label
	echo all

%.manip.o: %.manip.cc
	$(CC) $(CFLAGS) -c $^ -o $@

domanip:
	echo starting manip...
	./manip manip.conf
	echo manip finished

%.o: %.cc
	$(CC) $(CFLAGS) -c $^ -o $@

manip: $(OBJS) manip.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

preassemble: $(OBJS) preassemble.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

assemble: $(OBJS) assemble.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

sim: $(OBJS) sim.manip.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

run: $(OBJS) run.manip.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

run_label: $(OBJS) run_label.manip.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

swap: swap.c
	$(CC) $(LDFLAGS) $< -o $@

clean:
	rm $(OBJS) sim run swap assemble preassemble run_label manip sim.o run.o assemble.o preassemble.o run_label.o manip.o $(MANIPTARGETS:%.manip=%.manip.cc) $(MANIPTARGETS:%.manip=%.manip.o)
