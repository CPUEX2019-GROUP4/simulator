#pragma once

#include "bit.hh"

#define MOUTPREC 11
#define FINV_MINPREC 11
#define FINV_LOOP_COUNT 1
#define SQRT_LOOP_COUNT 1
#define SQRT_MINPREC 10

#define MUSE(prec) (0x00800000 - (1 << (23 - prec)))

uint32_t finv_init_m(uint32_t m);
uint32_t finv_init_u(float f);
float finv_init(float f);
uint32_t sqrt_inv_init_m(uint32_t emod2, uint32_t m);
uint32_t sqrt_inv_init_u(float f);
float sqrt_init(float f);
