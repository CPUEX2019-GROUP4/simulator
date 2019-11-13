CC = g++
CFLAGS = -Wall -O2
LDFLAGS = -lm
SRCS=$(wildcard *.cc)
TARGETS = swap preassemble assemble sim run run_label
OBJTARGETS = $(TARGETS:=.o)
OBJS = $(filter-out swap.o preassemble.o assemble.o sim.o run.o run_label.o, $(SRCS:.cc=.o))

.PHONY: all
all: sim run assemble preassemble run_label

sim: $(OBJS) sim.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

run: $(OBJS) run.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

run_label: $(OBJS) run_label.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

preassemble: $(OBJS) preassemble.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

assemble: $(OBJS) assemble.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $^

swap: swap.o
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm $(OBJS) $(TARGETS) $(OBJTARGETS)
