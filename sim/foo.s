    ori r2 r2 13
    or r31 r0 r31
    sw r31 r3 4
    addi r3 r3 8
    jal fib10
    subi r3 r3 8
    lw r31 r3 4
    or r31 r0 r31
    or r31 r0 r31
    sw r31 r3 4
    addi r3 r3 8
    nop
#    main program ends
fib10:
    ori r28 r0 1
    slt r28 r28 r2
    bne r0 r28 ble_else24
    jr r31
ble_else24:
    subi r5 r2 1
    sw r2 r3 0
    or r31 r0 r31
#mv r2 r5
    or r2 r0 r5
    sw r31 r3 4
    addi r3 r3 8
    jal fib10
    subi r3 r3 8
    lw r31 r3 4
    or r31 r0 r31
    lw r5 r3 0
    subi r5 r5 2
    sw r2 r3 4
    or r31 r0 r31
#mv r2 r5
    or r2 r0 r5
    sw r31 r3 12
    addi r3 r3 16
    jal fib10
    subi r3 r3 16
    lw r31 r3 12
    or r31 r0 r31
    lw r5 r3 4
    add r2 r5 r2
    jr r31
