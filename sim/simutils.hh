#pragma once

#define BYTES_INSTRUCTION 4
#define LEN_INSTRUCTION 200000
#define N_REG 32
#define SIZE_MEM (2<<20)

#define $rd (int_reg[get_rd(inst)])
#define $ra (int_reg[get_ra(inst)])
#define $rb (int_reg[get_rb(inst)])
#define $fd (float_reg[get_rd(inst)])
#define $fa (float_reg[get_ra(inst)])
#define $fb (float_reg[get_rb(inst)])

enum Comm {STEP, PRINT, CLEAR, MONITOR, UNMONITOR, BREAK, UNBREAK, HELP, NIL, QUIT, ERR, RUN, FAST, NOP, OP};
