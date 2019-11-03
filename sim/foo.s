fib:
  ori r28 r0 4
  slt r28 r28 r1
  bne r0 r28 fib
  jr r31
end:
  nop
