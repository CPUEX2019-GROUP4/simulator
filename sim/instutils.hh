#pragma once
#include <cstdint>

uint32_t get_opcode(uint32_t inst);
uint32_t get_rd(uint32_t inst);
uint32_t get_ra(uint32_t inst);
uint32_t get_rb(uint32_t inst);
uint32_t get_shift(uint32_t inst);
int32_t get_shift_signed(uint32_t inst);
uint32_t get_func(uint32_t inst);
uint16_t get_imm(uint32_t inst);
int16_t get_imm_signed(uint32_t inst);
uint32_t get_addr(uint32_t inst);
