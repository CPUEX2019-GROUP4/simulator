  addi r1 r0 3
  addi r5 r0 100
  sw r1 124 r5
  addi r5 r5 128
  lw r1 -4 r5
  slti r2 r1 2
  bne r2 r0 13
  addi r1 r1 -1
  sw r1 124 r5
  addi r5 r5 128
  j 4
  sw r1 -8 r5
  lw r1 -4 r5
  addi r1 r1 -2
  sw r1 124 r5
  addi r5 r5 128
  j 4
  lw r2 -8 r5
  add r1 r1 r2
  j 21
  addi r1 r0 1
  nop
