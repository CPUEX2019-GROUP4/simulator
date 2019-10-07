all: sim swap assemble verify

sim: sim.cc
	g++ -Wall sim.cc -o sim

swap: swap.c
	gcc -Wall swap.c -o swap

assemble: assemble.cc
	g++ -Wall assemble.cc -o assemble

verify: verify.cc
	g++ -Wall verify.cc -o verify

clean:
	rm sim swap assemble verify
