    ori r2 r0 125
    or r31 r0 r31
    sw r31 r3 4
    addi r3 r3 8
    jal x8
    subi r3 r3 8
    lw r31 r3 4
    or r31 r0 r31
    sub r5 r0 r2
    sub r2 r2 r5
    or r31 r0 r31
    sw r31 r3 4
    addi r3 r3 8
    nop
x8:
    sub r5 r0 r2
    sub r2 r2 r5
    sub r5 r0 r2
    sub r6 r0 r5
    sub r5 r5 r6
    sub r2 r2 r5
    sub r5 r0 r2
    sub r2 r2 r5
    jr r31
