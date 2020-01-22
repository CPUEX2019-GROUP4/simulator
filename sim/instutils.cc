#include "instutils.hh"

uint32_t get_opcode(uint32_t inst) {return (inst >> 26) & 0x3f;}
uint32_t get_rd(uint32_t inst) {return (inst >> 21) & 0x1f;}
uint32_t get_ra(uint32_t inst) {return (inst >> 16) & 0x1f;}
uint32_t get_rb(uint32_t inst) {return (inst >> 11) & 0x1f;}
uint32_t get_shift(uint32_t inst) {return (inst >> 6) & 0x1f;}
int32_t get_shift_signed(uint32_t inst) {return (inst & 0x3ff) - (inst & 0x400);}
//int32_t get_shift_signed(uint32_t inst) {return (int16_t)(inst & 0x3ff);}
uint32_t get_func(uint32_t inst) {return (inst >> 0) & 0x3f;}
uint16_t get_imm(uint32_t inst) {return (inst >> 0) & 0xffff;}
int16_t get_imm_signed(uint32_t inst) {return (inst & 0x7fff) - (inst & 0x8000);}
//int16_t get_imm_signed(uint32_t inst) {return (int16_t)(inst & 0x7fff);}
uint32_t get_addr(uint32_t inst) {return (inst >> 0) & 0x3ffffff;}
