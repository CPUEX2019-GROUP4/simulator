#pragma once

#include <cstdint>

//XXX: fpuのcopy (for performance) (as of 25 Nov)
#define MINPREC 6
#define MOUTPREC 6
#define SQRT_LOOP_COUNT 2
#define FINV_LOOP_COUNT 2

#define MUSE(prec) (0x00800000 - (1 << (23 - prec)))

uint32_t sqrt_init_m(uint32_t emod2, uint32_t m);
uint32_t sqrt_init_u(float f);
float sqrt_init(float f);
uint32_t finv_init_m(uint32_t m);
uint32_t finv_init_u(float f);
float finv_init(float f);
float mfinv(float x, float init);
